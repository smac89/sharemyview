import QtQml 2.15
import QtQuick.Layouts 1.15

RowLayout {
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
