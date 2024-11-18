#include "dockerapi.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace
{
QString getJsonString(const QString& daemonResponse)
{
    const auto startIndex = daemonResponse.indexOf('[');
    const auto endIndex = daemonResponse.lastIndexOf(']') + 1; // keep termination
    return daemonResponse.mid(startIndex, endIndex - startIndex);
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
        qDebug() << "Connected to docker daemon!";
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
    if (m_socket->state() == QLocalSocket::UnconnectedState)
    {
        // Weird workaround hack: socket disconnects after each query
        // Not sure if I have time to find out why
        connectSocket(*m_socket);
    }

    m_socket->write("GET /containers/json?all=1 HTTP/1.1\r\n"
                   "Host: 127.0.0.1\r\n"
                   "Connection: close\r\n"
                   "\r\n");

    // By design: share the docker API between different threads
    // querying different data
    if (!m_socket->waitForBytesWritten(1000))
    {
        qDebug() << "Failed to write HTTP GET for docker daemon";
    }
    if (!m_socket->waitForReadyRead(1000))
    {
        qDebug() << "Did not receive response from docker daemon";
    }
    QByteArray response;
    while (m_socket->bytesAvailable())
    {
        response.append(m_socket->readAll());
    }
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

    // Docker format for supported queries is always an array
    if (!doc.isArray())
    {
        return;
    }
    QJsonArray array = doc.array();
    Containers result;
    for (const auto arrayElement : array)
    {
        QJsonObject jsonObject = arrayElement.toObject();
        // TODO error handling
        QString containerName = jsonObject.value("Names")[0].toString();
        // Prettify name (erase '/' from the front)
        containerName.erase(containerName.begin());

        const auto stateString = jsonObject.value("State").toString();
        const auto state = stateFromString(stateString);
        result.push_back({containerName, state});
    }
    // TODO remove "running" and pass all with state
    emit runningContainersReady(result);

}

bool DockerAPI::connect()
{
        // Not portable, windows named pipe: npipe:////./pipe/docker_engine
    // Fine for a linux demo for now
    if (!m_socket)
    {
        m_socket = new QLocalSocket(this);
        QObject::connect(m_socket, &QLocalSocket::errorOccurred, this, &DockerAPI::onError);
    }

    if (connectSocket(*m_socket))
    {
        return true;
    }

    qDebug() << "Cannot connect to docker daemon! " << m_socket->errorString();
    return false;
}

void DockerAPI::onError(QLocalSocket::LocalSocketError socketError)
{
   // qInfo() << "socket disconnected :(";
}
