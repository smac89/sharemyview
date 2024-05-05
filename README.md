## Name
SMV Capture Application (Alpha)

![Qt](https://img.shields.io/badge/5.15-%2341CD52?style=for-the-badge&logo=Qt&logoColor=white&logoSize=auto)
![C++](https://img.shields.io/badge/17-%2300599C?style=for-the-badge&logo=cplusplus&logoColor=white&logoSize=auto)
![X11](https://img.shields.io/badge/XCB-%23F28834?style=for-the-badge&logo=xdotorg&logoColor=white&logoSize=auto)

## Description
Share My View (SMV) is being designed with the simple goal of making anything you can see on your desktop, sharable with anyone you choose. The target device form-factor are desktops computers (especially Linux), because screen-sharing is ubiquitos on mobile.

The goal is to have a very simple interface that doesn't get in your way (or becomes cluttered), so that all you really have to do to share is to select a region of your screen, and start sharing.
With that being said, the potential for misuse cannot be ignored, therefore secondary goal of SMV is to be as secure as possible so that potential misuse is easy to detect.

Although SMV is meant to be cross-platform (Windows, MacOs, Linux), I am primarily a Linux user, so the current focus is on the Linux side.

## Visuals (captured with SMV :smile:)
<div style="width: 100%; display: flex; flex-wrap: wrap">
<img src="docs/images/screenshot.png" style="max-width: 320px">
<img src="docs/images/record.png" style="max-width: 320px">
<img src="docs/images/stream.png" style="max-width: 320px">
<img src="docs/images/settings.png" style="max-width: 320px">
</div>

## Installation
At the moment, no installation instructions are provided because we are still in very early stages

## Usage
- You need `xmake`
- Run `xmake b --rebuild capture`
- Run `xmake run capture`

## Support
Expect bugs. Report any glaring ones

## Roadmap
- At the moment, it only supports taking screenshots of the desktop and saving it in `png`, or `jpg`. I plan to add one more useful image format: `GIF`.
- Integrate sharing via KDE-Connect or GSConnect
- Display list of recent captures
- Support for recording audio, and video
- Support for streaming audio and video
- Address many of the TODO's in the code (there's a lot)

## Contributing
I've only been developing this for a month (April 30th, 2024), so all the

## Authors and acknowledgment
Show your appreciation to those who have contributed to the project.

## License
CC BY-NC 4.0

## Project status
Currently under heavy maintenance

---

## Maintainer notes
The following are notes I've made for myself as I developed this app. I had no idea how XCB, QT, QML worked before starting. I had also never written a C++ application desktop application before, so these served as useful references for both learning and reminding myself. I also scattered a number of links throughout the code.

### Useful links
- https://github.com/alaingalvan/CrossWindow
- https://xcb.freedesktop.org/XcbUtil/
- https://xcb.freedesktop.org/tutorial/
- https://xcb.freedesktop.org/manual/modules.html
- https://gist.github.com/penk/470486
- https://stackoverflow.com/questions/40533318/xcb-custom-message-to-event-loop
- https://github.com/Airblader/xedgewarp/blob/master/src/randr.c#L101
- https://github.com/TAAPArthur/XMouseControl/blob/master/xutil.c
- http://metan.ucw.cz/blog/things-i-wanted-to-know-about-libxcb.html
- https://cppget.org/spdlog?f=full#description
- https://unix.stackexchange.com/questions/503806/what-are-x-server-display-and-screen
- https://unix.stackexchange.com/questions/367732/what-are-display-and-screen-with-regard-to-0-0
- https://www.x.org/wiki/guide/
- https://www.x.org/releases/current/doc/xproto/x11protocol.html

### Qt
- https://stackoverflow.com/a/52624533/2089675
- https://github.com/Luxoft/qml-coreui
- https://doc.qt.io/QMLLive/qmllive-installation.html
- https://github.com/carlonluca/lqtutils

### Conventions
- https://manual.gromacs.org/documentation/2019/dev-manual/naming.html

### Some questions that could be answered
- https://stackoverflow.com/questions/71131688/how-can-i-get-all-events-on-the-root-window-with-xcb
- https://stackoverflow.com/questions/37359063/x11-list-top-level-windows
