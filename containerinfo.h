#ifndef CONTAINERINFO_H
#define CONTAINERINFO_H

#include <QtTypes>

struct ContainerInfo
{
    double cpuUsagePercentage{0.0};
    double memoryUsagePercentage{0.0};
    double memoryUsageMiB{0};
};

#endif // CONTAINERINFO_H
