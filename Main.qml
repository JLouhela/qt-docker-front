import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtDockerFront

ApplicationWindow {
    id: appWindow

    visible: true
    title: qsTr("QtDockerFront")
    readonly property int margin: 11

    property int runningContainersCount : 0
    property int stoppedContainersCount : 0
    property real cpuUsagePercentage : 0.0
    property string containerImage : "N/A"
    property string containerId : "N/A"
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
        GroupBox {
            id: rowBox
            title: qsTr("Overview")

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
                    Layout.maximumWidth: 20
                }
                Layout.minimumWidth: mainLayout.Layout.minimumWidth

                ComboBox {
                    model: appWindow.containerNames
                    onCurrentIndexChanged: {
                        dockerBackend.switchActiveContainer(appWindow.containerNames[currentIndex])
                        appWindow.containerImage = dockerBackend.currentContainerImage
                        appWindow.containerId = dockerBackend.currentContainerId
                        appWindow.cpuUsagePercentage = 0.0
                    }
                    Layout.minimumWidth: 40;
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
                        text: "Uptime:"
                        Layout.preferredWidth: 50
                    }
                    Label {
                        text: "UPTIME_PLACEHOLDER"
                    }
                    Label {
                        text: "CPU: "
                        Layout.preferredWidth: 50
                    }
                    Label {
                        text: appWindow.cpuUsagePercentage.toFixed(4) + "%"
                    }
                }
            }
        }


    }
}
