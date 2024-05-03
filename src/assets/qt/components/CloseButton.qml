import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

RoundButton {
    id: root

    icon {
        source: "image://smv/icons/close.svg"
        width: root.width - 5
        height: root.height - 5
        color: mouseArea.containsMouse ? "#d81616" : palette.buttonText

        ColorAnimation on color {}
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            Window.window.close();
        }
    }

    SystemPalette {
        id: palette
    }
}
