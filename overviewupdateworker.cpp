#include "overviewupdateworker.h"
#include "dockerapi.h"

OverviewUpdateWorker::OverviewUpdateWorker(QObject *parent)
    : QObject{parent}
    , m_dockerAPI{new DockerAPI(this)}
{
    m_enabled = m_dockerAPI->createSocket();
    if (m_enabled)
    {
        connect(m_dockerAPI, &DockerAPI::runningContainersReady, this, &OverviewUpdateWorker::onContainersUpdated);
    }
}

void OverviewUpdateWorker::onContainersUpdated(const Containers& containers)
{
    emit containersUpdated(containers);
}

void OverviewUpdateWorker::queryOverviewUpdate()
{
    // Avoid polluting logs with warnings if socket is not accessible
    if (m_enabled)
    {
        m_dockerAPI->queryRunningContainers();
    }
}
