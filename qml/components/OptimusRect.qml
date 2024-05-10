import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.15

Item {
    id: root
    containmentMask: background
    anchors.topMargin: topPadding + borderWidth
    anchors.bottomMargin: bottomPadding + borderWidth

    property real radius: 0
    property real bottomLeftRadius: radius
    property real bottomRightRadius: radius
    property real topLeftRadius: radius
    property real topRightRadius: radius
    property color bgColor: palette.base
    property color borderColor: bgColor
    property real borderWidth: 2

    property bool hideBorders: false
    property bool hideTopBorder: hideBorders
    property bool hideBottomBorder: hideBorders
    property bool hideLeftBorder: hideBorders
    property bool hideRightBorder: hideBorders

    readonly property real topPadding: {
        // Only required for calculating padding for inverse borders
        if (topLeftRadius < 0 || topRightRadius < 0) {
            return Math.abs(Math.min(topLeftRadius, topRightRadius));
        } else {
            return 0;
        }
    }

    readonly property real bottomPadding: {
        // Only required for calculating padding for inverse borders
        if (bottomLeftRadius < 0 || bottomRightRadius < 0) {
            return Math.abs(Math.min(bottomLeftRadius, bottomRightRadius));
        } else {
            return 0;
        }
    }

    readonly property real maxBorderWidth: Math.max(Math.abs(topLeftRadius), Math.abs(topRightRadius), Math.abs(bottomLeftRadius), Math.abs(bottomRightRadius))

    Component.onCompleted: {
        if (radius < 0) {
            radius = 0;
        }
        // if (borderWidth < 0) {
        //     borderWidth = Math.abs(borderWidth);
        // }
        // if (topLeftRadius < 0) {
        //     topLeftRadius = 0;
        // }
        // if (topRightRadius < 0) {
        //     topRightRadius = 0;
        // }
        // if (bottomLeftRadius < 0) {
        //     bottomLeftRadius = 0;
        // }
        // if (bottomRightRadius < 0) {
        //     bottomRightRadius = 0;
        // }
    }

    Rectangle {
        id: background
        color: "transparent"
        anchors.fill: parent
        containmentMask: backgroundShape

        Rectangle {
            width: parent.width - Math.abs(topLeftRadius) - Math.abs(topRightRadius)
            height: maxBorderWidth
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Math.abs(topLeftRadius)
            anchors.rightMargin: Math.abs(topRightRadius)
            color: root.bgColor
        }
        Rectangle {
            width: parent.width - Math.abs(bottomLeftRadius) - Math.abs(bottomRightRadius)
            height: maxBorderWidth
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.leftMargin: Math.abs(bottomLeftRadius)
            anchors.rightMargin: Math.abs(bottomRightRadius)
            color: root.bgColor
        }

        Rectangle {
            width: maxBorderWidth
            height: parent.height - Math.abs(bottomLeftRadius) - Math.abs(topLeftRadius)
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: Math.abs(topLeftRadius)
            anchors.bottomMargin: Math.abs(bottomLeftRadius)
            color: root.bgColor
        }

        Rectangle {
            width: parent.width - maxBorderWidth
            height: parent.height - maxBorderWidth * 2
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: maxBorderWidth
            anchors.bottomMargin: maxBorderWidth
            anchors.leftMargin: maxBorderWidth
            color: root.bgColor
        }

        Shape {
            id: backgroundShape
            anchors.fill: parent
            containsMode: Shape.FillContains
            OptShapePath {
                id: bgTopLeftCorner
                fillColor: root.bgColor
                startX: 0
                startY: Math.abs(topLeftRadius)
                PathLine {
                    x: Math.abs(topLeftRadius)
                    y: bgTopLeftCorner.startY
                }
                PathLine {
                    x: Math.abs(topLeftRadius)
                    y: 0
                }
            }

            OptShapePath {
                id: bgTop
                startX: Math.abs(topLeftRadius)
                startY: 0
                strokeColor: root.hideTopBorder ? "transparent" : root.borderColor
                PathLine {
                    x: background.width - Math.abs(topRightRadius)
                    y: bgTop.startY
                }
            }

            OptShapePath {
                id: bgTopRightCorner
                fillColor: root.bgColor
                startX: background.width - Math.abs(topRightRadius)
                startY: bgTop.startY
                PathLine {
                    x: bgTopRightCorner.startX
                    y: Math.abs(topRightRadius)
                }
                PathLine {
                    x: background.width
                    y: Math.abs(topRightRadius)
                }
            }

            OptShapePath {
                id: bgRight
                startX: background.width
                startY: Math.abs(topRightRadius)
                strokeColor: root.hideRightBorder ? "transparent" : root.borderColor
                PathLine {
                    x: bgRight.startX
                    y: background.height - Math.abs(bottomRightRadius)
                }
            }

            OptShapePath {
                id: bgBottomRightCorner
                fillColor: root.bgColor
                startX: bgRight.startX
                startY: background.height - Math.abs(bottomRightRadius)

                PathLine {
                    x: background.width - Math.abs(bottomRightRadius)
                    y: bgBottomRightCorner.startY
                }
                PathLine {
                    x: background.width - Math.abs(bottomRightRadius)
                    y: background.height
                }
            }

            OptShapePath {
                id: bgBottom
                startX: background.width - Math.abs(bottomRightRadius)
                startY: background.height
                strokeColor: root.hideBottomBorder ? "transparent" : root.borderColor
                PathLine {
                    x: Math.abs(bottomLeftRadius)
                    y: bgBottom.startY
                }
            }

            OptShapePath {
                id: bgBottomLeftCorner
                fillColor: root.bgColor
                startX: Math.abs(bottomLeftRadius)
                startY: bgBottom.startY
                PathLine {
                    x: bgBottomLeftCorner.startX
                    y: background.height - Math.abs(bottomLeftRadius)
                }
                PathLine {
                    x: 0
                    y: background.height - Math.abs(bottomLeftRadius)
                }
            }

            OptShapePath {
                id: bgLeft
                startX: 0
                startY: background.height - Math.abs(bottomLeftRadius)
                strokeColor: root.hideLeftBorder ? "transparent" : root.borderColor
                PathLine {
                    x: bgLeft.startX
                    y: Math.abs(topLeftRadius)
                }
            }
        }
    }

    component OptShapePath: ShapePath {
        fillColor: "transparent"
        strokeColor: "transparent"
        strokeWidth: Math.abs(root.borderWidth)
        fillRule: ShapePath.WindingFill
        capStyle: ShapePath.RoundCap
        joinStyle: ShapePath.RoundJoin
    }

    Loader {
        active: topLeftRadius !== 0
        sourceComponent: {
            if (topLeftRadius > 0) {
                return roundedBorder;
            } else {
                return invertedBorder;
            }
        }
        onLoaded: {
            item.corner = Item.TopLeft;
        }
    }

    Loader {
        active: topRightRadius !== 0
        sourceComponent: {
            if (topRightRadius > 0) {
                return roundedBorder;
            } else {
                return invertedBorder;
            }
        }
        onLoaded: {
            item.corner = Item.TopRight;
        }
    }

    Loader {
        active: bottomLeftRadius !== 0
        sourceComponent: {
            if (bottomLeftRadius > 0) {
                return roundedBorder;
            } else {
                return invertedBorder;
            }
        }
        onLoaded: {
            item.corner = Item.BottomLeft;
        }
    }

    Loader {
        active: bottomRightRadius !== 0
        sourceComponent: {
            if (bottomRightRadius > 0) {
                return roundedBorder;
            } else {
                return invertedBorder;
            }
        }
        onLoaded: {
            item.corner = Item.BottomRight;
        }
    }

    Component {
        id: roundedBorder
        Item {
            id: shape
            property int corner: -1
            property real radius: 0
            property bool hideBorder: false

            layer.enabled: true
            layer.samples: 12
            layer.smooth: true
            width: radius * 2
            height: radius * 2
            parent: root

            Shape {
                ShapePath {
                    id: curve
                    fillRule: ShapePath.WindingFill
                    // fillColor: "purple"
                    fillColor: root.bgColor
                    // strokeColor: "red"
                    strokeColor: hideBorder ? "transparent" : root.borderColor
                    strokeWidth: root.borderWidth
                    startX: {
                        switch (corner) {
                        case Item.TopLeft:
                            return root.borderWidth;
                        case Item.TopRight:
                            return shape.width - root.borderWidth;
                        case Item.BottomLeft:
                            return root.borderWidth;
                        case Item.BottomRight:
                            return shape.width - root.borderWidth;
                        }
                    }
                    startY: {
                        switch (corner) {
                        case Item.TopLeft:
                            return radius + root.borderWidth;
                        case Item.TopRight:
                            return radius + root.borderWidth;
                        case Item.BottomLeft:
                            return radius - root.borderWidth;
                        case Item.BottomRight:
                            return radius - root.borderWidth;
                        }
                    }
                    PathArc {
                        radiusX: radius
                        radiusY: radiusX
                        x: {
                            switch (corner) {
                            case Item.TopLeft:
                                direction = PathArc.Clockwise;
                                return radius + root.borderWidth;
                            case Item.BottomRight:
                                direction = PathArc.Clockwise;
                                return radius - root.borderWidth;
                            case Item.TopRight:
                                direction = PathArc.Counterclockwise;
                                return radius - root.borderWidth;
                            case Item.BottomLeft:
                                direction = PathArc.Counterclockwise;
                                return radius + root.borderWidth;
                            }
                        }
                        y: {
                            switch (corner) {
                            case Item.TopLeft:
                                return root.borderWidth;
                            case Item.TopRight:
                                return root.borderWidth;
                            case Item.BottomLeft:
                                return shape.height - root.borderWidth;
                            case Item.BottomRight:
                                return shape.height - root.borderWidth;
                            }
                        }
                    }
                }
            }
            onCornerChanged: {
                switch (corner) {
                case Item.TopLeft:
                    radius = Math.abs(root.topLeftRadius);
                    hideBorder = root.hideTopBorder || root.hideLeftBorder;
                    anchors.top = root.top;
                    anchors.left = root.left;
                    anchors.leftMargin = -root.borderWidth;
                    anchors.topMargin = -root.borderWidth;
                    break;
                case Item.TopRight:
                    radius = Math.abs(root.topRightRadius);
                    hideBorder = root.hideTopBorder || root.hideRightBorder;
                    anchors.top = root.top;
                    anchors.right = root.right;
                    anchors.topMargin = -root.borderWidth;
                    anchors.rightMargin = -root.borderWidth;
                    break;
                case Item.BottomLeft:
                    radius = Math.abs(root.bottomLeftRadius);
                    hideBorder = root.hideBottomBorder || root.hideLeftBorder;
                    anchors.bottom = root.bottom;
                    anchors.left = root.left;
                    anchors.bottomMargin = -root.borderWidth;
                    anchors.leftMargin = -root.borderWidth;
                    break;
                case Item.BottomRight:
                    radius = Math.abs(root.bottomRightRadius);
                    hideBorder = root.hideBottomBorder || root.hideRightBorder;
                    anchors.bottom = root.bottom;
                    anchors.right = root.right;
                    anchors.bottomMargin = -root.borderWidth;
                    anchors.rightMargin = -root.borderWidth;
                    break;
                }
            }

            // Rectangle {
            //     property int corner: -1

            //     color: root.bgColor
            // width: radius * 2
            // height: radius * 2
            //     // border.width: root.borderWidth
            //     // border.color: "transparent"
            //     parent: root

            //     onCornerChanged: {
            //         switch (corner) {
            //         case Item.TopLeft:
            //             anchors.top = root.top;
            //             anchors.left = root.left;
            //             radius = Math.abs(root.topLeftRadius);
            //             break;
            //         case Item.TopRight:
            //             anchors.top = root.top;
            //             anchors.right = root.right;
            //             radius = Math.abs(root.topRightRadius);
            //             break;
            //         case Item.BottomLeft:
            //             anchors.bottom = root.bottom;
            //             anchors.left = root.left;
            //             radius = Math.abs(root.bottomLeftRadius);
            //             break;
            //         case Item.BottomRight:
            //             anchors.bottom = root.bottom;
            //             anchors.right = root.right;
            //             radius = Math.abs(root.bottomRightRadius);
            //             break;
            //         }
            //     }
            // }
        }
    }

    Component {
        id: invertedBorder

        Item {
            id: shape
            parent: root
            property real radius: 0
            property int corner: -1
            property bool hideVerticalBorder: false
            property bool hideHorizontalBorder: false
            width: radius + 2 * root.borderWidth
            height: radius * 2
            layer.enabled: true
            layer.samples: 12
            layer.smooth: true

            Shape {
                id: arcShape
                ShapePath {
                    id: horizontal
                    fillRule: ShapePath.WindingFill
                    // fillColor: "purple"
                    fillColor: "transparent"
                    // strokeColor: "red"
                    strokeColor: hideHorizontalBorder ? "transparent" : root.borderColor
                    strokeWidth: root.borderWidth
                    startX: {
                        switch (corner) {
                        case Item.TopLeft:
                        case Item.BottomLeft:
                            return root.borderWidth;
                        case Item.TopRight:
                        case Item.BottomRight:
                            return shape.width - root.borderWidth;
                        }
                    }
                    startY: {
                        switch (corner) {
                        case Item.TopLeft:
                        case Item.TopRight:
                            return 0;
                        case Item.BottomLeft:
                        case Item.BottomRight:
                            return shape.height;
                        }
                    }

                    PathArc {
                        radiusX: radius
                        radiusY: radiusX
                        x: {
                            switch (corner) {
                            case Item.TopLeft:
                                direction = PathArc.Counterclockwise;
                            case Item.BottomLeft:
                                return shape.width - root.borderWidth;
                            case Item.BottomRight:
                                direction = PathArc.Counterclockwise;
                            case Item.TopRight:
                                return root.borderWidth;
                            }
                        }
                        y: shape.height / 2
                    }

                    // PathMove {
                    //     x: curve.startX
                    //     y: curve.startY
                    // }

                    // PathLine {
                    //     relativeX: 0
                    //     y: shape.height / 2
                    // }
                    // PathLine {
                    //     relativeX: 0
                    //     y: {
                    //         switch (corner) {
                    //         case Item.TopLeft:
                    //         case Item.TopRight:
                    //             return shape.height;
                    //         case Item.BottomLeft:
                    //         case Item.BottomRight:
                    //             return 0;
                    //         }
                    //     }
                    // }
                    // PathLine {
                    //     x: {
                    //         switch (corner) {
                    //         case Item.TopLeft:
                    //         case Item.BottomLeft:
                    //             return 0;
                    //         case Item.TopRight:
                    //         case Item.BottomRight:
                    //             return shape.width;
                    //         }
                    //     }
                    //     relativeY: 0
                    // }
                    // PathLine {
                    //     relativeX: 0
                    //     y: {
                    //         switch (corner) {
                    //         case Item.TopLeft:
                    //         case Item.TopRight:
                    //             return 0;
                    //         case Item.BottomLeft:
                    //         case Item.BottomRight:
                    //             return shape.height;
                    //         }
                    //     }
                    // }
                }
            }
            Shape {
                ShapePath {
                    id: vertical
                    fillRule: ShapePath.WindingFill
                    // fillColor: "transparent"
                    fillColor: root.bgColor
                    // strokeColor: "red"
                    strokeColor: hideVerticalBorder ? "transparent" : root.borderColor
                    strokeWidth: root.borderWidth
                    startX: {
                        switch (corner) {
                        case Item.TopLeft:
                        case Item.BottomLeft:
                            return 0 + root.borderWidth;
                        case Item.TopRight:
                        case Item.BottomRight:
                            return shape.width - root.borderWidth;
                        }
                    }
                    startY: {
                        switch (corner) {
                        case Item.TopLeft:
                        case Item.TopRight:
                            return horizontal.startY + root.borderWidth;
                        case Item.BottomLeft:
                        case Item.BottomRight:
                            return horizontal.startY - root.borderWidth;
                        }
                    }

                    PathArc {
                        radiusX: radius
                        radiusY: radiusX
                        x: {
                            switch (corner) {
                            case Item.TopLeft:
                                direction = PathArc.Counterclockwise;
                            case Item.BottomLeft:
                                return shape.width;
                            case Item.BottomRight:
                                direction = PathArc.Counterclockwise;
                            case Item.TopRight:
                                return 0;
                            }
                        }
                        y: {
                            switch (corner) {
                            case Item.TopLeft:
                            case Item.TopRight:
                                return shape.height / 2;
                            case Item.BottomLeft:
                            case Item.BottomRight:
                                return shape.height / 2;
                            }
                        }
                    }

                    PathLine {
                        relativeX: 0
                        y: {
                            switch (corner) {
                            case Item.TopLeft:
                            case Item.TopRight:
                                return shape.height;
                            case Item.BottomLeft:
                            case Item.BottomRight:
                                return 0;
                            }
                        }
                    }

                    PathLine {
                        x: {
                            switch (corner) {
                            case Item.TopRight:
                            case Item.BottomRight:
                                return shape.width;
                            case Item.TopLeft:
                            case Item.BottomLeft:
                                return 0;
                            }
                        }
                        relativeY: 0
                    }

                    PathLine {
                        relativeX: 0
                        y: {
                            switch (corner) {
                            case Item.TopRight:
                            case Item.TopLeft:
                                return 0;
                            case Item.BottomLeft:
                            case Item.BottomRight:
                                return shape.height;
                            }
                        }
                    }
                }

                transform: [
                    Translate {
                        x: {
                            switch (corner) {
                            case Item.TopLeft:
                            case Item.BottomLeft:
                                return root.borderWidth;
                            case Item.TopRight:
                            case Item.BottomRight:
                                return 0;
                            }
                        }
                        y: {
                            switch (corner) {
                            case Item.TopLeft:
                            case Item.TopRight:
                                return root.borderWidth / 2;
                            case Item.BottomLeft:
                            case Item.BottomRight:
                                return -root.borderWidth / 2;
                            }
                        }
                    }
                ]
            }
            // Rectangle {
            //     color: root.bgColor
            //     width: shape.width
            //     height: width
            //     Component.onCompleted: {
            //         switch (corner) {
            //         case Item.TopLeft:
            //         case Item.TopRight:
            //             anchors.bottom = arcShape.top;
            //             break;
            //         case Item.BottomLeft:
            //         case Item.BottomRight:
            //             anchors.bottom = arcShape.top;
            //             break;
            //         }
            //     }
            // }
            // Shape {
            //     anchors.top: arcShape.bottom
            //     ShapePath {
            //         fillRule: ShapePath.WindingFill
            //         // fillColor: "purple"
            //         fillColor: root.bgColor
            //         // strokeColor: "red"
            //         strokeColor: root.borderColor
            //         strokeWidth: root.borderWidth
            //         startX: {
            //             switch (corner) {
            //             case Item.TopLeft:
            //             case Item.BottomLeft:
            //                 return shape.width;
            //             case Item.BottomRight:
            //             case Item.TopRight:
            //                 return 0;
            //             }
            //         }
            //         startY: shape.height / 2
            //         PathLine {
            //             relativeX: 0
            //             y: {
            //                 switch (corner) {
            //                 case Item.TopLeft:
            //                 case Item.TopRight:
            //                     return shape.height;
            //                 case Item.BottomLeft:
            //                 case Item.BottomRight:
            //                     return 0;
            //                 }
            //             }
            //         }
            //         PathLine {
            //             x: {
            //                 switch (corner) {
            //                 case Item.TopLeft:
            //                 case Item.BottomLeft:
            //                     return 0;
            //                 case Item.TopRight:
            //                 case Item.BottomRight:
            //                     return shape.width;
            //                 }
            //             }
            //             relativeY: 0
            //         }
            //         PathLine {
            //             relativeX: 0
            //             y: {
            //                 switch (corner) {
            //                 case Item.TopLeft:
            //                 case Item.TopRight:
            //                     return 0;
            //                 case Item.BottomLeft:
            //                 case Item.BottomRight:
            //                     return shape.height;
            //                 }
            //             }
            //         }
            //     }
            // }
            onCornerChanged: {
                switch (corner) {
                case Item.TopLeft:
                    radius = Math.abs(root.topLeftRadius);
                    hideHorizontalBorder = root.hideTopBorder;
                    hideVerticalBorder = root.hideLeftBorder;
                    anchors.leftMargin = -root.borderWidth;
                    anchors.left = root.left;
                    anchors.verticalCenter = parent.top;
                    break;
                case Item.TopRight:
                    radius = Math.abs(root.topRightRadius);
                    hideHorizontalBorder = root.hideTopBorder;
                    hideVerticalBorder = root.hideRightBorder;
                    anchors.rightMargin = -root.borderWidth;
                    anchors.right = root.right;
                    anchors.verticalCenter = parent.top;
                    break;
                case Item.BottomLeft:
                    radius = Math.abs(root.bottomLeftRadius);
                    hideHorizontalBorder = root.hideBottomBorder;
                    hideVerticalBorder = root.hideLeftBorder;
                    anchors.leftMargin = -root.borderWidth;
                    anchors.left = root.left;
                    anchors.verticalCenter = parent.bottom;
                    break;
                case Item.BottomRight:
                    radius = Math.abs(root.bottomRightRadius);
                    hideHorizontalBorder = root.hideBottomBorder;
                    hideVerticalBorder = root.hideRightBorder;
                    anchors.rightMargin = -root.borderWidth;
                    anchors.right = root.right;
                    anchors.verticalCenter = parent.bottom;
                    break;
                }
            }
        }
    }

    SystemPalette {
        id: palette
    }
}
