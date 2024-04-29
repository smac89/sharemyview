import QtQuick 2.15
import smv.app.capture 1.0

Item {
    id: root
    required property var screenshotCallback
    required property var recordingCallback
    required property var streamCallback
    required property QtObject target

    signal mediaCaptureRequested(int mode)
    signal mediaCaptureStarted(int mode)
    signal mediaCaptureFailed(int mode, string error)
    signal mediaCaptureSuccess(int mode, string path)
    signal mediaCaptureRequestEnded(int mode)

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
            AnimateOpacity {}
        },
        Transition {
            to: "screenshotRequested"
            SequentialAnimation {
                AnimateOpacity {}
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

        function onMediaCaptureSuccess(mode: int, result: string) {
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
        smvApp.mediaCaptureStarted.connect(root.mediaCaptureStarted);
        smvApp.mediaCaptureFailed.connect(root.mediaCaptureFailed);
        smvApp.mediaCaptureSuccess.connect(root.mediaCaptureSuccess);
        smvApp.mediaCaptureStopped.connect(root.mediaCaptureRequestEnded);
    }

    component AnimateOpacity: NumberAnimation {
        target: root.target
        property: "opacity"
        easing.type: Easing.InOutQuad
    }

    // Component {
    //     id: transitionMedia
    //     // can be used in a signal handler!
    //     // https://doc.qt.io/qt-5/qml-qtquick-propertyanimation.html#details
    //     PropertyAnimation {
    //         required property var callback
    //         property: "opacity"
    //         target: root.target
    //         easing.type: Easing.InOutQuad
    //         onFinished: callback()
    //     }
    // }
}
