# Qt exercise

## QtQuick application displaying sample stats from docker containers.

Application connects tp unix socket serving docker engine API. It polls data periodically from the docker engine socket and displays it on the screen. If there are no containers, or docker unix socket is not available in the default location (var/run/docker.sock in Linux, \\.\pipe\docker_engine in Windows), the application will only display N/A values with empty dropdown menu. If socket is accessible and there are containers available (both stopped and running are accepted) in the docker engine, the application will display some basic information of the each container. CPU and Memory stats are only available for running containers.

![Image of the application](screenshot_qt_front.png?raw=true "QtDockerFront")


## Tech

I had some false assumptions in the beginning and had to adjust the architecture accordingly. a) I did not understand that signal/slot system is a robust way to handle asynchronous tasks and b) I did not realize that usage of socket is not allowed from different threads. I didn't want to overcomplicate this exercise and start all over. Instead, design evolved in a way that everything works, but it is not the best way to handle the issue. See lessons learned below for alternate solution.

Anyway, here is the tech used within this project. Solution uses two background threads (one for overview of available containers and one for stats) to send HTTP GET messages into docker socket, receive responses and parse the json content from the response. Serves as an examples of how to 
- Use C++ backend from qml file
- Use QThreads with worker scheme
- Use QTimer for periodical polling
- Use signals and slots for data propagation
- Use QMutex and QMutexLocker to synchronize socket usage
- Use QLocalSocket to send to and read from a unix socket
    - Proper handling of chunked encoding included (https://en.wikipedia.org/wiki/Chunked_transfer_encoding)!
- Use QJsonDocument to parse json responses (not the best example of error handling though!)

See docker engine API details in https://docs.docker.com/reference/api/engine/


## Lessons learned

As mentioned above, I would approach this problem a bit differently with gained knowledge. Instead of polling threads and separate socket instances, I would try to go with finite state machine with a request queue. Just process one request at a time in the event loop using signals and slots only.
- No Threads needed
- Single socket instance used from single thread, responses handled asynchronously via socket signals
- Less "proxy" signals to delegate data from threads towards qml


## Prerequirements

Qt6 required for building the solution

Tested and developed in native Linux, should also work in WSL. Socket path cannot be configured currently.

Quickly tested in Windows too and adjusted socket path to match docker pipe in Windows. Doesn't work as good as Linux (polling seems to react slower, local socket receives empty messags), but did not check in details why it's like this.

**Requires either root permission, or user must have docker group membership (preferred) in order to access the socket.**
