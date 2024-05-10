import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import smv.app.CaptureMode 1.0

Drawer {
    id: root
    dim: false
    modal: false
    transformOrigin: Popup.Left
    edge: Qt.RightEdge
    background: Debuggable {
        anchors.fill: parent
        anchors.verticalCenter: parent.verticalCenter
        OptimusRect {
            radius: 20
            bottomRightRadius: -20
            topRightRadius: -20
            hideRightBorder: true
            borderColor: palette.text
            bgColor: palette.base
            anchors.fill: parent

            Loader {
                anchors.margins: 10
                anchors.fill: parent
                active: root.visible
                sourceComponent: recentMedia
            }
        }
    }

    property int mode

    Component {
        id: recentMedia
        Item {
            anchors.fill: parent
            anchors.margins: 5

            states: [
                State {
                    name: "searching"
                },
                State {
                    name: "loading"
                },
                State {
                    name: "notFound"
                },
                State {
                    name: "loaded"
                }
            ]

            Label {
                text: qsTr("Recent Media")
                font.bold: true
                font.pointSize: 20
                font.letterSpacing: -1
                font.underline: true
                font.capitalization: Font.Capitalize
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
            }

            ProgressBar {
                visible: parent.state == "loading"
                indeterminate: true
                anchors.centerIn: parent
                width: parent.width - 40
            }

            Item {
                visible: parent.state == "notFound"
                width: parent.width * 0.75
                height: parent.height * 0.75
                anchors.centerIn: parent

                Image {
                    id: notFound
                    sourceSize.width: Math.min(parent.width, 180)
                    sourceSize.height: Math.min(parent.width, 150)
                    x: (parent.width - width) / 2
                    y: (parent.height - height) / 2
                    // anchors.centerIn: parent
                    // anchors.verticalCenter: parent.verticalCenter
                    source: "image://smv/images/nomedia.svg"
                    antialiasing: true
                    smooth: true
                    visible: false

                    SpringAnimation on y {
                        spring: 5
                        damping: 0
                        epsilon: 0.25
                        mass: 30
                        to: (parent.height - height) / 2 + 20
                        from: (parent.height - height) / 2
                        velocity: 20
                    }
                }

                ColorOverlay {
                    id: notFoundOverlay
                    anchors.fill: notFound
                    source: notFound
                    cached: false
                    color: "#8000ff00"
                    antialiasing: true
                }

                Label {
                    text: qsTr("No recent media found.")
                    font.pointSize: 16
                    font.letterSpacing: -1
                    anchors.topMargin: 20
                    anchors.horizontalCenter: notFound.horizontalCenter
                    anchors.top: notFound.bottom
                }
            }

            Component.onCompleted: {
                state = "notFound";
            }
        }
    }

    SystemPalette {
        id: palette
    }
}
