// https://doc.qt.io/qtcreator/creator-quick-ui-forms.html
// https://doc.qt.io/qt-5/qml-qtqml-qt.html
// https://doc.qt.io/qt-5/qtqml-javascript-functionlist.html

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.15
import smv.app.CaptureMode 1.0
import smv.app.ScreenshotFormat 1.0
import smv.app.AppCore 1.0
import easy.colors 1.0
import "qrc:/components"
import "qrc:/settings"

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
        target: rootWindow
        screenshotCallback: () => {
            AppCore.takeScreenshot(Qt.rect(rootWindow.x, rootWindow.y, rootWindow.width, rootWindow.height), ScreenshotFormat.PNG);
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
        property bool resizing: false

        // width: initialWidth
        // height: initialHeight
        radius: 5
        border.color: Qt.lighter(rootWindow.palette.window)
        border.width: 5
        color: parent.color

        ControlSettings {
            parent: windowFrame
            width: recordingControls.controlsWidth + background.anchors.rightMargin - 50
            height: parent.height - recordingControls.height * 4
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

        Rectangle {
            width: 150
            height: 50
            anchors.centerIn: parent
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

        DragHandler {
            id: dragHandler
            target: null
            enabled: !windowFrame.drawerOpen
            acceptedButtons: Qt.LeftButton
            dragThreshold: 10
            acceptedDevices: PointerDevice.GenericPointer
            // grabPermissions: PointerHandler.CanTakeOverFromItems | PointerHandler.CanTakeOverFromHandlersOfDifferentType | PointerHandler.ApprovesTakeOverByItems
            grabPermissions: PointerHandler.TakeOverForbidden
            // TODO: Fix bug with after resizing, cannot start dragging
            onActiveChanged: {
                // if this seems confusing where we get "active" from,
                // it's actually in the name of this handler. It's a handler for when the
                // active attribute of this component changes
                if (active) {
                    console.log("Start system move");
                    // https://codereview.qt-project.org/c/qt/qtbase/+/219277
                    rootWindow.startSystemMove();
                }
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

        Behavior on color {
            ColorAnimation {}
        }

        Component.onCompleted: {
            color = Qt.binding(function () {
                if (drawerOpen) {
                    return rgba("#1a1918", 0.8);
                }
                if (resizing || dragHandler.active) {
                    return rgba("#1a1918", 0.4);
                }
                switch (mode) {
                case CaptureMode.Screenshot:
                    return rgba("#e3c61e", 0.8);
                case CaptureMode.Record:
                    return rgba("#c5066f", 0.8);
                case CaptureMode.Stream:
                    return rgba("#3339ee", 0.8);
                default:
                    return "transparent";
                }
            });
        }
    }
}
