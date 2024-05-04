import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.15
import easy.colors 1.0

StackView {
    id: root
    initialItem: child
    implicitWidth: child.width
    implicitHeight: child.height

    // TODO: Choose a contrasting color automatically
    property string borderColor: q(cc`random`)
    property bool isDebug: false
    property int borderWidth: 4
    property int borderRadius: 3

    // this will usually correspond to the last child
    default required property Item child

    DebugRect {
        id: debugParent
        visible: root.isDebug
    }

    states: [
        State {
            when: root.isDebug
            ParentChange {
                target: child
                parent: debugParent
            }
        },
        State {
            when: !root.isDebug
            ParentChange {
                target: child
                parent: root
            }
        }
    ]

    TapHandler {
        acceptedButtons: Qt.LeftButton
        acceptedModifiers: Qt.ShiftModifier | Qt.ControlModifier
        enabled: root.isDebug
        onTapped: root.borderColor = q(cc`random`)
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton
        acceptedModifiers: Qt.ControlModifier
        onTapped: root.isDebug = !root.isDebug
    }

    component DebugRect: Rectangle {
        width: parent.width
        height: parent.height
        color: "transparent"
        radius: root.borderRadius
        containmentMask: border

        Shape {
            id: border
            containsMode: Shape.BoundingRectContains
            ShapePath {
                strokeColor: root.borderColor
                strokeWidth: root.borderWidth
                fillColor: "transparent"
                fillRule: ShapePath.WindingFill
                strokeStyle: ShapePath.DashLine
                startX: 0
                startY: 0
                PathLine {
                    x: border.parent.width
                    y: 0
                }
                PathLine {
                    x: border.parent.width
                    y: border.parent.height
                }
                PathLine {
                    x: 0
                    y: border.parent.height
                }
                PathLine {
                    x: 0
                    y: 0
                }
            }
        }
    }
}
