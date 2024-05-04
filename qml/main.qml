// https://doc.qt.io/qtcreator/creator-quick-ui-forms.html
// https://doc.qt.io/qt-5/qml-qtqml-qt.html
// https://doc.qt.io/qt-5/qtqml-javascript-functionlist.html

import QtQuick 2.15
import QtGraphicalEffects 1.15
import QtQuick.Controls 2.15
import smv.app.CaptureMode 1.0
import smv.app.ScreenshotFormat 1.0
import smv.app.AppCore 1.0
import smv.app.AppData 1.0
import easy.colors 1.0
import "qrc:/components"
import "qrc:/components/settings"

ApplicationWindow {
    // smvApp.qquickWindowReady(rootWindow);
    id: rootWindow
    readonly property int initialWidth: 1024
    readonly property int initialHeight: 768
    readonly property int initialX: rootWindow.x
    readonly property int initialY: rootWindow.y
    property int targetWidth: initialWidth
    property int targetHeight: initialHeight
    property int targetPosX: initialX
    property int targetPosY: initialY

    width: initialWidth
    height: initialHeight
    minimumWidth: 480
    minimumHeight: minimumWidth
    visible: true
    title: "Share My View"
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowSystemMenuHint
    background: windowFrame

    MediaCaptureHandler {
        id: mediaCapture
        screenshotCallback: () => {
            AppCore.takeScreenshot(Qt.rect(rootWindow.x, rootWindow.y, rootWindow.width, rootWindow.height), AppData.screenshot);
        }
        recordingCallback: () => {
            console.log("Recording not implemented");
        }

        streamCallback: () => {
            console.log("Stream not implemented");
        }
    }

    SequentialAnimation {
        id: reparentAnimation

        onFinished: {
            console.log("Reparent finished");
        }

        SmoothedAnimation {
            target: rootWindow
            property: "opacity"
            duration: 300
            to: 0
        }

        ParallelAnimation {
            PropertyAction {
                target: rootWindow
                property: "x"
                value: targetPosX
            }

            PropertyAction {
                target: rootWindow
                property: "y"
                value: targetPosY
            }
        }

        SmoothedAnimation {
            target: rootWindow
            property: "opacity"
            duration: 500
            to: 1
        }
    }

    Connections {
        // reparentAnimation.restart()
        // animateSizePos.restart()
        // rootWindow.width = sz.width
        // rootWindow.height = sz.height
        // animateSizePos.restart()
        // reparentAnimation.restart()
        // rootWindow.width = sz.width
        // rootWindow.height = sz.height
        // animateSizePos.restart()

        // https://doc.qt.io/qt-5/qtqml-cppintegration-interactqmlfromcpp.html
        function onTargetWindowMoved(pos: point) {
            console.log("Target moved:", pos);
            // rootWindow.x = pos.x
            // rootWindow.y = pos.y
            targetPosX = pos.x;
            targetPosY = pos.y;
        }

        function onTargetWindowResized(sz: size) {
            console.log("Target resized:", sz);
        }

        function onTargetWindowChanged(sz: size, pos: point) {
            console.log("Target changed:", sz, pos);
            // rootWindow.x = pos.x
            // rootWindow.y = pos.y
            targetPosX = pos.x;
            targetPosY = pos.y;
        }
        target: AppCore
    }

    Rectangle {
        id: windowFrame

        property int mode
        property bool drawerOpen: false
        property bool modeSettingsOpen: false
        property bool resizing: false

        // width: initialWidth
        // height: initialHeight
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
                buttonColor: $.invert(windowFrame.color)
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

        ResizeGrip {
            position: ResizeGrip.Pos.TL
            onDragStarted: {
                rootWindow.startSystemResize(edges);
                parent.resizing = true;
            }
            onDragEnded: parent.resizing = false
            anchors.left: parent.left
            anchors.top: parent.top
        }

        ResizeGrip {
            position: ResizeGrip.Pos.TR
            onDragStarted: {
                rootWindow.startSystemResize(edges);
                parent.resizing = true;
            }
            onDragEnded: parent.resizing = false
            anchors.right: parent.right
            anchors.top: parent.top
        }

        ResizeGrip {
            position: ResizeGrip.Pos.BL
            onDragStarted: {
                rootWindow.startSystemResize(edges);
                parent.resizing = true;
            }
            onDragEnded: parent.resizing = false
            anchors.left: parent.left
            anchors.bottom: parent.bottom
        }

        ResizeGrip {
            position: ResizeGrip.Pos.BR
            onDragStarted: {
                rootWindow.startSystemResize(edges);
                parent.resizing = true;
            }
            onDragEnded: parent.resizing = false
            anchors.right: parent.right
            anchors.bottom: parent.bottom
        }

        // MouseArea {
        //     anchors.fill: parent
        //     acceptedButtons: Qt.LeftButton
        //     enabled: pointer.pressed
        //     propagateComposedEvents: true

        //     onPressed: {
        //         mouse.accepted = false;
        //     }

        //     onReleased: {
        //         if (pointer.pressed) {
        //             console.log("End system move");
        //         }
        //     }
        // }

        TapHandler {
            id: pointer
            dragThreshold: 5
            gesturePolicy: TapHandler.DragThreshold
            enabled: !(windowFrame.drawerOpen || windowFrame.modeSettingsOpen)
            // grabPermissions: PointerHandler.TakeOverForbidden
            onGrabChanged: (transition, pt) => {
                console.log("Grab changed", transition, pt.state);
                if (transition === EventPoint.GrabPassive) {
                    console.log("Start system move");
                    // https://codereview.qt-project.org/c/qt/qtbase/+/219277
                    rootWindow.startSystemMove();
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
                if (resizing || pointer.active) {
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
}
