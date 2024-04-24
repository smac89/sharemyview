import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    property color bgColor: Qt.rgba(0.18, 0.25, 0.34, 0)
    signal record
    signal stopRecord
    signal stream
    signal stopStream
    signal capture

    Frame {
        RowLayout {
            spacing: 6
            Rectangle {
                color: bgColor
                Layout.fillWidth: true
            }
        }
    }
}
