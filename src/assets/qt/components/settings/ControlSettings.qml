import QtQuick 2.15
import QtQuick.Controls 2.15
import smv.app.CaptureMode 1.0

Drawer {
    id: root
    dim: false
    modal: false
    topMargin: (parent.height - height) / 2
    rightPadding: background.anchors.rightMargin
    transformOrigin: Popup.Left
    edge: Qt.RightEdge

    // Loader
    property int mode

    SystemPalette {
        id: palette
    }

    // TODO: use tabs
    Loader {
        anchors.margins: 5
        anchors.fill: parent
        active: mode === CaptureMode.Screenshot
        source: visible ? "ConfigScreenshot.qml" : ""
    }
}
