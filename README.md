# Qt exercise

## QtQuick application displaying sample stats from docker containers.

Application connects tp unix socket serving docker engine API. It polls data periodically from the docker engine socket and displays it on the screen. If there are no containers, or docker unix socket is not available in the default location (var/run/docker.sock), the application will only display N/A values with empty dropdown menu. If socket is accessible and there are containers available (both stopped and running are accepted) in the docker engine, the application will display some basic information of the each container. CPU and Memory stats are only available for running containers.

![Image of the application](screenshot_qt_front.png?raw=true "QtDockerFront")


## Tech

Uses two background threads (one for overview of available containers and one for stats) to send HTTP GET messages into docker socket, receive responses and parse the json content from the response. Serves as an examples of how to 
- Use C++ backend from qml file
- Use QThreads with worker scheme
- Use QTimer for periodical polling
- Use signals and slots for data propagation
- Use QMutex and QMutexLocker to synchronize socket usage
- Use QLocalSocket to send to and read from a unix socket
    - Proper handling of chunked encoding included (https://en.wikipedia.org/wiki/Chunked_transfer_encoding)!
- Use QJsonDocument to parse json responses (not the best example of error handling though!)

See docker engine API details in https://docs.docker.com/reference/api/engine/


## Prerequirements

Qt6 required for building the solution

Only tested in native Linux, should also work in WSL. Windows support would require changing socket url to pipe location, which itself is a small task, but requires validation on Windows platform. I don't have the environment available right now.

**Requires either root permission, or user must have docker group membership (preferred) in order to access the socket.**
