import QtQml 2.15
import QtQuick.Layouts 1.15

ColumnLayout {
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
