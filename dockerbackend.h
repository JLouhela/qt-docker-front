#ifndef DOCKERBACKEND_H
#define DOCKERBACKEND_H

#include <QObject>
#include <QtQml>
#include <QTimer>

// TODO own header
class TaskUpdateContainerStatus : public QRunnable
{
public:
    explicit TaskUpdateContainerStatus(std::function<void(int)> callback)
        : m_callback(std::move(callback))
    {

    }

    void run() override
    {
        m_callback(static_cast<int>(QRandomGenerator::global()->generate()));
    }
private:
    std::function<void(int)> m_callback;
};

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

public slots:
    void pollContainerStatus();


private:
    QStringList m_containers;
    QTimer m_timer;

};

#endif // DOCKERBACKEND_H
