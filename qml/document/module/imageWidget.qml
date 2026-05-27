import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent

        Flickable {
            id: flickable
            clip: true
            contentWidth: Math.max(width, imageContainer.width); contentHeight: Math.max(height, imageContainer.height)
            Layout.fillWidth: true; Layout.fillHeight: true
            property real ratio: 1.0
            property real minRatio: 0.1
            property real maxRatio: 10.0

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }
            ScrollBar.horizontal: ScrollBar {
                policy: ScrollBar.AsNeeded
            }
            Behavior on ratio {
                NumberAnimation {
                    duration: 150
                    easing.type: Easing.OutCubic
                }
            }

            Item {
                id: imageContainer
                anchors.centerIn: parent
                width: image.implicitWidth * flickable.ratio
                height: image.implicitHeight * flickable.ratio

                Image {
                    id: image
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                }
            }

            TapHandler {
                acceptedButtons: Qt.MiddleButton

                onTapped: flickable.ratio = 1.0
            }

            WheelHandler {
                onWheel: function (event) {
                    const zoom = event.angleDelta.y > 0 ? 1.2 : 0.8;
                    flickable.ratio = Math.max(flickable.minRatio, Math.min(flickable.maxRatio, flickable.ratio * zoom))
                    event.accepted = true
                }
            }
        }

        Label {
            id: ratioLabel
            font.pointSize: 24
            text: "x" + flickable.ratio.toFixed(2)
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