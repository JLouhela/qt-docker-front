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
    property list<string> containerNames : []

    DockerBackend {
        id: dockerBackend
        onRunningContainersCountUpdated: (count) => {
                                             appWindow.runningContainersCount = count
                                         }
        onContainersChanged: () => {
                                 appWindow.containerNames = dockerBackend.containers
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
                    text: "x stopped containers"
                }
            }
        }

        GroupBox {
            id: containerToolBox
            title: qsTr("Container view")
            Layout.minimumWidth: containerLayout.Layout.minimumWidth + 30

            ColumnLayout {
                id: containerToolLayout
                RowLayout {
                    id: containerLayout

                    Label { text: qsTr("Container") }

                    ComboBox {
                        model: appWindow.containerNames
                    }

                    Button {
                        text: "Start/Stop"
                        enabled: false
                    }
                    Button {
                        text: ">"
                        enabled: false
                    }
                    Button {
                        text: "Resource monitor"
                        enabled: false
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
                            text: "IMAGE_PLACEHOLDER"
                        }
                        Label {
                            text: "Uptime:"
                            Layout.preferredWidth: 40
                        }
                        Label {
                            text: "UPTIME_PLACEHOLDER"
                        }
                        Label {
                            text: "Something:"
                            Layout.preferredWidth: 40
                        }
                        Label {
                            text: "SOMETHING_PLACEHOLDER"
                        }
                    }
                }
            }

        }

    }
}
