import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Shapes 1.15
import "qrc:/components"

ApplicationWindow {
    id: rootWindow
    width: 640
    height: 480
    visible: true
    title: "Share My View"
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowSystemMenuHint
    background: rootBackground

    // Accessing qt from qml
    // https://doc.qt.io/qt-5/qtqml-cppintegration-interactqmlfromcpp.html
    // signal targetWindowMoved(point pos)
    // signal targetWindowResized(size sz)
    // signal targetWindowChanged(size sz, point pos)

    Connections {
        target: smvApp

        function onTargetWindowMoved(pos) {
            console.log(pos)
        }

        function onTargetWindowResized(sz) {
            console.log(sz)
        }

        function onTargetWindowChanged(sz, pos) {
            console.log(pos, sz)
        }
    }

    // ParallelAnimation {
    //     id: animateSize
    //     NumberAnimation on width { target: rootWindow; duration: 350 }
    //     NumberAnimation on height { target: rootWindow; duration: 350 }
    //     onFinished: {
    //         console.log("Animation done")
    //     }
    // }

    Rectangle {
        id: rootBackground
        anchors.fill: parent
        border { color: Qt.lighter(rootWindow.palette.window); width: 3 }
        radius: 5
        color: "transparent"

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
