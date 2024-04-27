import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: debug

    StackLayout {
        id: container

        anchors.fill: parent

        Rectangle {
            anchors.fill: parent
            border.color: Qt.rgba(Math.random(), Math.random(), Math.random(), 1)
            border.width: 2
        }

        Rectangle {
            width: parent.width - 2
            height: parent.height - 2
            children: Array(debug.children).filter((c) => {
                return c != container;
            })
            Component.onCompleted: {
                for (var i = 0; i < debug.children.length; i++) {
                    console.log(debug.children[i]);
                }
            }
        }
    }
}
