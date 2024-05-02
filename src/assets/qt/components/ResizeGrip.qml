import QtQuick 2.15
import QtQuick.Shapes 1.15
import easy.colors 1.0

Item {
    id: root
    width: 20
    height: 20
    anchors.margins: 5

    // TODO: This might need to be removed to detect mouse enter
    // containmentMask: gripShape

    SystemPalette {
        id: palette
        colorGroup: SystemPalette.Active
    }

    enum Pos {
        TL,
        TR,
        BL,
        BR
    }

    // https://doc.qt.io/qt-5/qtqml-syntax-signals.html#adding-signals-to-custom-qml-types
    signal dragStarted(int edges)
    signal dragEnded

    required property int position
    // https://rgbcolorpicker.com/0-1
    property color gripColor: rgba("#4a7c15", 0.8)

    QtObject {
        id: priv
        // https://doc.qt.io/qt-5/qt.html#Edge-enum
        property int edges
        // https://doc.qt.io/qt-5/qt.html#CursorShape-enum
        property int cursorType
    }

    Component.onCompleted: {
        switch (position) {
        case ResizeGrip.Pos.TL:
            priv.cursorType = Qt.SizeFDiagCursor;
            priv.edges = Qt.TopEdge | Qt.LeftEdge;
            break;
        case ResizeGrip.Pos.TR:
            priv.cursorType = Qt.SizeBDiagCursor;
            priv.edges = Qt.TopEdge | Qt.RightEdge;
            root.rotation = 90;
            break;
        case ResizeGrip.Pos.BL:
            priv.cursorType = Qt.SizeBDiagCursor;
            priv.edges = Qt.BottomEdge | Qt.LeftEdge;
            root.rotation = -90;
            break;
        case ResizeGrip.Pos.BR:
            priv.cursorType = Qt.SizeFDiagCursor;
            priv.edges = Qt.BottomEdge | Qt.RightEdge;
            root.rotation = 180;
            break;
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        visible: mouseArea.containsPress

        // border.color: "white"

        Shape {
            anchors.fill: parent
            containsMode: Shape.FillContains
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
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        // acceptedButtons: Qt.LeftButton
        hoverEnabled: true
        // propagateComposedEvents: false
        cursorShape: priv.cursorType
        onPressed: mouse => {
            console.log("Pressed");
            root.dragStarted(priv.edges);
        }
        onReleased: mouse => {
            console.log("Released");
            root.dragEnded();
        }
        onExited: mouse => {
            console.log("Released");
            root.dragEnded();
        }
    }
}
