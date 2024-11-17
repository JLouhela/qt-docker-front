#include "overiewupdateworker.h"
#include "dockerapi.h"

OverviewUpdateWorker::OverviewUpdateWorker(QObject *parent)
    : QObject{parent}
    , m_dockerAPI{new DockerAPI(this)}
{
    const bool connected = m_dockerAPI->connect();
    if (connected)
    {
        connect(m_dockerAPI, &DockerAPI::runningContainersReady, this, &OverviewUpdateWorker::onContainersUpdated);
    }
}

void OverviewUpdateWorker::onContainersUpdated(const QStringList& containers)
{
    emit containersUpdated(containers);
}

void OverviewUpdateWorker::queryContainerUpdate()
{
    m_dockerAPI->queryRunningContainers();
}