import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1
import Qt.labs.settings 1.0
import smv.app.ScreenshotFormat 1.0
import smv.app.AppCore 1.0
import smv.app.AppData 1.0
import easy.colors 1.0
import "qrc:/components"

Frame {
    id: root
    height: background.height

    background: Rectangle {
        border.width: 2
        radius: 5
        anchors.margins: 5
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        color: "transparent"
    }

    Settings {
        id: settings
        category: AppData.screenshot.category
        property alias format: formatChoiceGroup.format
        property alias saveLocation: saveLocation.text
    }

    FolderDialog {
        id: saveLocationDialog
        title: "Choose Default Save Location"
        folder: StandardPaths.writableLocation(StandardPaths.PicturesLocation)
        onAccepted: {
            saveLocation.text = currentFolder;
        }
    }

    AutoSizeColumnLayout {
        width: parent.width
        spacing: 15
        anchors.bottomMargin: 5
        anchors.topMargin: 5
        Debugable {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.bottomMargin: 20
            AutoSizeColumnLayout {
                anchors.topMargin: 5
                anchors.bottomMargin: 5
                Label {
                    id: title
                    text: "Screenshot Settings"
                    Layout.alignment: Qt.AlignHCenter
                    font.letterSpacing: -1
                    font.capitalization: Font.Capitalize
                    font.pointSize: 20
                }
                Rectangle {
                    id: titleDivider
                    Layout.fillWidth: true
                    color: palette.buttonText
                    width: parent.width
                    anchors.margins: 10
                    height: 2
                }
            }
        }

        Debugable {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            AutoSizeColumnLayout {
                spacing: 10
                SettingTitle {
                    id: naming
                    text: "Name format"
                }
                AutoSizeRowLayout {
                    id: nameOpts
                    spacing: 4
                    // anchors.topMargin: 10
                    Label {
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        text: "Prefix"
                    }
                    TextField {
                        Layout.fillWidth: true
                        placeholderText: "Screenshot_"
                    }
                    Label {
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        text: "Suffix"
                    }
                    TextField {
                        Layout.fillWidth: true
                        placeholderText: "%d_%H-%M-%S."
                    }
                }
            }
        }

        Debugable {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            AutoSizeRowLayout {
                anchors.fill: parent
                SettingTitle {
                    anchors.bottomMargin: 20
                    text: "Save location"
                }
                TextField {
                    id: saveLocation
                    selectByMouse: true
                    anchors.topMargin: 10
                    Layout.fillWidth: true
                    placeholderText: "~/Pictures/"
                }
                Button {
                    text: "..."
                    width: 10
                    background: Rectangle {
                        color: palette.button
                        radius: 5
                    }
                    onClicked: {
                        saveLocationDialog.open();
                    }
                }
            }
        }

        Debugable {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            AutoSizeRowLayout {
                spacing: 10
                anchors.fill: parent
                ButtonGroup {
                    id: formatChoiceGroup
                    property string format: ""
                    Component.onDestruction: {
                        settings.format = format;
                    }
                }
                SettingTitle {
                    text: "Format"
                }
                Repeater {
                    model: AppData.screenshot.formatsList
                    RadioButton {
                        checked: modelData === settings.format
                        text: modelData
                        ButtonGroup.group: formatChoiceGroup
                        onCheckedChanged: {
                            formatChoiceGroup.format = modelData;
                        }
                    }
                    Component.onCompleted: {
                        if (formatChoiceGroup.checkedButton === null && count > 0) {
                            itemAt(0).toggle();
                        }
                    }
                }
            }
        }
    }

    component SettingTitle: Label {
        font.bold: true
        font.pointSize: 15
        font.letterSpacing: -1
        font.capitalization: Font.Capitalize
    }

    component AutoSizeRowLayout: RowLayout {
        property bool autoWidth: true
        property bool autoHeight: true

        spacing: 10
        Component.onCompleted: {
            // TODO: Why does this syntax work?? 🤯
            if (autoWidth) {
                implicitWidth: childrenRect.width + anchors.leftMargin + anchors.rightMargin;
            }
            if (autoHeight) {
                implicitHeight: childrenRect.height + anchors.topMargin + anchors.bottomMargin;
            }
        }
    }

    component AutoSizeColumnLayout: ColumnLayout {
        property bool autoWidth: true
        property bool autoHeight: true

        spacing: 10
        Component.onCompleted: {
            if (autoWidth) {
                implicitWidth: childrenRect.width + anchors.leftMargin + anchors.rightMargin;
            }
            if (autoHeight) {
                implicitHeight: childrenRect.height + anchors.topMargin + anchors.bottomMargin;
            }
        }
    }

    SystemPalette {
        id: palette
    }
}
