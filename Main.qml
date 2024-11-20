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

        GroupBox {
            id: containerToolBox
            title: qsTr("Container view")
            Layout.minimumWidth: mainLayout.implicitWidth

            ColumnLayout {
                id: containerToolLayout
                RowLayout {
                    id: containerLayout

                    Label { text: qsTr("Container") }

                    ComboBox {
                        model: appWindow.containerNames
                        onCurrentIndexChanged: {
                            dockerBackend.switchActiveContainer(appWindow.containerNames[currentIndex])
                            appWindow.image = dockerBackend.currentContainerImage
                        }
                        Layout.minimumWidth: 32;
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
                            text: "Image:"
                            Layout.preferredWidth: 40
                        }
                        Label {
                            text: appWindow.image
                        }
                        Label {
                            text: "Uptime:"
                            Layout.preferredWidth: 40
                        }
                        Label {
                            text: "UPTIME_PLACEHOLDER"
                        }
                        Label {
                            text: "CPU %:"
                            Layout.preferredWidth: 40
                        }
                        Label {
                            text: appWindow.cpuUsagePercentage.toFixed(2) + "%"
                        }
                    }
                }
            }

        }

    }
}
