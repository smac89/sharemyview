import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Shapes 1.15
import org.ubiquity.view.ShareMyViewWindow 1.0

Item {
    ShareMyViewWindow {
        id: rootWindow
        width: 640
        height: 480
        visible: true
        title: "Share My View"
        color: "transparent"
        flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowSystemMenuHint

        SystemPalette { id: palette; colorGroup: SystemPalette.Active }

        Component {
            id: resizeGrip
            Item {
                id: container
                width: 15
                height: 15
                containmentMask: gripShape
                // https://doc.qt.io/qt-5/qt.html#Edge-enum
                property int edges
                // https://doc.qt.io/qt-5/qt.html#CursorShape-enum
                property int cursorType

                Shape {
                    id: gripShape
                    anchors.fill: parent
                    containsMode: Shape.FillContains
                    ShapePath {
                        strokeWidth: 2
                        strokeColor: palette.window
                        fillColor: palette.window
                        fillRule: ShapePath.WindingFill
                        startX: 0; startY: 0
                        PathLine { x: container.width; y: 0 }
                        PathLine { x: 0; y: container.height }
                        PathLine { x: 0; y: 0 }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    propagateComposedEvents: true
                    cursorShape: cursorType
                    onPressed: {
                        rootWindow.startSystemResize(edges)
                    }
                }
            }
        }

        Rectangle {
            anchors.fill: parent
            border { color: Qt.lighter(palette.window); width: 3 }
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
                        // https://codereview.qt-project.org/c/qt/qtbase/+/219277
                        rootWindow.startSystemMove()
                    }
                }
            }

            Loader {
                sourceComponent: resizeGrip
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: 5
                onLoaded: {
                    item.edges = Qt.TopEdge | Qt.LeftEdge
                    item.cursorType = Qt.SizeFDiagCursor
                }
            }

            Loader {
                sourceComponent: resizeGrip
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 5
                rotation: 90
                onLoaded: {
                    item.edges = Qt.TopEdge | Qt.RightEdge
                    item.cursorType = Qt.SizeBDiagCursor
                }
            }

            Loader {
                sourceComponent: resizeGrip
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 5
                rotation: 180
                onLoaded: {
                    item.edges = Qt.BottomEdge | Qt.RightEdge
                    item.cursorType = Qt.SizeFDiagCursor
                }
            }

            Loader {
                sourceComponent: resizeGrip
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: 5
                rotation: -90
                onLoaded: {
                    item.edges = Qt.BottomEdge | Qt.LeftEdge
                    item.cursorType = Qt.SizeBDiagCursor
                }
            }
        }
    }
}
