import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    SystemPalette { id: palette; colorGroup: SystemPalette.Active }

    property bool locked: false
    property int iconWidth: 36
    property int iconHeight: 36
    signal clicked

    height: iconHeight + 2
    width: iconWidth + 2

    Button {
        id: button
        display: AbstractButton.IconOnly
        icon {
            cache: true;
            source: locked ? "image://smv/icons/lock.svg" : "image://smv/icons/unlock.svg";
            height: iconHeight;
            width: iconWidth;
            color: locked ? Qt.rgba(0.082, 0.969, 0.02, 0.8) : Qt.rgba(0.969, 0.804, 0.016, 0.8)
        }
        anchors.fill: parent
        padding: 2
        verticalPadding: 3
        onClicked: {
            locked = !locked
            root.clicked()
        }
        background: Rectangle{
            opacity: 0.2
            border {
                color: palette.button
                width: 2
            }
            radius: 5
        }
    }
}
