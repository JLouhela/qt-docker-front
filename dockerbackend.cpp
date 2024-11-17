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
    connect(&m_overviewPollingThread, &QThread::finished, overviewUpdateWorker, &QObject::deleteLater);
    QObject::connect(&m_timer, &QTimer::timeout, overviewUpdateWorker, &OverviewUpdateWorker::queryContainerUpdate);
    QObject::connect(overviewUpdateWorker, &OverviewUpdateWorker::containersUpdated, this, &DockerBackend::onContainersUpdated);
    m_overviewPollingThread.start();
    m_timer.start(1000);

}

DockerBackend::~DockerBackend()
{
    m_overviewPollingThread.quit();
    m_overviewPollingThread.wait();
}

QStringList DockerBackend::containers()
{
    return m_containers;
}

void DockerBackend::onContainersUpdated(const QStringList &containers)
{
    m_containers = containers;
    // TODO keep only one signal and provide getters
    emit runningContainersCountUpdated(m_containers.size());
    emit containersChanged();
}
