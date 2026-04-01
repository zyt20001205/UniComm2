import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 6; anchors.rightMargin: 6; anchors.bottomMargin: 6

        Item {
            Layout.preferredWidth: 600; Layout.preferredHeight: 24

            TextField {
                id: replaceTextField
                anchors.fill: parent
                rightPadding: 24

                onTextChanged: searchWidget.searchRequest(searchTextField.text)
            }

            RowLayout {
                anchors.right: parent.right
                spacing: 0

                Button {
                    Layout.preferredWidth: 24; Layout.preferredHeight: 24
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    flat: true
                    icon.source: "qrc:/icon/close.svg"
                    icon.width: 12; icon.height: 12

                    onClicked: replaceTextField.clear()
                }
            }
        }

        Button {
            id: replaceButton
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            flat: true
            icon.source: "qrc:/icon/replace.svg"
            icon.width: 16; icon.height: 16

            onClicked: replaceWidget.replace()

            HoverHandler {
                onHoveredChanged: {
                    if (!hovered) {
                        mainTooltip.text = ""
                    }
                }
                onPointChanged: {
                    mainTooltip.position = parent.mapToGlobal(point.position)
                    mainTooltip.text = qsTr("Replace")
                }
            }
        }

        Button {
            id: replaceAllButton
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            flat: true
            icon.source: "qrc:/icon/replaceAll.svg"
            icon.width: 16; icon.height: 16

            onClicked: replaceWidget.replaceAll()

            HoverHandler {
                onHoveredChanged: {
                    if (!hovered) {
                        mainTooltip.text = ""
                    }
                }
                onPointChanged: {
                    mainTooltip.position = parent.mapToGlobal(point.position)
                    mainTooltip.text = qsTr("Replace All")
                }
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }
}