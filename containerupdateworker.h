#ifndef CONTAINERUPDATEWORKER_H
#define CONTAINERUPDATEWORKER_H

#include <QObject>
#include "containerinfo.h"

class DockerAPI;

class ContainerUpdateWorker : public QObject
{
    Q_OBJECT
public:
    explicit ContainerUpdateWorker(QObject *parent = nullptr);

public slots:
    void queryContainerUpdate(const QString& containerName);
    void onContainerUpdated(const ContainerInfo& containerInfo);

signals:
    void containerUpdated(const ContainerInfo& containerInfo);

private:
    DockerAPI* m_dockerAPI;
};

#endif // CONTAINERUPDATEWORKER_H
