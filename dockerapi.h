#ifndef DOCKERAPI_H
#define DOCKERAPI_H

#include <QObject>
#include <QLocalSocket>
#include "container.h"

class DockerAPI : public QObject
{
    Q_OBJECT
public:
    explicit DockerAPI(QObject *parent = nullptr);
    ~DockerAPI() override;
    void queryRunningContainers();
    bool connect();

signals:
    void runningContainersReady(const Containers& containers);

private slots:

private:
    QLocalSocket* m_socket;

};

#endif // DOCKERAPI_H
