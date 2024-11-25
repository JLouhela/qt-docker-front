import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtDockerFront

ApplicationWindow {
    id: appWindow

    visible: true
    maximumHeight: minimumHeight
    maximumWidth: minimumWidth
    title: qsTr("QtDockerFront")
    readonly property int margin: 11

    property int runningContainersCount : 0
    property int stoppedContainersCount : 0
    property real cpuUsagePercentage : -1.0
    property real memoryUsagePercentage : -1.0
    property real memoryUsageMiB : -1.0
    property string containerImage : "N/A"
    property string containerId : "N/A"
    property string containerStatus : "N/A"
    property list<string> containerNames : []

    DockerBackend {
        id: dockerBackend
        onContainersChanged: () => {
                                 appWindow.containerNames = dockerBackend.containers
                                 appWindow.runningContainersCount = dockerBackend.runningContainersCount
                                 appWindow.stoppedContainersCount = dockerBackend.stoppedContainersCount
                             }
        onContainerInfoChanged: () => {
                                    appWindow.cpuUsagePercentage = dockerBackend.currentContainerCpuUsage
                                    appWindow.memoryUsagePercentage = dockerBackend.currentContainerMemoryPercentage
                                    appWindow.memoryUsageMiB = dockerBackend.currentContainerMemoryUsage
                             }
    }

    Component.onCompleted: {
        width = mainLayout.implicitWidth + 2 * margin
        height = mainLayout.implicitHeight + 2 * margin
    }

    minimumWidth: mainLayout.Layout.minimumWidth + 2 * margin
    minimumHeight: mainLayout.Layout.minimumHeight + 2 * margin

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: appWindow.margin
        Layout.minimumWidth: 700
        Label {
            text: "Overview"
        }
        GroupBox {
            id: rowBox

            ColumnLayout {
                id: overviewLayout
                anchors.fill: parent
                Label {
                    text: appWindow.runningContainersCount + " running containers"
                }
                Label {
                    text: appWindow.stoppedContainersCount + " stopped containers"
                }
            }
        }


        ColumnLayout {
            id: containerToolLayout
            RowLayout {
                id: containerLayout

                Label {
                    text: qsTr("Container")
                    Layout.maximumWidth: 12
                }
                Layout.minimumWidth: mainLayout.Layout.minimumWidth

                ComboBox {
                    model: appWindow.containerNames
                    onCurrentIndexChanged: {
                        dockerBackend.switchActiveContainer(appWindow.containerNames[currentIndex])
                        appWindow.containerImage = dockerBackend.currentContainerImage
                        appWindow.containerId = dockerBackend.currentContainerId
                        appWindow.containerStatus = dockerBackend.currentContainerStatus
                        appWindow.cpuUsagePercentage = -1.0
                        appWindow.memoryUsagePercentage = -1.0
                        appWindow.memoryUsageMiB = -1.0
                    }
                    implicitContentWidthPolicy: ComboBox.WidestText;
                }
            }
            GroupBox {
                id: containerInfoBox
                Layout.preferredWidth: containerLayout.width

                GridLayout {
                    columns: 2
                    flow: GridLayout.LeftToRight
                    anchors.fill: parent
                    Label {
                        text: "Id:  "
                        Layout.preferredWidth: 50
                    }
                    TextEdit {
                        text: appWindow.containerId
                        readOnly: true
                        selectByMouse: true
                    }
                    Label {
                        id: imageText
                        text: "Image:  "
                        Layout.preferredWidth: 50
                    }
                    TextEdit {
                        text: appWindow.containerImage
                        Layout.preferredWidth: containerInfoBox.Layout.preferredWidth - imageText.width
                        clip: true
                        readOnly: true
                        selectByMouse: true
                    }
                    Label {
                        text: "Status:  "
                        Layout.preferredWidth: 50
                    }
                    Label {
                        text: appWindow.containerStatus
                    }
                    Label {
                        text: "CPU: "
                        Layout.preferredWidth: 50
                    }
                    Label {
                        text: appWindow.cpuUsagePercentage < 0.0 ? "N/A" : (appWindow.cpuUsagePercentage.toFixed(2) + "%")
                    }
                    Label {
                        text: "Mem: "
                        Layout.preferredWidth: 50
                    }
                    Label {
                        text: appWindow.memoryUsagePercentage < 0.0 ? "N/A" : (appWindow.memoryUsagePercentage.toFixed(2) + "% (" + appWindow.memoryUsageMiB.toFixed(2) + " MiB)")
                    }
                }
            }
        }
    }
}
