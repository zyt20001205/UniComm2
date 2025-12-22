import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent
    property bool modelVisible: standardItemModel.rowCount() > 0

    Item {
        id: hintItem
        anchors.fill: parent
        visible: !modelVisible

        RowLayout {
            anchors.centerIn: parent

            Label {
                text: qsTr("Click to create port.")
                font.pixelSize: 16
                Layout.alignment: Qt.AlignVCenter
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton

                onSingleTapped: portModule.portSetting()
            }
        }
    }

    ListView {
        id: listView
        anchors.fill: parent
        clip: true
        model: standardItemModel
        visible: modelVisible

        delegate: SwitchDelegate {
            implicitWidth: listView.width;
            text: model.display

            onClicked: portModule.portToggle(model.display, checked)

            HoverHandler {
                onHoveredChanged: {
                    if (hovered) {
                        cursorShape = Qt.PointingHandCursor
                    }
                }
            }

            TapHandler {
                acceptedButtons: Qt.RightButton
                gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                onSingleTapped: {
                    portMenu.portName = text
                    portMenu.popup()
                }
            }
        }

        TapHandler {
            acceptedButtons: Qt.RightButton

            onSingleTapped: rootMenu.popup()
        }
    }

    Connections {
        target: standardItemModel

        function onRowsInserted() {
            modelVisible = true
        }

        function onRowsRemoved() {
            modelVisible = standardItemModel.rowCount() > 0
        }

        function onModelReset() {
            modelVisible = false
        }
    }

    function setChecked(portName, status) {
        for (let i = 0; i < listView.count; i++) {
            const item = listView.itemAtIndex(i)
            if (item.text === portName) {
                item.checked = status
                break
            }
        }
    }
}

