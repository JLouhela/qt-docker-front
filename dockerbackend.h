#ifndef DOCKERBACKEND_H
#define DOCKERBACKEND_H

#include <QObject>
#include <QtQml>
#include <QTimer>


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
