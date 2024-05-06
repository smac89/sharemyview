import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import smv.app.CaptureMode 1.0

Popup {
    id: root
    dim: false
    modal: false
    padding: 2
    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0.0
            to: 1.0
        }
    }
    exit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1.0
            to: 0.0
        }
    }

    property int mode
    background: Rectangle {
        color: palette.base
        radius: 10
        border.width: 2
        border.color: palette.text
        anchors.fill: parent
    }

    Loader {
        anchors.fill: parent
        active: mode === CaptureMode.Screenshot
        source: visible ? "ConfigScreenshot.qml" : ""
    }

    Connections {
        function onAboutToShow() {
            if (mode === undefined) {
                console.log("Mode not set. Closing...");
                root.close();
            }
        }
        target: root
    }

    SystemPalette {
        id: palette
    }
}
