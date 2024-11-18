#include "dockerbackend.h"
#include "overiewupdateworker.h"
#include <QThreadPool>

namespace
{
}

DockerBackend::DockerBackend(QObject *parent)
    : QObject{parent}
{
    auto* overviewUpdateWorker = new OverviewUpdateWorker();
    overviewUpdateWorker->moveToThread(&m_overviewPollingThread);
    QObject::connect(&m_overviewPollingThread, &QThread::finished, overviewUpdateWorker, &QObject::deleteLater);
    QObject::connect(&m_timer, &QTimer::timeout, overviewUpdateWorker, &OverviewUpdateWorker::queryContainerUpdate);
    QObject::connect(overviewUpdateWorker, &OverviewUpdateWorker::containersUpdated, this, &DockerBackend::onContainersUpdated);
    m_overviewPollingThread.start();
    m_timer.start(500);
}

DockerBackend::~DockerBackend()
{
    m_overviewPollingThread.quit();
    m_overviewPollingThread.wait();
}

QStringList DockerBackend::containers()
{
    QStringList result;
    for (const auto& container : m_containers)
    {
        result.push_back(container.name);
    }
    return result;
}

int DockerBackend::runningContainersCount()
{
    int result{0};
    for (const auto& container : m_containers)
    {
        if (container.state == Container::State::RUNNING)
        {
            result++;
        }
    }
    return result;
}


int DockerBackend::stoppedContainersCount()
{
    int result{0};
    for (const auto& container : m_containers)
    {
        if (container.state == Container::State::STOPPED)
        {
            result++;
        }
    }
    return result;
}

void DockerBackend::onContainersUpdated(const Containers &containers)
{
    if (containers != m_containers)
    {
        m_containers = containers;
        emit containersChanged();
    }
}
