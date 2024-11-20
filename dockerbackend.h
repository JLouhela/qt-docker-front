#ifndef DOCKERBACKEND_H
#define DOCKERBACKEND_H

#include <QObject>
#include <QtQml>
#include <QThread>
#include <QTimer>
#include "container.h"
#include "containerinfo.h"

class DockerBackend : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QStringList containers READ containers)
    Q_PROPERTY(int runningContainersCount READ runningContainersCount)
    Q_PROPERTY(int stoppedContainersCount READ stoppedContainersCount)
    Q_PROPERTY(double currentContainerCpuUsage READ currentContainerCpuUsage)
    Q_PROPERTY(double currentContainerMemoryPercentage READ currentContainerMemoryPercentage)
    Q_PROPERTY(double currentContainerMemoryUsage READ currentContainerMemoryUsage)
    Q_PROPERTY(QString currentContainerImage READ currentContainerImage)
    Q_PROPERTY(QString currentContainerId READ currentContainerId)
    Q_PROPERTY(QString currentContainerStatus READ currentContainerStatus)

public:
    explicit DockerBackend(QObject *parent = nullptr);
    ~DockerBackend() override;

    Q_INVOKABLE void switchActiveContainer(const QString& containerName);

    QStringList containers();
    int runningContainersCount();
    int stoppedContainersCount();
    double currentContainerCpuUsage();
    double currentContainerMemoryPercentage();
    double currentContainerMemoryUsage();
    QString currentContainerImage();
    QString currentContainerId();
    QString currentContainerStatus();

signals:
    void containersChanged();
    void containerInfoChanged();
    void containerQueryRequest(const QString& container);

private slots:
    void onContainersUpdated(const Containers& containers);
    void onContainerUpdated(const ContainerInfo& containerInfo);
    void onContainerTimerTriggered();

private:
    Containers m_containers;
    QTimer m_overviewTimer;
    QTimer m_containerTimer;
    QThread m_overviewPollingThread;
    QThread m_containerPollingThread;
    QString m_currentActiveContainer;
    ContainerInfo m_currentContainerInfo;
};

#endif // DOCKERBACKEND_H
