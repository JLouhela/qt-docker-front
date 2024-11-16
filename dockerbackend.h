#ifndef DOCKERBACKEND_H
#define DOCKERBACKEND_H

#include <QObject>
#include <QtQml>
#include <QTimer>
#include "dockerapi.h"


class DockerBackend : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QStringList containers READ containers)
public:
    explicit DockerBackend(QObject *parent = nullptr);
    QStringList containers();
signals:
    void runningContainersCountUpdated(int count);

private slots:
    void onContainersUpdated(const QStringList& containers);


private:
    void setupPolling();
    void pollContainerStatus();
    QStringList m_containers;
    QTimer m_timer;
    DockerAPI m_dockerAPI;
    bool m_connected{false};
};

#endif // DOCKERBACKEND_H
