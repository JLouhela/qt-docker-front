#include "dockerapi.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <cstdint>

namespace
{
QString getJsonString(const QString& daemonResponse)
{
    QChar jsonStartChar = '\0';
    QChar jsonEndChar = '\0';

    if (daemonResponse.indexOf('[') != -1 && daemonResponse.indexOf('[') < daemonResponse.indexOf('{'))
    {
        jsonStartChar = '[';
        jsonEndChar = ']';
    }
    else
    {
        jsonStartChar = '{';
        jsonEndChar = '}';
    }

    const auto startIndex = daemonResponse.indexOf(jsonStartChar);
    const auto endIndex = daemonResponse.lastIndexOf(jsonEndChar) + 1; // keep termination
    return daemonResponse.mid(startIndex, endIndex - startIndex);
}

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
    // Probably missing something
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

bool connectSocket(QLocalSocket& socket)
{
    socket.connectToServer("/var/run/docker.sock");
    if (socket.waitForConnected(1000))
    {
        return true;
    }
    return false;
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
    if (m_socket->state() == QLocalSocket::UnconnectedState
        || m_socket->waitForDisconnected(1000))
    {
        qDebug("Disconnected!");
    }
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


            const auto stateString = jsonObject.value("State").toString();
            const auto state = stateFromString(stateString);
            result.push_back({containerName, id, image, state});
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
    const auto parseFunc = [this](const QJsonDocument& jsonDoc){
        // containers status response is an object
        if (!jsonDoc.isObject())
        {
            return;
        }
        ContainerInfo result;
        QJsonObject jsonObj = jsonDoc.object();

/*
 * cpu_delta = cpu_stats.cpu_usage.total_usage - precpu_stats.cpu_usage.total_usage
system_cpu_delta = cpu_stats.system_cpu_usage - precpu_stats.system_cpu_usage
number_cpus = length(cpu_stats.cpu_usage.percpu_usage) or cpu_stats.online_cpus
CPU usage % = (cpu_delta / system_cpu_delta) * number_cpus * 100.0
*/
        // object.value(QString("id")).toVariant().toLongLong();
        // TODO unpack json
        const auto cpuStats = jsonObj["cpu_stats"].toObject();
        const auto cpuUsage = cpuStats["cpu_usage"].toObject();
        const auto cpuTotalUsage = cpuUsage["total_usage"].toInteger();

        const auto preCpuStats = jsonObj["precpu_stats"].toObject();
        const auto preCpuUsage = preCpuStats["cpu_usage"].toObject();
        const auto preCpuTotalUsage = preCpuUsage["total_usage"].toInteger();

        const auto cpuDelta = cpuTotalUsage - preCpuTotalUsage;
        const auto systemCpuDelta = cpuStats["system_cpu_usage"].toInteger(); - preCpuStats["system_cpu_usage"].toInteger();
        const auto number_cpus = cpuStats["online_cpus"].toInteger();

        if (number_cpus == 0)
        {
            return;
        }
        double cpuPercentage = (static_cast<double>(cpuDelta) / systemCpuDelta) * number_cpus * 100.0;
        result.cpuUsagePercentage = cpuPercentage;
        emit containerUpdateReady(result);

    };
    performQuery(getContainerInfoQuery(containerName).c_str(), parseFunc);

}

bool DockerAPI::createSocket()
{
    // Not portable, windows named pipe: npipe:////./pipe/docker_engine
    // Fine for a linux demo for now
    if (!m_socket)
    {
        m_socket = new QLocalSocket(this);
        QObject::connect(m_socket, &QLocalSocket::errorOccurred, this, &DockerAPI::onError);
    }
    return true;
}

void DockerAPI::onError(QLocalSocket::LocalSocketError socketError)
{
   // qInfo() << "socket disconnected :(";
}

void DockerAPI::performQuery(const char* msg, std::function<void(const QJsonDocument& json)> jsonHandler)
{
    QByteArray response;
    // Static lock to work out design flaws
    // Socket should not be written by two threads at a same time
    static QMutex m_lock;
    {
        QMutexLocker lockScope(&m_lock);
        if (m_socket->state() == QLocalSocket::UnconnectedState)
        {
            connectSocket(*m_socket);
        }
        // Discard possible old stuff
        m_socket->readAll();
        m_socket->write(msg);

        // TODO: switch to signal & slot
        // TODO: Take into account chunked response
        if (!m_socket->waitForBytesWritten(1000))
        {
            qDebug() << "Failed to write HTTP GET for docker daemon";
        }
        if (!m_socket->waitForReadyRead(2000))
        {
            qDebug() << "Did not receive response from docker daemon";
        }
        while (m_socket->bytesAvailable())
        {
            // TODO read to string, parse header + end
            response.append(m_socket->readAll());
            // Allow some time for docker daemon to write everything
            // In general, this should be done with readyRead signal
            // and combining responses together
            QThread::msleep(50);
        }
        m_socket->close();
    }
    QString rawString = QString::fromUtf8(response);
    QString jsonString = getJsonString(QString::fromUtf8(response));
    if (jsonString == "")
    {
        return;
    }
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
    {
        qWarning() << "Failure parsing docker daemon response, " << jsonError.errorString();
        return;
    }
    jsonHandler(doc);
}
