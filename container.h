#ifndef CONTAINER_H
#define CONTAINER_H

#include <QVector>

struct Container
{
    enum class Status
    {
        UNKNOWN,
        EXITED,
        STOPPED,
        RUNNING
    };

    QString name;
    Status status{Status::UNKNOWN};
};

using Containers = QVector<Container>;

#endif // CONTAINER_H
