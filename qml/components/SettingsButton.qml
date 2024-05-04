import QtQuick 2.15
import QtGraphicalEffects 1.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: 40
    height: width
    property color buttonColor: palette.button
    signal clicked

    Button {
        id: button
        visible: false
        background: Rectangle {
            color: "transparent"
            width: root.width
            height: root.height
            anchors.centerIn: parent
        }
        icon {
            source: "image://smv/icons/settings.svg"
            color: buttonColor
            width: root.width - 10
            height: root.width - 10
        }
    }

    DropShadow {
        anchors.fill: button
        horizontalOffset: 0
        verticalOffset: 0
        radius: 1
        color: palette.buttonText
        samples: 3
        source: button
        cached: true
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
        cursorShape: Qt.PointingHandCursor
    }

    SystemPalette {
        id: palette
    }
}
