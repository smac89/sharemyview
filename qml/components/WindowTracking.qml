import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import smv.app.AppCore 1.0
import smv.app.AppData 1.0

Item {
    id: root
    readonly property int initialWidth: 1024
    readonly property int initialHeight: 768

    signal targetResized(int width, int height)
    signal targetMoved(int x, int y)

    readonly property Window target: Window.window

    property int targetWidth: initialWidth
    property int targetHeight: initialHeight
    property int targetPosX: 0
    property int targetPosY: 0

    Component.onCompleted: {
        // initially position the window in the center of the screen
        const screen = AppData.cursorScreen;
        const screenRect = screen.geometry;
        Qt.callLater(() => {
            targetMoved(screenRect.x + screenRect.width / 2 - initialWidth / 2, screenRect.y + screenRect.height / 2 - initialHeight / 2);
            targetResized(initialWidth, initialHeight);
        });
    }

    // SequentialAnimation {
    //     id: reparentAnimation

    //     onFinished: {
    //         console.log("Reparent finished");
    //     }

    //     SmoothedAnimation {
    //         target: rootWindow
    //         property: "opacity"
    //         duration: 300
    //         to: 0
    //     }

    //     ParallelAnimation {
    //         PropertyAction {
    //             target: rootWindow
    //             property: "x"
    //             value: targetPosX
    //         }

    //         PropertyAction {
    //             target: rootWindow
    //             property: "y"
    //             value: targetPosY
    //         }
    //     }

    //     SmoothedAnimation {
    //         target: rootWindow
    //         property: "opacity"
    //         duration: 500
    //         to: 1
    //     }
    // }

    Connections {
        // reparentAnimation.restart()
        // animateSizePos.restart()
        // rootWindow.width = sz.width
        // rootWindow.height = sz.height
        // animateSizePos.restart()
        // reparentAnimation.restart()
        // rootWindow.width = sz.width
        // rootWindow.height = sz.height
        // animateSizePos.restart()

        // https://doc.qt.io/qt-5/qtqml-cppintegration-interactqmlfromcpp.html
        function onTargetWindowMoved(pos: point) {
            console.log("Target moved:", pos);
            // rootWindow.x = pos.x
            // rootWindow.y = pos.y
            targetPosX = pos.x;
            targetPosY = pos.y;
        }

        function onTargetWindowResized(sz: size) {
            console.log("Target resized:", sz);
        }

        function onTargetWindowChanged(sz: size, pos: point) {
            console.log("Target changed:", sz, pos);
            // rootWindow.x = pos.x
            // rootWindow.y = pos.y
            targetPosX = pos.x;
            targetPosY = pos.y;
        }
        target: AppCore
    }
}
