#ifndef OVERIEWUPDATEWORKER_H
#define OVERIEWUPDATEWORKER_H

#include <QObject>
#include "container.h"

class DockerAPI;

class OverviewUpdateWorker : public QObject
{
    Q_OBJECT
public:
    explicit OverviewUpdateWorker(QObject *parent = nullptr);

public slots:
    void queryContainerUpdate();
    void onContainersUpdated(const Containers& containers);

signals:
    void containersUpdated(const Containers& containers);

private:
    DockerAPI* m_dockerAPI;
};

#endif // OVERIEWUPDATEWORKER_H
