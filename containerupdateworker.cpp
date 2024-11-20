#include "containerupdateworker.h"
#include "dockerapi.h"

ContainerUpdateWorker::ContainerUpdateWorker(QObject *parent)
    : QObject{parent}
    , m_dockerAPI{new DockerAPI(this)}
{
    const bool connected = m_dockerAPI->createSocket();
    if (connected)
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
    m_dockerAPI->queryContainer(containerName);
}
