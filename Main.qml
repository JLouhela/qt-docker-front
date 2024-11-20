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
    property string image : "N/A"
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
                        appWindow.image = dockerBackend.currentContainerImage
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
                        id: imageText
                        text: "Image:  "
                        Layout.preferredWidth: 50
                    }
                    Label {
                        text: appWindow.image
                        Layout.preferredWidth: containerInfoBox.Layout.preferredWidth - imageText.width
                        clip: true
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
