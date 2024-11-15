#include "taskfetchcontainers.h"

void TaskFetchContainers::run()
{
    QStringList tmp;
    tmp.push_back("asd");
    tmp.push_back("lol");
    tmp.push_back("banaani");
    m_callback(tmp);
}
