// Layouts tutorial: https://www.dmcinfo.com/latest-thinking/blog/id/10393/resizing-uis-with-qml-layouts

import QtGraphicalEffects 1.15
import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import smv.app.CaptureMode 1.0

Item {
    id: root

    // TODO add list of video sources
    // TODO add list of audio sources
    property int maxWidth: 640
    property int mode: CaptureMode.Screenshot
    property bool drawerOpen: false
    readonly property int controlsWidth: controls.width
    readonly property int controlsX: controls.x
    readonly property int controlsY: controls.y

    signal takeScreenshot
    signal openRecordingSettings(bool open)
    signal recordRegion(bool streaming)

    SystemPalette {
        id: palette
    }

    ColumnLayout {
        anchors.fill: parent

        Rectangle {
            id: controls
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            Layout.fillHeight: true
            border.color: palette.text
            border.width: 2
            radius: height / 2
            color: palette.base
            Layout.maximumWidth: maxWidth
            Layout.fillWidth: true

            Loader {
                id: screenshotLoader
                anchors.centerIn: parent
                sourceComponent: screenshotControls
                active: mode === CaptureMode.Screenshot
            }

            Connections {
                target: screenshotLoader.item
                function onTakeScreenshot() {
                    Qt.callLater(root.takeScreenshot);
                }
            }

            Loader {
                anchors.centerIn: parent
                sourceComponent: recordControls
                active: mode === CaptureMode.Record
                onLoaded: () => {
                    console.log("Record loaded");
                }
            }

            Loader {
                anchors.centerIn: parent
                sourceComponent: streamControls
                active: mode === CaptureMode.Stream
                onLoaded: () => {
                    console.log("Stream loaded");
                }
            }

            Button {
                id: modeDrawerButton
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                display: AbstractButton.IconOnly
                background: Rectangle {
                    color: "transparent"
                    width: 40
                    height: 40
                }
                icon {
                    source: root.drawerOpen ? "image://smv/icons/menu_on.svg" : "image://smv/icons/menu_off.svg"
                    color: palette.buttonText
                }
                onClicked: root.openRecordingSettings(!root.drawerOpen)
            }
        }
    }

    Component {
        id: screenshotControls

        Item {
            id: root

            readonly property bool hovered: clickable.containsMouse
            property int hoverWidth: 44
            property int hoverHeight: 44

            signal takeScreenshot

            containmentMask: button
            implicitWidth: hovered ? hoverWidth : 38
            implicitHeight: hovered ? hoverHeight : 38
            anchors.margins: 1

            RoundButton {
                id: button

                anchors.fill: parent
                hoverEnabled: true
                visible: false
                padding: 6

                icon {
                    color: hovered ? palette.buttonText : Qt.lighter(palette.buttonText)
                    source: "image://smv/icons/camera_on.svg"
                    height: parent.height
                    width: parent.width

                    Behavior on color {
                        ColorAnimation {
                            duration: 150
                        }
                    }
                }

                background: Rectangle {
                    color: palette.button
                    radius: width / 2
                    border.width: 1
                    border.color: Qt.darker(palette.button, 1.5)

                    Behavior on color {
                        ColorAnimation {
                            duration: 100
                        }
                    }
                }
            }

            DropShadow {
                anchors.fill: button
                horizontalOffset: 0
                verticalOffset: 0
                radius: 8
                samples: 17
                color: Qt.darker(palette.button, 1.5)
                source: button
                cached: true
            }

            MouseArea {
                id: clickable

                anchors.fill: parent
                hoverEnabled: true
                onClicked: Qt.callLater(takeScreenshot)
            }

            Behavior on implicitWidth {

                animation: AnimateSize {}
            }

            Behavior on implicitHeight {

                animation: AnimateSize {}
            }
        }
    }

    component AnimateSize: NumberAnimation {
        easing.type: Easing.InOutQuad
        duration: 150
    }

    Component {
        id: streamControls

        Item {}
    }

    Component {
        id: recordControls

        Item {}
    }

    component ActionButton: Item {
        required property string activeIcon
        required property string inactiveIcon
        property bool active: true

        signal clicked

        containmentMask: button
        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        Layout.preferredWidth: 38
        Layout.preferredHeight: 38
        Component.onCompleted: {
            console.log("button width:", button.width);
        }

        RoundButton {
            // Behavior on color {
            //     ColorAnimation {
            //         duration: 100
            //     }
            // }

            id: button

            anchors.fill: parent
            hoverEnabled: true
            icon.color: hovered ? palette.buttonText : Qt.lighter(palette.buttonText)
            icon.source: active ? activeIcon : inactiveIcon
            opacity: enabled ? 1 : 0.3
            visible: false

            background: Rectangle {
                color: palette.button
                radius: width / 2
                border.width: 1.5
                border.color: Qt.darker(palette.button, 1.5)
            }
        }

        DropShadow {
            anchors.fill: button
            horizontalOffset: 0
            verticalOffset: 0
            radius: 12
            samples: 17
            color: Qt.darker(palette.button)
            source: button
            cached: true

            MouseArea {
                anchors.fill: parent
                onClicked: mouse => {
                    active = !active;
                    parent.parent.clicked();
                }
            }
        }
    }
}
