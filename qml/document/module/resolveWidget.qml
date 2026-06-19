import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem

    Rectangle {
        anchors.fill: parent
        anchors.margins: 6
        color: global.stroke
        opacity: 0.3
        radius: 6
    }

    RowLayout {
        id: resolveBar
        anchors.fill: parent
        anchors.margins: 6

        Label {
            id: resolveStatLabel
            horizontalAlignment: Text.AlignRight; verticalAlignment: Text.AlignVCenter
            leftPadding: 6
            Layout.fillWidth: true; Layout.preferredHeight: 24
        }

        Button {
            id: resolvePrevButton
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            flat: true
            icon.source: "qrc:/icon/arrowUp.svg"
            icon.width: 16; icon.height: 16

            onClicked: resolveWidget.resolvePrev()

            HoverHandler {
                onHoveredChanged: {
                    if (!hovered) {
                        mainToolTip.text = ""
                    }
                }
                onPointChanged: {
                    mainToolTip.position = parent.mapToGlobal(point.position)
                    mainToolTip.text = qsTr("Resolve Previous")
                }
            }
        }

        Button {
            id: resolveNextButton
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            flat: true
            icon.source: "qrc:/icon/arrowDown.svg"
            icon.width: 16; icon.height: 16

            onClicked: resolveWidget.resolveNext()

            HoverHandler {
                onHoveredChanged: {
                    if (!hovered) {
                        mainToolTip.text = ""
                    }
                }
                onPointChanged: {
                    mainToolTip.position = parent.mapToGlobal(point.position)
                    mainToolTip.text = qsTr("Resolve Next")
                }
            }
        }
    }

    Component.onCompleted: {
        const objects = {
            "resolvePrevButton": resolvePrevButton,
            "resolveNextButton": resolveNextButton,
            "resolveStatLabel": resolveStatLabel
        };
        resolveWidget.propertyGet(objects)
    }
}
