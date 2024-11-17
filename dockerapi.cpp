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
}

DockerAPI::DockerAPI(QObject *parent)
    : QObject{parent}
    , m_socket{nullptr}
{
}

DockerAPI::~DockerAPI()
{
    // TODO mutex
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
        // Weird workaround hack: socket disconnects for some reason
        qInfo() << "Socket unconnected!";
        m_socket->connectToServer("/var/run/docker.sock");
        if (m_socket->waitForConnected(1000))
        {
            qDebug() << "reconnected to docker daemon!";
        }

    }
    // TODO: split waits to slots
    m_socket->write("GET /containers/json HTTP/1.1\r\n"
                   "Host: 127.0.0.1\r\n"
                   "Connection: close\r\n"
                   "\r\n");

    m_socket->waitForBytesWritten(300);
    m_socket->waitForReadyRead(300);
    QByteArray response;
    while (m_socket->bytesAvailable()) {
        response.append(m_socket->readAll());
    }
    QString jsonString = getJsonString(QString::fromUtf8(response));
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
    {
        qWarning() << "Failure parsing docker daemon response, " << jsonError.errorString();
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
        // TODO proper error handling
        const auto containerName = jsonObject.value("Names")[0].toString();
        result.push_back({containerName, Container::Status::UNKNOWN});
    }
    // TODO remove "running" and pass all with state
    emit runningContainersReady(result);

}

bool DockerAPI::connect()
{
    // TODO mutex
    // Not portable, windows named pipe: npipe:////./pipe/docker_engine
    // Fine for a linux demo for now
    if (!m_socket)
    {
        m_socket = new QLocalSocket(this);
    }
    m_socket->connectToServer("/var/run/docker.sock");
    if (m_socket->waitForConnected(1000))
    {
        qDebug() << "Connected to docker daemon!";
        return true;
    }

    qDebug() << "Cannot connect to docker daemon! " << m_socket->errorString();
    return false;
}
