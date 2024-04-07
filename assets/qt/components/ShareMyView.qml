import QtQuick 2.15

// https://stackoverflow.com/questions/9204226/qml-opacity-inheritance/20633727#20633727
Item {
    id: root
    width: 100;
    height: 100
    opacity: 0.0

    Rectangle {
        id: border
        anchors.fill: parent
        radius: 10
        color: "black"
        border.width: 5
        border.color: "darkorange"
    }
}
