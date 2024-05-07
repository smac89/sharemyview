import QtQuick 2.15
import QtQuick.Shapes 1.15
import easy.colors 1.0
import "qrc:/components"

Item {
    id: root
    width: 30
    height: 30
    anchors.margins: 5

    SystemPalette {
        id: palette
        colorGroup: SystemPalette.Active
    }

    // https://doc.qt.io/qt-5/qtqml-syntax-signals.html#adding-signals-to-custom-qml-types
    signal dragStarted(int edges)
    signal dragEnded

    required property int position
    // https://rgbcolorpicker.com/0-1
    property color gripColor: rgba("#0d1406", 0.8)

    QtObject {
        id: priv
        // https://doc.qt.io/qt-5/qt.html#Edge-enum
        property int edges
        // https://doc.qt.io/qt-5/qt.html#CursorShape-enum
        property int cursorType
    }

    Component.onCompleted: {
        switch (position) {
        case Item.TopLeft:
            priv.cursorType = Qt.SizeFDiagCursor;
            priv.edges = Qt.TopEdge | Qt.LeftEdge;
            break;
        case Item.TopRight:
            priv.cursorType = Qt.SizeBDiagCursor;
            priv.edges = Qt.TopEdge | Qt.RightEdge;
            root.rotation = 90;
            break;
        case Item.BottomLeft:
            priv.cursorType = Qt.SizeBDiagCursor;
            priv.edges = Qt.BottomEdge | Qt.LeftEdge;
            root.rotation = -90;
            break;
        case Item.BottomRight:
            priv.cursorType = Qt.SizeFDiagCursor;
            priv.edges = Qt.BottomEdge | Qt.RightEdge;
            root.rotation = 180;
            break;
        }
    }

    Rectangle {
        id: handle
        anchors.fill: parent
        // color: q(cc`random`)
        color: "transparent"
        visible: mouseArea.containsMouse

        Shape {
            anchors.fill: parent
            containsMode: Shape.FillContains
            // visible: mouseArea.containsMouse /* TODO: Do we need this */
            ShapePath {
                strokeWidth: 2
                strokeColor: palette.window
                fillColor: gripColor
                fillRule: ShapePath.WindingFill
                startX: 0
                startY: 0
                PathLine {
                    x: root.width
                    y: 0
                }
                PathLine {
                    x: 0
                    y: root.height
                }
                PathLine {
                    x: 0
                    y: 0
                }
            }
        }

        DragHandler {
            acceptedDevices: PointerDevice.Mouse
            acceptedButtons: Qt.LeftButton
            // grabPermissions: PointerHandler.CanTakeOverFromAnything
            onGrabChanged: (transition, pt) => {
                if (transition === EventPoint.GrabExclusive && pt.state === EventPoint.Updated) {
                    root.dragStarted(priv.edges);
                } else if (transition === EventPoint.CancelGrabExclusive) {
                    root.dragEnded();
                }
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: priv.cursorType
    }
}
