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
    property point targetPos

    signal mediaCaptureRequested(int mode)
    signal mediaCaptureStarted(int mode)
    signal mediaCaptureFailed(int mode, string error)
    signal mediaCaptureSuccess(int mode, variant result)
    signal mediaCaptureRequestEnded(int mode)

    Component.onCompleted: {
        // https://doc.qt.io/qt-5/qtqml-syntax-signals.html#connecting-signals-to-methods-and-signals
        AppCore.mediaCaptureStarted.connect(root.mediaCaptureStarted);
        AppCore.mediaCaptureFailed.connect(root.mediaCaptureFailed);
        AppCore.mediaCaptureSuccess.connect(root.mediaCaptureSuccess);
        AppCore.mediaCaptureStopped.connect(root.mediaCaptureRequestEnded);
        targetPos = Qt.point(target.x, target.y);
    }

    states: [
        State {
            name: "screenshotRequested"
            PropertyChanges {
                target: root
                explicit: true // means: do not bind the property to the assigned value
                targetPos: Qt.point(root.target.x, root.target.y)
                restoreEntryValues: false
            }
            PropertyChanges {
                target: root.target
                visible: false
                restoreEntryValues: false
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
                x: root.targetPos.x
                y: root.targetPos.y
                restoreEntryValues: false
            }
        }
    ]

    transitions: [
        // Note: transitions are run in parallel depending on which match the current state
        // See https://doc.qt.io/qt-5/qml-qtquick-transition.html#reversible-prop
        Transition {
            to: "screenshotRequested"
            SequentialAnimation {
                ScriptAction {
                    script: {
                        console.log("Target Position Before:", targetPos);
                        console.log("Window Position Before:", root.target.x, root.target.y);
                    }
                }
                PauseAnimation {
                    /* TODO: This is mostly done to prevent the window
                    from still being visible when the screenshot is taken.
                    The bug is due to window-closing animations on some linux desktops.

                    Things I've tried:
                    - Using opacity to slowly fade out the window
                        - This didn't work. Opacity did not ever go to zero no matter how long the animation lasted
                    - Manually hiding the window in Qt.
                        - This didn't work. Even though window.isVisible() returns false, and the window manager reports that the window is unmapped,
                        the window is still sometimes caught by the screenshot.

                    Things I haven't tried:
                    - Instruct the window manager not to use compositing effects on the window, as
                    I suspect this is what keeps the window still visible even after it has been made invisible*/
                    duration: 300
                }
                ScriptAction {
                    scriptName: "takeScreenshot"
                }
            }
        },
        Transition {
            from: "screenshotRequested"
            SequentialAnimation {
                PropertyAction {
                    target: root.target
                    property: "visible"
                    value: true
                }
                ScriptAction {
                    script: {
                        console.log("Target Position After:", targetPos);
                        console.log("Window Position After:", root.target.x, root.target.y);
                    }
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
}
