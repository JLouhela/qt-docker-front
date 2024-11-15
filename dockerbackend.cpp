#include "dockerbackend.h"
#include <QThreadPool>

namespace
{
}

DockerBackend::DockerBackend(QObject *parent)
    : QObject{parent}
{
    // TODO : threadpool & timer
    // schedule update task every secondor
    QObject::connect(&m_timer, &QTimer::timeout, this, &DockerBackend::pollContainerStatus);
    m_timer.start(1000);

}

QStringList DockerBackend::containers()
{
    return QStringList("paska");
}

void DockerBackend::pollContainerStatus()
{
    auto* containerStatusTask = new TaskUpdateContainerStatus([this](int value){
        runningContainersCountUpdated(value);
    });
    QThreadPool::globalInstance()->start(containerStatusTask);
}
