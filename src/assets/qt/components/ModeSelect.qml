import QtGraphicalEffects 1.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import smv.app.capture 1.0

ComboBox {
    id: root

    property int mode

    Component.onCompleted: {
        // set default mode
        mode = CaptureMode.Screenshot;
    }
    anchors.fill: parent
    anchors.leftMargin: 1
    anchors.rightMargin: 1
    valueRole: "mode"
    textRole: "text"
    model: [{
        "icon": "image://smv/icons/mode_screenshot.svg",
        "text": "Screenshot",
        "mode": CaptureMode.Screenshot
    }, {
        "icon": "image://smv/icons/mode_record.svg",
        "text": "Record",
        "mode": CaptureMode.Record
    }, {
        "icon": "image://smv/icons/mode_stream.svg",
        "text": "Stream",
        "mode": CaptureMode.Stream
    }]
    onActivated: {
        mode = root.currentValue;
        var text = "mode changed: %1";
        switch (root.currentValue) {
        case CaptureMode.Screenshot:
            console.log(text.arg("Screenshot mode"));
            break;
        case CaptureMode.Record:
            console.log(text.arg("Record mode"));
            break;
        case CaptureMode.Stream:
            console.log(text.arg("Stream mode"));
            break;
        }
    }

    SystemPalette {
        id: palette
    }

    background: Rectangle {
        id: bg

        color: palette.button
        radius: 5
        visible: true
    }

    indicator: Item {
        width: height
        height: 20
        x: root.width - width - root.rightPadding
        y: root.topPadding + (root.availableHeight - height) / 2
        Component.onCompleted: {
            console.log("chevron:", width, height);
        }

        Image {
            id: chevron

            source: "image://smv/icons/chevron.svg"
            smooth: true
            sourceSize: Qt.size(parent.width, parent.height)
            visible: false
        }

        ColorOverlay {
            anchors.fill: chevron
            source: chevron
            color: palette.buttonText
        }

    }

    delegate: ItemDelegate {
        text: modelData.text
        width: parent.width
        icon.source: modelData.icon
        highlighted: root.highlightedIndex === index

        background: Rectangle {
            width: parent.width
            height: parent.height
            color: root.currentIndex === index ? Qt.lighter(palette.button) : palette.window
        }

    }

}
