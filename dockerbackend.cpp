#include "dockerbackend.h"
#include "overviewupdateworker.h"
#include "containerupdateworker.h"
#include <QThreadPool>

namespace
{
}

DockerBackend::DockerBackend(QObject *parent)
    : QObject{parent}
{

    // Worker to update overview (available containers)
    auto* overviewUpdateWorker = new OverviewUpdateWorker();
    overviewUpdateWorker->moveToThread(&m_overviewPollingThread);
    QObject::connect(&m_overviewPollingThread, &QThread::finished, overviewUpdateWorker, &QObject::deleteLater);
    QObject::connect(&m_overviewTimer, &QTimer::timeout, this, &DockerBackend::onOverviewTimerTriggered);
    QObject::connect(this, &DockerBackend::overviewQueryRequest, overviewUpdateWorker, &OverviewUpdateWorker::queryOverviewUpdate);
    QObject::connect(overviewUpdateWorker, &OverviewUpdateWorker::containersUpdated, this, &DockerBackend::onContainersUpdated);
    m_overviewPollingThread.start();
    m_overviewTimer.start(1000);

    // Worker to update stats for individual container
    auto* containerUpdateWorker = new ContainerUpdateWorker();
    containerUpdateWorker->moveToThread(&m_containerPollingThread);
    QObject::connect(&m_containerPollingThread, &QThread::finished, containerUpdateWorker, &QObject::deleteLater);
    QObject::connect(& m_containerTimer, &QTimer::timeout, this, &DockerBackend::onContainerTimerTriggered);
    QObject::connect(this, &DockerBackend::containerQueryRequest, containerUpdateWorker, &ContainerUpdateWorker::queryContainerUpdate);
    QObject::connect(containerUpdateWorker, &ContainerUpdateWorker::containerUpdated, this, &DockerBackend::onContainerUpdated);
    m_containerPollingThread.start();

    // Fire overview query immediately on launch
    overviewQueryRequest();
}

DockerBackend::~DockerBackend()
{
    m_containerPollingThread.quit();
    m_overviewPollingThread.quit();
    m_containerPollingThread.wait();
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

double DockerBackend::currentContainerCpuUsage()
{
    return m_currentContainerInfo.cpuUsagePercentage;
}

double DockerBackend::currentContainerMemoryUsage()
{
    return m_currentContainerInfo.memoryUsageMiB;
}

double DockerBackend::currentContainerMemoryPercentage()
{
    return m_currentContainerInfo.memoryUsagePercentage;
}

QString DockerBackend::currentContainerImage()
{
    const auto containerIt = std::find_if(
        m_containers.cbegin(), m_containers.cend(), [this](const Container& container){
            return container.name == m_currentActiveContainer;
        });

    if (containerIt == m_containers.end())
    {
        return "N/A";
    }
    return containerIt->image;
}

QString DockerBackend::currentContainerId()
{
    const auto containerIt = std::find_if(
        m_containers.cbegin(), m_containers.cend(), [this](const Container& container){
            return container.name == m_currentActiveContainer;
        });

    if (containerIt == m_containers.end())
    {
        return "N/A";
    }
    return containerIt->id;
}

QString DockerBackend::currentContainerStatus()
{
    const auto containerIt = std::find_if(
        m_containers.cbegin(), m_containers.cend(), [this](const Container& container){
            return container.name == m_currentActiveContainer;
        });

    if (containerIt == m_containers.end())
    {
        return "N/A";
    }
    return containerIt->status;
}

void DockerBackend::onContainersUpdated(const Containers &containers)
{
    if (containers != m_containers)
    {
        m_containers = containers;
        emit containersChanged();
    }
}

void DockerBackend::onContainerUpdated(const ContainerInfo& containerInfo)
{
    // Only accept responses to currently active selection
    if (m_currentActiveContainer == containerInfo.name)
    {
        m_currentContainerInfo = containerInfo;
        emit containerInfoChanged();
    }
}

void DockerBackend::switchActiveContainer(const QString& containerName)
{
    m_currentActiveContainer = containerName;

    const auto containerIt = std::find_if(
        m_containers.cbegin(), m_containers.cend(), [&containerName](const Container& container){
        return container.name == containerName;
    });

    if (containerIt == m_containers.end())
    {
        qDebug() << containerName << " not found from active containers";
        return;
    }

    if (containerIt->state != Container::State::RUNNING)
    {   // Stats are only provided for running containers
        // If container is not running, polling can be but on hold.
        m_containerTimer.stop();
        m_currentContainerInfo = ContainerInfo{};
    }
    else if (!m_containerTimer.isActive())
    {
        // Docker sends oneshot stats once per two seconds
        m_containerTimer.start(2000);
        m_currentContainerInfo = ContainerInfo{};
        onContainerTimerTriggered();
    }
}

void DockerBackend::onContainerTimerTriggered()
{
    if (m_currentActiveContainer == "")
    {
        return;
    }
    emit containerQueryRequest(m_currentActiveContainer);
}


void DockerBackend::onOverviewTimerTriggered()
{
    emit overviewQueryRequest();
}
