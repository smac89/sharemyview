import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import smv.app.CaptureMode 1.0
import smv.app.AppCore 1.0
import smv.app.AppData 1.0
import easy.colors 1.0
import "qrc:/components"
import "qrc:/components/settings"

// https://doc.qt.io/qtcreator/creator-quick-ui-forms.html
// https://doc.qt.io/qt-5/qml-qtqml-qt.html
// https://doc.qt.io/qt-5/qtqml-javascript-functionlist.html

ApplicationWindow {
    id: root
    title: "Share My View"
    color: "transparent"
    minimumWidth: 480
    minimumHeight: minimumWidth
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowSystemMenuHint
    visible: true
    background: windowFrame

    property bool resizing: false

    WindowTracking {
        onTargetResized: (width, height) => {
            root.width = width;
            root.height = height;
        }
        onTargetMoved: (x, y) => {
            root.x = x;
            root.y = y;
        }
    }

    MediaCaptureHandler {
        id: mediaCapture
        screenshotCallback: () => {
            const x = Math.max(0, root.x);
            const y = Math.max(0, root.y);
            const w = Math.min(root.width + (root.x > 0 ? 0 : root.x), Screen.width - root.x);
            const h = Math.min(root.height + (root.y > 0 ? 0 : root.y), Screen.height - root.y);
            AppCore.takeScreenshot(Qt.rect(x, y, w, h), AppData.screenshot);
        }

        recordingCallback: () => {
            console.log("Recording not implemented");
        }

        streamCallback: () => {
            console.log("Stream not implemented");
        }
    }

    Rectangle {
        id: windowFrame

        property int mode
        property bool drawerOpen: false
        property bool modeSettingsOpen: false

        radius: 5
        border.color: palette.text
        border.width: 4
        color: parent.color

        CloseButton {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.topMargin: 30
            width: 30
            height: 30
            visible: !(windowFrame.drawerOpen || windowFrame.modeSettingsOpen)
        }

        ModeSettings {
            anchors.centerIn: parent
            width: recordingControls.controlsWidth + background.anchors.rightMargin - 50
            height: {
                // https://doc.qt.io/qt-5/qtqml-syntax-propertybinding.html
                const h = parent.height - recordingControls.height * 2;
                // TODO: control height better
                return h > 400 ? 400 : h;
            }
            visible: windowFrame.modeSettingsOpen
            mode: parent.mode
            onClosed: {
                parent.modeSettingsOpen = false;
            }
        }

        AutoSizeRowLayout {
            anchors.centerIn: parent
            spacing: 0
            Rectangle {
                width: 150
                height: 50
                radius: 5
                color: "transparent"
                border.color: palette.text
                border.width: 2

                ModeSelect {
                    anchors.fill: parent
                    onModeChanged: {
                        windowFrame.mode = mode;
                    }
                }
            }

            SettingsButton {
                buttonColor: palette.base
                onClicked: {
                    windowFrame.modeSettingsOpen = true;
                }
            }
        }

        ControlSettings {
            parent: windowFrame
            width: recordingControls.controlsWidth + background.anchors.rightMargin - 50
            height: {
                // https://doc.qt.io/qt-5/qtqml-syntax-propertybinding.html
                const h = parent.height - recordingControls.height * 2;
                // TODO: control height better
                return h > 400 ? 400 : h;
            }
            visible: windowFrame.drawerOpen
            mode: parent.mode
            background: Rectangle {
                color: palette.base
                radius: 10
                border.width: 2
                border.color: palette.text
                anchors.fill: parent
                anchors.rightMargin: (windowFrame.width - recordingControls.controlsWidth) / 2 + 25
            }
            onClosed: {
                parent.drawerOpen = false;
            }
        }

        RecordingControls {
            id: recordingControls
            width: parent.width - 20
            height: 50
            visible: !(windowFrame.drawerOpen || windowFrame.modeSettingsOpen)
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.margins: 10
            mode: parent.mode
            drawerOpen: parent.drawerOpen
            onTakeScreenshot: {
                mediaCapture.mediaCaptureRequested(CaptureMode.Screenshot);
            }
            onOpenRecordingSettings: open => {
                parent.drawerOpen = open;
            }
        }

        DragHandler {
            id: pointer
            target: null
            enabled: !(windowFrame.drawerOpen || windowFrame.modeSettingsOpen)
            onGrabChanged: (transition, pt) => {
                if (transition === EventPoint.GrabExclusive && pt.state === EventPoint.Updated) {
                    console.log("Start system move");
                    // https://codereview.qt-project.org/c/qt/qtbase/+/219277
                    root.startSystemMove();
                }
            }
        }

        Behavior on color {
            ColorAnimation {
                duration: 150
            }
        }

        Component.onCompleted: {
            color = Qt.binding(function () {
                if (drawerOpen || modeSettingsOpen) {
                    return rgba("#1a1918", 0.8);
                }
                if (root.resizing || pointer.active) {
                    return rgba("#1a1918", 0.4);
                }
                switch (mode) {
                case CaptureMode.Screenshot:
                    return rgba("#058e0e", 0.7);
                case CaptureMode.Record:
                    return rgba("#c5066f", 0.7);
                case CaptureMode.Stream:
                    return rgba("#3339ee", 0.7);
                default:
                    return "transparent";
                }
            });
        }
    }

    ResizeGrip {
        position: Item.TopLeft
        z: 100
        onDragStarted: {
            root.startSystemResize(edges);
            root.resizing = true;
        }
        onDragEnded: root.resizing = false
        anchors.horizontalCenter: parent.left
        anchors.verticalCenter: parent.top
    }

    ResizeGrip {
        position: Item.TopRight
        z: 100
        onDragStarted: {
            root.startSystemResize(edges);
            root.resizing = true;
        }
        onDragEnded: root.resizing = false
        anchors.horizontalCenter: parent.right
        anchors.verticalCenter: parent.top
    }

    ResizeGrip {
        position: Item.BottomLeft
        z: 100
        onDragStarted: {
            root.startSystemResize(edges);
            root.resizing = true;
        }
        onDragEnded: root.resizing = false
        anchors.horizontalCenter: parent.left
        anchors.verticalCenter: parent.bottom
    }

    ResizeGrip {
        position: Item.BottomRight
        z: 100
        onDragStarted: {
            root.startSystemResize(edges);
            root.resizing = true;
        }
        onDragEnded: root.resizing = false
        anchors.horizontalCenter: parent.right
        anchors.verticalCenter: parent.bottom
    }

    // Behavior on opacity {
    //     SmoothedAnimation {
    //         duration: 3000
    //         velocity: 0.3
    //     }
    // }
}
