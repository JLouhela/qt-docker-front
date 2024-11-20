#include "containerupdateworker.h"
#include "dockerapi.h"

ContainerUpdateWorker::ContainerUpdateWorker(QObject *parent)
    : QObject{parent}
    , m_dockerAPI{new DockerAPI(this)}
{
    m_enabled = m_dockerAPI->createSocket();
    if (m_enabled)
    {
        connect(m_dockerAPI, &DockerAPI::containerUpdateReady, this, &ContainerUpdateWorker::onContainerUpdated);
    }
}

void ContainerUpdateWorker::onContainerUpdated(const ContainerInfo& containerInfo)
{
    emit containerUpdated(containerInfo);
}

void ContainerUpdateWorker::queryContainerUpdate(const QString& containerName)
{
    // Avoid polluting logs with warnings if socket is not accessible
    if (m_enabled)
    {
        m_dockerAPI->queryContainer(containerName);
    }
}
