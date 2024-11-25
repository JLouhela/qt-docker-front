#include "dockerapi.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>

namespace
{

std::string getContainersQuery()
{
    return "GET /containers/json?all=1 HTTP/1.1\r\n"
           "Host: 127.0.0.1\r\n"
           "Connection: close\r\n"
           "\r\n";
}

std::string getContainerInfoQuery(const QString& containerName)
{
    QString ret = "GET /containers/" +containerName + "/stats?stream=false HTTP/1.1\r\n"
           "Host: 127.0.0.1\r\n"
           "Connection: close\r\n"
           "\r\n";
    return ret.toStdString();
}

Container::State stateFromString(const QString& stateString)
{
    if (stateString == "running")
    {
        return Container::State::RUNNING;
    }
    else if (stateString == "exited")
    {
        return Container::State::STOPPED;
    }
    else if (stateString == "paused")
    {
        return Container::State::PAUSED;
    }
    else if (stateString == "restarting")
    {
        return Container::State::RESTARTING;
    }
    return Container::State::UNKNOWN;
}

double getCpuUsage(const QJsonObject& jsonObj)
{
    const auto cpuStats = jsonObj["cpu_stats"].toObject();
    const auto cpuUsage = cpuStats["cpu_usage"].toObject();
    const auto cpuTotalUsage = cpuUsage["total_usage"].toInteger();

    const auto preCpuStats = jsonObj["precpu_stats"].toObject();
    const auto preCpuUsage = preCpuStats["cpu_usage"].toObject();
    const auto preCpuTotalUsage = preCpuUsage["total_usage"].toInteger();

    const auto cpuDelta = cpuTotalUsage - preCpuTotalUsage;
    const auto systemCpuDelta = cpuStats["system_cpu_usage"].toInteger() - preCpuStats["system_cpu_usage"].toInteger();
    const auto number_cpus = cpuStats["online_cpus"].toInteger(1);

    return (static_cast<double>(cpuDelta) / systemCpuDelta) * number_cpus * 100.0;
}

// Pair of percentage, bytes
std::pair<double, qint64> getMemoryUsage(const QJsonObject& jsonObj)
{
    const auto memStats = jsonObj["memory_stats"].toObject();
    const auto stats = memStats["stats"].toObject();
    const auto usedMemory = memStats["usage"].toInteger() - stats["cache"].toInteger();
    const auto availableMemory = memStats["limit"].toInteger();
    return std::make_pair(((static_cast<double>(usedMemory) / availableMemory) * 100.0), usedMemory);
}

}

DockerAPI::DockerAPI(QObject *parent)
    : QObject{parent}
    , m_socket{nullptr}
{
}

DockerAPI::~DockerAPI()
{
    m_socket->disconnectFromServer();
}

void DockerAPI::queryRunningContainers()
{
    const auto parseFunc = [this](const QJsonDocument& jsonDoc){
        // containers/json response is an array
        if (!jsonDoc.isArray())
        {
            return;
        }
        Containers result;
        QJsonArray array = jsonDoc.array();
        for (const auto arrayElement : array)
        {
            QJsonObject jsonObject = arrayElement.toObject();
            QString containerName = jsonObject.value("Names")[0].toString();
            // Prettify name (erase '/' from the front)
            containerName.erase(containerName.begin());

            QString image = jsonObject.value("Image").toString();
            QString id = jsonObject.value("Id").toString();
            QString status = jsonObject.value("Status").toString();

            const auto stateString = jsonObject.value("State").toString();
            const auto state = stateFromString(stateString);
            result.push_back({containerName, id, image, status, state});
        }
        if (!result.empty())
        {
            emit runningContainersReady(result);
        }
    };
    performQuery(getContainersQuery().c_str(), parseFunc);
}

void DockerAPI::queryContainer(const QString& containerName)
{
    const auto parseFunc = [this, &containerName](const QJsonDocument& jsonDoc){
        // container status response is an object
        if (!jsonDoc.isObject())
        {
            return;
        }
        ContainerInfo result;
        result.name = containerName;
        QJsonObject jsonObj = jsonDoc.object();
        result.cpuUsagePercentage = getCpuUsage(jsonObj);

        auto[memoryPercentage, memoryTotal] = getMemoryUsage(jsonObj);
        result.memoryUsagePercentage = memoryPercentage;

        static constexpr qint32 BYTES_TO_MEBIBYTES = 1048576;
        result.memoryUsageMiB = static_cast<double>(memoryTotal) / BYTES_TO_MEBIBYTES;

        emit containerUpdateReady(result);

    };
    performQuery(getContainerInfoQuery(containerName).c_str(), parseFunc);

}

bool DockerAPI::createSocket()
{
    // Separate function for creating socket in order to be called from the thread context
    // -> avoid "Cannot create children for a parent that is in a different thread."
    if (!m_socket)
    {
        m_socket = new QLocalSocket(this);
        // Try connecting to socket to see if it's
        // accessible
        return connectSocket();
    }
    return false;
}

void DockerAPI::performQuery(const char* msg, std::function<void(const QJsonDocument& json)> jsonHandler)
{
    QString result;
    // Static lock to flatten out design flaws
    // Socket should not be written by two threads at a same time
    // Lessons learned: statemachine and request queue would be better fit
    static QMutex m_lock;
    {
        QMutexLocker lockScope(&m_lock);
        if (m_socket->state() == QLocalSocket::UnconnectedState)
        {
            connectSocket();
        }

        m_socket->write(msg);

        if (!m_socket->waitForBytesWritten(3000))
        {
            qDebug() << "Failed to write HTTP GET for docker daemon";
        }
        result = exhaustSocket();

    }
    if (result == "")
    {
        // Seems to happen in Windows, cannot help this one out
        return;
    }
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
    {
        qWarning() << "Failure parsing docker daemon response, " << jsonError.errorString();
        return;
    }

    jsonHandler(doc);
}

// Fully exhaust response to a single http GET
// Docker requests socket closure after read.
QString DockerAPI::exhaustSocket()
{
    QString result;
    while (m_socket->waitForReadyRead(3000))
    {
        QByteArray response;
        while(m_socket->bytesAvailable() > 0)
        {
            response.append(m_socket->readAll());
        }
        QStringList responseLines = QString::fromUtf8(response).split("\r\n");
        const auto terminatorIndex = responseLines.indexOf("0");

        if (terminatorIndex == 0)
        {
            // Message was complete on the last read, nothing tbd
            break;
        }
        else if (terminatorIndex > 0)
        {
            // Terminator found, need to copy still stuff but nothing more to come
            result.append(responseLines[terminatorIndex - 1]);
            break;
        }
        if (responseLines.size() > 1)
        {
            result.append(responseLines[responseLines.size() - 1]);
        }
    }
    m_socket->disconnect();
    m_socket->close();
    return result;
}

bool DockerAPI::connectSocket()
{
    if (!m_socket)
    {
        return false;
    }

    // Note: Only default paths supported here
    //       Should be configurable
    QString socketPath = "/var/run/docker.sock";
#ifdef _WIN32
    socketPath = "\\\\.\\pipe\\docker_engine";
#endif
    m_socket->connectToServer(socketPath);
    if (m_socket->waitForConnected(1000))
    {
        return true;
    }
    return false;
}

