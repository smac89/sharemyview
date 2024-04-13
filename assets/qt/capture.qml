import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Shapes 1.15
import "qrc:/components"

ApplicationWindow {
    id: rootWindow
    width: targetWidth
    height: targetHeight
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
        console.log("setting window ready")
        smvApp.qquickWindowReady(rootWindow)
        console.log("set window ready")
    }

    Connections {
        target: smvApp
        // https://doc.qt.io/qt-5/qtqml-cppintegration-interactqmlfromcpp.html

        function onTargetWindowMoved(pos: point) {
            animateMove.stop()
            console.log("Target moved:", pos)
            targetPosX = pos.x
            targetPosY = pos.y
            animateMove.start()
        }

        function onTargetWindowResized(sz: size) {
            animateSize.stop()
            console.log("Target resized:", sz)
            targetWidth = sz.width
            targetHeight = sz.height
            animateSize.start()
        }

        function onTargetWindowChanged(sz: size, pos: point) {
            animateSize.stop()
            animateMove.stop()
            console.log("Target resized:", sz)
            targetWidth = sz.width
            targetHeight = sz.height
            targetPosX = pos.x
            targetPosY = pos.y
            animateSize.start()
            animateMove.start()
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
    }

    Rectangle {
        id: windowFrame
        width: initialWidth
        height: initialHeight
        anchors {
            // fill: parent
            horizontalCenter: rootWindow.horizontalCenter
            verticalCenter: rootWindow.verticalCenter
        }
        border { color: Qt.lighter(rootWindow.palette.window); width: 3 }
        radius: 5
        color: "transparent"

        ParallelAnimation {
            id: animateSize
            alwaysRunToEnd: false
            NumberAnimation { target: windowFrame; property: "width"; to: targetWidth; duration: 250 }
            NumberAnimation { target: windowFrame; property: "height"; to: targetHeight; duration: 250 }
            onFinished: {
                console.log("Resize finished")
            }
        }

        ParallelAnimation {
            id: animateMove
            alwaysRunToEnd: false
            NumberAnimation { target: rootWindow; property: "x"; to: targetPosX; duration: 350 }
            NumberAnimation { target: rootWindow; property: "y"; to: targetPosY; duration: 350 }
        }

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
