import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Shapes 1.15
import "qrc:/components"

/* https://doc.qt.io/qtcreator/creator-quick-ui-forms.html */
/* https://doc.qt.io/qt-5/qml-qtqml-qt.html */

ApplicationWindow {
    id: rootWindow
    width: initialWidth
    height: initialHeight
    visible: true
    title: "Share My View"
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowSystemMenuHint
    background: windowFrame

    readonly property int initialWidth: 640
    readonly property int initialHeight: 480
    readonly property int initialX: rootWindow.x
    readonly property int initialY: rootWindow.y
    property int targetWidth: initialWidth
    property int targetHeight: initialHeight
    property int targetPosX: initialX
    property int targetPosY: initialY

    Component.onCompleted: {
        smvApp.qquickWindowReady(rootWindow)
    }

    SequentialAnimation {
        id: reparentAnimation
        SmoothedAnimation { target: rootWindow; property: "opacity"; duration: 300; to: 0 }
        ParallelAnimation {
            PropertyAction { target: rootWindow; property: "x"; value: targetPosX }
            PropertyAction { target: rootWindow; property: "y"; value: targetPosY }
        }
        SmoothedAnimation { target: rootWindow; property: "opacity"; duration: 500; to: 1 }
        onFinished: {
            console.log("Reparent finished")
        }
    }

    Connections {
        target: smvApp
        // https://doc.qt.io/qt-5/qtqml-cppintegration-interactqmlfromcpp.html
        function onTargetWindowMoved(pos: point) {
            console.log("Target moved:", pos)
            // rootWindow.x = pos.x
            // rootWindow.y = pos.y
            targetPosX = pos.x
            targetPosY = pos.y
            reparentAnimation.restart()
            // animateSizePos.restart()
        }

        function onTargetWindowResized(sz: size) {
            console.log("Target resized:", sz)
            // rootWindow.width = sz.width
            // rootWindow.height = sz.height
            // animateSizePos.restart()
        }

        function onTargetWindowChanged(sz: size, pos: point) {
            console.log("Target changed:", sz, pos)
            // rootWindow.x = pos.x
            // rootWindow.y = pos.y
            targetPosX = pos.x
            targetPosY = pos.y
            reparentAnimation.restart()
            // rootWindow.width = sz.width
            // rootWindow.height = sz.height
            // animateSizePos.restart()
        }
    }

    /* SCREEN LOCK BUTTON */
    LockTargetButton {
        iconWidth: 38
        iconHeight: 38
        anchors {
            right: parent.right;
            top: parent.top;
            rightMargin: 10;
            topMargin: 10
        }
        onClicked: {
            console.log("Lock target clicked")
        }
    }

    /* CONTROLS */
    CaptureControls {
        anchors {
            bottom: parent.bottom;
            horizontalCenter: parent.horizontalCenter;
            bottomMargin: 10
        }
    }

    Rectangle {
        id: windowFrame
        width: initialWidth
        height: initialHeight
        anchors {
            fill: parent
            // horizontalCenter: rootWindow.horizontalCenter
            // verticalCenter: rootWindow.verticalCenter
        }
        border { color: Qt.lighter(rootWindow.palette.window); width: 5 }
        radius: 5
        color: "transparent"

        ParallelAnimation {
            id: animateSizePos
            alwaysRunToEnd: false
            SmoothedAnimation { target: rootWindow; property: "width"; to: targetWidth; duration: 250 }
            SmoothedAnimation { target: rootWindow; property: "height"; to: targetHeight; duration: 250 }
            SmoothedAnimation { target: rootWindow; property: "x"; to: targetPosX; duration: 250 }
            SmoothedAnimation { target: rootWindow; property: "y"; to: targetPosY; duration: 250 }

            onFinished: {
                console.log("Animation finished")
            }
        }

        // ParallelAnimation {
        //     id: animateMove
        //     alwaysRunToEnd: true
        //     SmoothedAnimation { target: rootWindow; property: "x"; to: targetPosX; duration: 350 }
        //     SmoothedAnimation { target: rootWindow; property: "y"; to: targetPosY; duration: 350 }
        // }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                // parent.grabToImage((result) => {
                //     result.saveToFile("/tmp/image.png")
                // })
            }
            onReleased: {
                console.log("Mouse button released")
            }
        }

        DragHandler {
            target: null
            acceptedButtons: Qt.LeftButton
            dragThreshold: 0
            acceptedDevices: PointerDevice.GenericPointer
            grabPermissions:  PointerHandler.CanTakeOverFromItems | PointerHandler.CanTakeOverFromHandlersOfDifferentType | PointerHandler.ApprovesTakeOverByItems
            // TODO: Fix bug with after resizing, cannot start dragging
            onActiveChanged: {
                if (active) {
                    console.log("Start system move")
                    // https://codereview.qt-project.org/c/qt/qtbase/+/219277
                    rootWindow.startSystemMove()
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
