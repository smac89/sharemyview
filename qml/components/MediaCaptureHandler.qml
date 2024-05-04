import QtQuick 2.15
import QtQuick.Controls 2.15
import smv.app.CaptureMode 1.0
import smv.app.AppCore 1.0

Item {
    id: root
    required property var screenshotCallback
    required property var recordingCallback
    required property var streamCallback
    readonly property ApplicationWindow target: ApplicationWindow.window

    signal mediaCaptureRequested(int mode)
    signal mediaCaptureStarted(int mode)
    signal mediaCaptureFailed(int mode, string error)
    signal mediaCaptureSuccess(int mode, variant result)
    signal mediaCaptureRequestEnded(int mode)

    QtObject {
        // TODO: Can we find a better way?
        // used to keep track of the global position of the target window
        property point targetPos: root.target.background.mapToGlobal(0, 0)

        Component.onCompleted: {
            // Keeps track of the windows global position as it seems to forget after becoming invisible
            root.target.x = Qt.binding(() => targetPos.x);
            root.target.y = Qt.binding(() => targetPos.y);
        }
    }

    states: [
        State {
            name: "screenshotRequested"
            PropertyChanges {
                target: root.target
                opacity: 0
            }
            PropertyChanges {
                target: root.target
                visible: false
            }
            StateChangeScript {
                name: "takeScreenshot"
                script: screenshotCallback()
            }
        },
        State {
            name: "screenshotRequestEnd"
            extend: ""
            PropertyChanges {
                target: root.target
                visible: true
            }
            PropertyChanges {
                target: root.target
                opacity: 1
            }
        }
    ]

    transitions: [
        // Note: transitions are run in parallel depending on which match the current state
        // See https://doc.qt.io/qt-5/qml-qtquick-transition.html#reversible-prop
        Transition {
            to: ""
            SequentialAnimation {
                AnimateVisible {}
                AnimateOpacity {}
            }
        },
        Transition {
            to: "screenshotRequested"
            SequentialAnimation {
                AnimateOpacity {}
                AnimateVisible {}
                ScriptAction {
                    scriptName: "takeScreenshot"
                }
            }
        }
    ]

    Connections {
        function onMediaCaptureRequested(mode: int) {
            switch (mode) {
            case CaptureMode.Screenshot:
                {
                    console.log("Media capture requested: Screenshot");
                    root.state = "screenshotRequested";
                    break;
                }
            case CaptureMode.Record:
                {
                    console.log("Media capture requested: Record");
                    break;
                }
            case CaptureMode.Stream:
                {
                    console.log("Media capture requested: Stream");
                    break;
                }
            default:
                {
                    console.log("Media capture requested: Unknown mode");
                    break;
                }
            }
        }

        function onMediaCaptureStarted(mode: int) {
            console.log("Media capture started...");
        }

        function onMediaCaptureFailed(mode: int, error: string) {
            console.log("Media capture failed...", error);
        }

        function onMediaCaptureSuccess(mode: int, result: variant) {
            console.log("Media capture success...", result);
        }

        function onMediaCaptureRequestEnded(mode: int) {
            switch (mode) {
            case CaptureMode.Screenshot:
                {
                    root.state = "screenshotRequestEnd";
                    console.log("Media capture request ended: Screenshot");
                    break;
                }
            case CaptureMode.Record:
                {
                    console.log("Media capture request ended: Record");
                    break;
                }
            case CaptureMode.Stream:
                {
                    console.log("Media capture request ended: Stream");
                    break;
                }
            default:
                {
                    console.log("Media capture request ended: Unknown mode");
                    break;
                }
            }
        }

        target: root
    }

    Component.onCompleted: {
        // https://doc.qt.io/qt-5/qtqml-syntax-signals.html#connecting-signals-to-methods-and-signals
        AppCore.mediaCaptureStarted.connect(root.mediaCaptureStarted);
        AppCore.mediaCaptureFailed.connect(root.mediaCaptureFailed);
        AppCore.mediaCaptureSuccess.connect(root.mediaCaptureSuccess);
        AppCore.mediaCaptureStopped.connect(root.mediaCaptureRequestEnded);
    }

    component AnimateOpacity: NumberAnimation {
        target: root.target
        property: "opacity"
        easing.type: Easing.InOutQuad
        duration: 300
    }

    component AnimateVisible: NumberAnimation {
        target: root.target
        property: "visible"
        duration: 0
    }
}
