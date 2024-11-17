#ifndef CONTAINER_H
#define CONTAINER_H

#include <QVector>

struct Container
{
    enum class State
    {
        UNKNOWN,
        STOPPED,
        PAUSED,
        RESTARTING,
        RUNNING
    };

    QString name;
    State state{State::UNKNOWN};
};

inline bool operator==(const Container& lhs, const Container& rhs)
{
    return lhs.name == rhs.name && lhs.state == rhs.state;
}

using Containers = QVector<Container>;

#endif // CONTAINER_H
