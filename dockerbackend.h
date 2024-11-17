#ifndef DOCKERBACKEND_H
#define DOCKERBACKEND_H

#include <QObject>
#include <QtQml>
#include <QThread>
#include <QTimer>
#include "container.h"


class DockerBackend : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QStringList containers READ containers)
public:
    explicit DockerBackend(QObject *parent = nullptr);
    ~DockerBackend() override;
    QStringList containers();
signals:
    void runningContainersCountUpdated(int count);
    void containersChanged();

private slots:
    void onContainersUpdated(const Containers& containers);


private:
    Containers m_containers;
    QTimer m_timer;
    QThread m_overviewPollingThread;
};

#endif // DOCKERBACKEND_H
