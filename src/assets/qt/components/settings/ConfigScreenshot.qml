import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1
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

    FolderDialog {
        id: saveLocationDialog
        title: "Choose Default Save Location"
        folder: AppData.screenshot.saveLocation ?? StandardPaths.writableLocation(StandardPaths.PicturesLocation)
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
            Layout.bottomMargin: root.height > 400 ? 10 : 5
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
                    Label {
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        text: "Prefix"
                    }
                    TextField {
                        id: prefix
                        Layout.fillWidth: true
                        text: AppData.screenshot.prefix
                        placeholderText: "Screenshot_"
                        onEditingFinished: {
                            AppData.screenshot.prefix = prefix.text;
                        }
                    }
                    Label {
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        text: "Suffix"
                    }
                    TextField {
                        id: suffix
                        Layout.fillWidth: true
                        text: AppData.screenshot.suffix
                        placeholderText: "yyyy-MMM-dd_hh-mm-ss-zzz"
                        onEditingFinished: {
                            // TODO check if text is valid before
                            // new Date(Date.now()).toLocaleString(Qt.locale(), suffix_)
                            AppData.screenshot.suffix = suffix.text;
                        }
                    }
                }
                Label {
                    topPadding: 10
                    bottomPadding: 10
                    font.italic: true
                    font.pointSize: 12
                    text: {
                        const format_ = formatChoiceGroup.format;
                        const prefix_ = prefix.displayText;
                        const suffix_ = suffix.displayText;
                        if (format_ && prefix_ && suffix_) {
                            return "example: %1%2.%3".arg(prefix_).arg(new Date(Date.now()).toLocaleString(Qt.locale(), suffix_)).arg(format_.toLowerCase());
                        }
                        return "";
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
                    text: AppData.screenshot.saveLocation
                    placeholderText: StandardPaths.writableLocation(StandardPaths.PicturesLocation)
                    onEditingFinished: {
                        AppData.screenshot.saveLocation = saveLocation.text;
                    }
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
                    property string format: AppData.screenshot.format
                    onClicked: {
                        AppData.screenshot.format = button.text;
                        // TODO: Why doesn't format update with app data?
                        format = button.text;
                    }
                }
                SettingTitle {
                    text: "Format"
                }
                Repeater {
                    model: AppData.screenshot.formatsList
                    RadioButton {
                        checked: modelData === formatChoiceGroup.format
                        text: modelData
                        ButtonGroup.group: formatChoiceGroup
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
