#ifndef DOCKERBACKEND_H
#define DOCKERBACKEND_H

#include <QObject>
#include <QtQml>

class DockerBackend : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QStringList containers READ containers)
public:
    explicit DockerBackend(QObject *parent = nullptr);
    QStringList containers();
signals:

private:
    QStringList m_containers;

};

#endif // DOCKERBACKEND_H
