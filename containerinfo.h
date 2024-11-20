#ifndef CONTAINERINFO_H
#define CONTAINERINFO_H

#include <QtTypes>
#include <QString>

struct ContainerInfo
{
    QString name;
    double cpuUsagePercentage{0.0};
    double memoryUsagePercentage{0.0};
    double memoryUsageMiB{0};
};

#endif // CONTAINERINFO_H
