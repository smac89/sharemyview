import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.15
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
            // Updates the screenshot area to only the visible portions
            const x = Math.max(0, root.x);
            const y = Math.max(0, root.y);
            const w = Math.min(root.width + (root.x > 0 ? 0 : root.x), Screen.width - root.x);
            const h = Math.min(root.height + (root.y > 0 ? 0 : root.y), Screen.height - root.y);
            AppCore.takeScreenshot(Qt.rect(x, y, w, h), AppData.screenshot);
        }

        recordingCallback: () => {
            console.warn("Recording not implemented");
        }

        streamCallback: () => {
            console.warn("Stream not implemented");
        }
    }

    Rectangle {
        id: windowFrame

        property int mode
        property bool mediaListOpen: false
        property bool modeSettingsOpen: false
        readonly property bool modalIsOpen: mediaListOpen || modeSettingsOpen

        radius: 5
        border.color: palette.text
        border.width: 2
        color: "transparent"

        CloseButton {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.topMargin: 30
            width: 30
            height: 30
            visible: !windowFrame.modalIsOpen
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
            enabled: !windowFrame.modalIsOpen
            spacing: 0
            Rectangle {
                width: 150
                height: 50
                radius: 5
                color: "transparent"
                // TODO: dim border when disabled
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

        MediaList {
            id: mediaList
            parent: windowFrame
            topMargin: (parent.height - height) / 2
            rightPadding: background.anchors.rightMargin
            width: Math.min(parent.width - 100, 400)
            height: parent.height - recordingControls.height * 2
            mode: parent.mode
            visible: windowFrame.mediaListOpen
            onClosed: {
                windowFrame.mediaListOpen = false;
            }
        }

        RecordingControls {
            id: recordingControls
            width: parent.width - 20
            height: 50
            visible: !windowFrame.modalIsOpen
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.margins: 10
            mode: parent.mode
            drawerOpen: parent.mediaListOpen
            onTakeScreenshot: {
                mediaCapture.mediaCaptureRequested(CaptureMode.Screenshot);
            }
            onOpenRecordMenu: open => {
                parent.mediaListOpen = open;
            }
        }

        DragHandler {
            id: drag
            target: null
            enabled: !windowFrame.modalIsOpen
            onGrabChanged: (transition, pt) => {
                if (transition === EventPoint.GrabExclusive && pt.state === EventPoint.Updated) {
                    // https://codereview.qt-project.org/c/qt/qtbase/+/219277
                    console.log("Start system move");
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
                if (modalIsOpen) {
                    return rgba("#1a1918", 0.8);
                }
                if (root.resizing || drag.active) {
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
}
