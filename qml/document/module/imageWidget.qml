import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: global.back
    }

    ColumnLayout {
        anchors.fill: parent

        Flickable {
            id: flickable
            clip: true
            interactive: false
            contentWidth: width; contentHeight: height
            Layout.fillWidth: true; Layout.fillHeight: true

            function center() {
                imageContainer.x = (width - imageContainer.width) / 2
                imageContainer.y = (height - imageContainer.height) / 2
            }

        Item {
                id: imageContainer
                x: 0; y: 0
                width: image.implicitWidth; height: image.implicitHeight
                scale: 1.0
                transformOrigin: Item.TopLeft

                Image {
                    id: image
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit

                    onStatusChanged: {
                        if (status !== Image.Ready) return
                        Qt.callLater(flickable.center)
                    }
                }
            }

            TapHandler {
                acceptedButtons: Qt.MiddleButton

                onTapped: {
                    imageContainer.scale = 1.0
                    flickable.center()
                }
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                hoverEnabled: true
                cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                property real pressedX: 0
                property real pressedY: 0
                property real imageX: 0
                property real imageY: 0

                onPressed: function (mouse) {
                    pressedX = mouse.x
                    pressedY = mouse.y
                    imageX = imageContainer.x
                    imageY = imageContainer.y
                }

                onPositionChanged: function (mouse) {
                    if (!pressed) return
                    imageContainer.x = imageX + mouse.x - pressedX
                    imageContainer.y = imageY + mouse.y - pressedY
                }
            }

            WheelHandler {
                id: wheelHandler

                onWheel: function (event) {
                    if (image.implicitWidth <= 0 || image.implicitHeight <= 0) return
                    const point = wheelHandler.point.position
                    const imageX = (point.x - imageContainer.x) / imageContainer.scale
                    const imageY = (point.y - imageContainer.y) / imageContainer.scale
                    imageContainer.scale *= event.angleDelta.y > 0 ? 1.2 : 0.8
                    imageContainer.x = point.x - imageX * imageContainer.scale
                    imageContainer.y = point.y - imageY * imageContainer.scale
                    event.accepted = true
                }
            }
        }

        Label {
            id: ratioLabel
            font.pointSize: 24
            text: "x" + imageContainer.scale.toFixed(2)
            Layout.alignment: Qt.AlignHCenter
        }
    }

    Component.onCompleted: {
        const objects = {
            "image": image,
        };
        imagePage.propertyGet(objects)
    }
}
