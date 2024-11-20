#ifndef OVERVIEWUPDATEWORKER_H
#define OVERVIEWUPDATEWORKER_H

#include <QObject>
#include "container.h"

class DockerAPI;

class OverviewUpdateWorker : public QObject
{
    Q_OBJECT
public:
    explicit OverviewUpdateWorker(QObject *parent = nullptr);

public slots:
    void queryOverviewUpdate();
    void onContainersUpdated(const Containers& containers);

signals:
    void containersUpdated(const Containers& containers);

private:
    DockerAPI* m_dockerAPI;
    bool m_enabled{false};
};

#endif // OVERVIEWUPDATEWORKER_H
