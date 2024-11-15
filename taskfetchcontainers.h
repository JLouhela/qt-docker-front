#ifndef TASKFETCHCONTAINERS_H
#define TASKFETCHCONTAINERS_H

#include <QRunnable>
#include <QStringList>

class TaskFetchContainers : public QRunnable
{
public:
    explicit TaskFetchContainers(std::function<void(const QStringList&)> callback)
        : m_callback(std::move(callback))
    {

    }
    void run() final;

private:
    std::function<void(const QStringList&)> m_callback;

};

#endif // TASKFETCHCONTAINERS_H
