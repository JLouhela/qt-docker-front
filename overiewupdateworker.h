#ifndef OVERIEWUPDATEWORKER_H
#define OVERIEWUPDATEWORKER_H

#include <QObject>

class DockerAPI;

class OverviewUpdateWorker : public QObject
{
    Q_OBJECT
public:
    explicit OverviewUpdateWorker(QObject *parent = nullptr);

public slots:
    void queryContainerUpdate();
    void onContainersUpdated(const QStringList& containers);

signals:
    void containersUpdated(const QStringList& containers);

private:
    DockerAPI* m_dockerAPI;
};

#endif // OVERIEWUPDATEWORKER_H
