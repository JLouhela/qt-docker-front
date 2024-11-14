#include "dockerbackend.h"

DockerBackend::DockerBackend(QObject *parent)
    : QObject{parent}
{}

QStringList DockerBackend::containers()
{
    return QStringList("paska");
}
