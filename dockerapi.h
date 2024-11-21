#ifndef DOCKERAPI_H
#define DOCKERAPI_H

#include <QObject>
#include <QLocalSocket>
#include "container.h"
#include "containerinfo.h"

class DockerAPI : public QObject
{
    Q_OBJECT
public:
    explicit DockerAPI(QObject *parent = nullptr);
    ~DockerAPI() override;
    void queryRunningContainers();
    void queryContainer(const QString& containerName);
    bool createSocket();

signals:
    void runningContainersReady(const Containers&);
    void containerUpdateReady(const ContainerInfo&);

private:
    void performQuery(const char* msg, std::function<void(const QJsonDocument& json)> jsonHandler);
    bool connectSocket();
    QString exhaustSocket();

    QLocalSocket* m_socket;
};

#endif // DOCKERAPI_H
