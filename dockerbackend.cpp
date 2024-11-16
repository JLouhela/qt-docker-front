#include "dockerbackend.h"
#include <QThreadPool>

namespace
{
}

DockerBackend::DockerBackend(QObject *parent)
    : QObject{parent}
{
    m_connected = m_dockerAPI.connect();
    if (m_connected)
    {
        QObject::connect(&m_dockerAPI, &DockerAPI::runningContainersReady, this, &DockerBackend::onContainersUpdated);
        setupPolling();
    }
    else
    {
        qDebug() << "TODO: display message somewhere for the user";
    }
}

void DockerBackend::setupPolling()
{
    QObject::connect(&m_timer, &QTimer::timeout, this, &DockerBackend::pollContainerStatus);
    m_timer.start(1000);
    m_dockerAPI.queryRunningContainers();
}

QStringList DockerBackend::containers()
{
    return m_containers;
}

void DockerBackend::pollContainerStatus()
{
   // m_dockerAPI.queryRunningContainers();
}

void DockerBackend::onContainersUpdated(const QStringList &containers)
{
    m_containers = containers;
}
