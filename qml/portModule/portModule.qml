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

            Button {
                flat: true
                text: qsTr("Click to create port.")
                font.pixelSize: 16
                Layout.alignment: Qt.AlignVCenter

                onClicked: portModule.portSetting()
            }
        }
    }

    VerticalHeaderView {
        id: verticalHeaderView
        anchors.left: parent.left
        width: 32; height: parent.height
        syncView: tableView
        clip: true
        interactive: false
        movableRows: true
        visible: modelVisible

        Rectangle {
            anchors.fill: parent
            color: "#e0e0e0"
            z: -1
        }

        delegate: VerticalHeaderViewDelegate {
            id: verticalHeaderViewDelegate
            implicitWidth: verticalHeaderView.width;
            padding: 0

            contentItem: Rectangle {
                width: 32; height: 32
                color: "white"

                Image {
                    width: 16; height: 16
                    anchors.centerIn: parent
                    source: "qrc:/icon/drag.svg"
                }
            }

            HoverHandler {
                onHoveredChanged: cursorShape = Qt.OpenHandCursor
            }
        }
    }

    TableView {
        id: tableView
        anchors.left: verticalHeaderView.right; anchors.right: parent.right
        height: parent.height
        alternatingRows: false
        clip: true
        editTriggers: TableView.NoEditTriggers
        rowSpacing: 1
        model: standardItemModel
        visible: modelVisible
        contentWidth: width

        Rectangle {
            anchors.fill: parent
            color: "#e0e0e0"
            z: -1
        }

        delegate: SwitchDelegate {
            implicitWidth: tableView.width
            text: model.display
            background: Rectangle {
                color: "white"
            }

            onClicked: portModule.portToggle(model.display, checked)

            HoverHandler {
                onHoveredChanged: cursorShape = Qt.PointingHandCursor
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
        for (let i = 0; i < tableView.rows; i++) {
            const item = tableView.itemAtCell(Qt.point(0, i))
            if (item.text === portName) {
                item.checked = status
                break
            }
        }
    }

    function getOrder() {
        const portList = [];
        for (let i = 0; i < tableView.rows; i++) {
            const item = tableView.itemAtCell(Qt.point(0, i))
            portList.push(item.text)
        }
        return portList
    }
}

