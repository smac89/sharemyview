import QtQuick 2.15
import QtQuick.Shapes 1.15

Item {
    id: root
    width: 20
    height: 20
    anchors.margins: 5
    // TODO: This might need to be removed to detect mouse enter
    // containmentMask: gripShape

    SystemPalette { id: palette; colorGroup: SystemPalette.Active }

    enum Pos {
        TL, TR, BL, BR
    }

    // https://doc.qt.io/qt-5/qtqml-syntax-signals.html#adding-signals-to-custom-qml-types
    signal dragStarted(edges: int)
    signal dragEnded

    required property int position
    // https://rgbcolorpicker.com/0-1
    property color gripColor: Qt.rgba(0.345, 0.514, 0.161, 0.8)
    // https://doc.qt.io/qt-5/qt.html#Edge-enum
    property int _edges
    // https://doc.qt.io/qt-5/qt.html#CursorShape-enum
    property int _cursorType

    Component.onCompleted: {
        switch (position) {
            case ResizeGrip.Pos.TL:
                _cursorType = Qt.SizeFDiagCursor
                _edges = Qt.TopEdge | Qt.LeftEdge
                break
            case ResizeGrip.Pos.TR:
                _cursorType = Qt.SizeBDiagCursor
                _edges = Qt.TopEdge | Qt.RightEdge
                rotation = 90
                break
            case ResizeGrip.Pos.BL:
                _cursorType = Qt.SizeBDiagCursor
                _edges = Qt.BottomEdge | Qt.LeftEdge
                rotation = -90
                break
            case ResizeGrip.Pos.BR:
                _cursorType = Qt.SizeFDiagCursor
                _edges = Qt.BottomEdge | Qt.RightEdge
                rotation = 180
                break
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        visible: mouseArea.containsMouse

        Shape {
            anchors.fill: parent
            containsMode: Shape.FillContains
            ShapePath {
                strokeWidth: 2
                strokeColor: palette.window
                fillColor: gripColor
                fillRule: ShapePath.WindingFill
                startX: 0; startY: 0
                PathLine { x: root.width; y: 0 }
                PathLine { x: 0; y: root.height }
                PathLine { x: 0; y: 0 }
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        hoverEnabled: true
        propagateComposedEvents: false
        cursorShape: _cursorType
        onPressed: (mouse) => {
            root.dragStarted(_edges)
            mouse.accepted = true
        }
        onReleased: (mouse) => {
            root.dragEnded()
            mouse.accepted = false
        }
    }
}
