// https://doc.qt.io/qtcreator/creator-quick-ui-forms.html
// https://doc.qt.io/qt-5/qml-qtqml-qt.html
// https://doc.qt.io/qt-5/qtqml-javascript-functionlist.html

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.15
import QtQuick.Window 2.15
import "qrc:/components"
import smv.app.capture 1.0

ApplicationWindow {
    id: rootWindow

    readonly property int initialWidth: 640
    readonly property int initialHeight: 480
    readonly property int initialX: rootWindow.x
    readonly property int initialY: rootWindow.y
    property int targetWidth: initialWidth
    property int targetHeight: initialHeight
    property int targetPosX: initialX
    property int targetPosY: initialY

    signal mediaCaptureRequested(int mode)
    signal mediaCaptureStarted()
    signal mediaCaptureStopped()

    width: initialWidth
    height: initialHeight
    minimumWidth: 480
    minimumHeight: minimumWidth
    visible: true
    title: "Share My View"
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowSystemMenuHint
    background: windowFrame
    Component.onCompleted: {
        // smvApp.qquickWindowReady(rootWindow);
    }

    Connections {
        function onMediaCaptureRequested(mode) {
            switch (mode) {
            case CaptureMode.Screenshot:
                {
                    Qt.callLater(smvApp.takeScreenshot, Qt.rect(rootWindow.x, rootWindow.y, rootWindow.width, rootWindow.height));
                    console.log("Media capture requested: Screenshot");
                    break;
                };
            case CaptureMode.Record:
                {
                    console.log("Media capture requested: Record");
                    break;
                };
            case CaptureMode.Stream:
                {
                    console.log("Media capture requested: Stream");
                    break;
                };
            default:
                {
                    console.log("Media capture requested: Unknown mode");
                    break;
                };
            }
        }

        target: rootWindow
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

        function onMediaCaptureStarted(mode: int) {
            console.log("Media capture started:", mode);
        }

        function onMediaCaptureStopped(mode: int) {
            console.log("Media capture stopped:", mode);
        }

        target: smvApp
    }

    // SCREEN LOCK BUTTON
    LockTargetButton {
        iconWidth: 38
        iconHeight: 38
        onClicked: {
            console.log("Lock target clicked");
        }

        anchors {
            right: parent.right
            top: parent.top
            rightMargin: 10
            topMargin: 10
        }

    }

    Rectangle {
        id: windowFrame

        property int mode

        width: initialWidth
        height: initialHeight
        radius: 5
        color: "transparent"

        anchors {
            fill: parent
        }

        border {
            color: Qt.lighter(rootWindow.palette.window)
            width: 5
        }

        ParallelAnimation {
            id: animateSizePos

            alwaysRunToEnd: false
            onFinished: {
                console.log("Animation finished");
            }

            SmoothedAnimation {
                target: rootWindow
                property: "width"
                to: targetWidth
                duration: 250
            }

            SmoothedAnimation {
                target: rootWindow
                property: "height"
                to: targetHeight
                duration: 250
            }

            SmoothedAnimation {
                target: rootWindow
                property: "x"
                to: targetPosX
                duration: 250
            }

            SmoothedAnimation {
                target: rootWindow
                property: "y"
                to: targetPosY
                duration: 250
            }

        }

        MouseArea {
            // parent.grabToImage((result) => {
            //     result.saveToFile("/tmp/image.png")
            // })

            anchors.fill: parent
            onClicked: {
            }
        }

        RecordingControls {
            width: parent.width - 20
            height: 50
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.margins: 10
            mode: parent.mode
            onTakeScreenshot: mediaCaptureRequested(CaptureMode.Screenshot)
        }

        Rectangle {
            width: 150
            height: 50
            anchors.centerIn: parent
            // border.color: "#f84a2b"
            radius: 5
            color: "transparent"

            ModeSelect {
                anchors.fill: parent
                onModeChanged: {
                    windowFrame.mode = mode;
                }
            }

        }

        DragHandler {
            target: null
            acceptedButtons: Qt.LeftButton
            dragThreshold: 10
            acceptedDevices: PointerDevice.GenericPointer
            grabPermissions: PointerHandler.CanTakeOverFromItems | PointerHandler.CanTakeOverFromHandlersOfDifferentType | PointerHandler.ApprovesTakeOverByItems
            // TODO: Fix bug with after resizing, cannot start dragging
            onActiveChanged: {
                if (active) {
                    console.log("Start system move");
                    // https://codereview.qt-project.org/c/qt/qtbase/+/219277
                    rootWindow.startSystemMove();
                }
            }
        }

        ResizeGrip {
            position: ResizeGrip.Pos.TL
            onDragStarted: rootWindow.startSystemResize(edges)
            anchors.left: parent.left
            anchors.top: parent.top
        }

        ResizeGrip {
            position: ResizeGrip.Pos.TR
            onDragStarted: rootWindow.startSystemResize(edges)
            anchors.right: parent.right
            anchors.top: parent.top
        }

        ResizeGrip {
            position: ResizeGrip.Pos.BL
            onDragStarted: rootWindow.startSystemResize(edges)
            anchors.left: parent.left
            anchors.bottom: parent.bottom
        }

        ResizeGrip {
            position: ResizeGrip.Pos.BR
            onDragStarted: rootWindow.startSystemResize(edges)
            anchors.right: parent.right
            anchors.bottom: parent.bottom
        }

    }

}
