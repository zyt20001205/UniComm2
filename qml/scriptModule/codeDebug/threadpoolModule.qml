import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    anchors.fill: parent

    Item {
        id: hintItem
        anchors.fill: parent

        RowLayout {
            anchors.centerIn: parent

            Label {
                text: qsTr("No active threads.")
                font.pixelSize: 16
                Layout.alignment: Qt.AlignVCenter
            }

            Image {
                source: "qrc:/icon/snooze.svg"
                Layout.alignment: Qt.AlignVCenter
            }
        }
    }

    HorizontalHeaderView {
        id: horizontalHeaderView
        anchors.top: parent.top
        width: parent.width; height: 32
        syncView: tableView
        clip: true
        interactive: false

        delegate: HorizontalHeaderViewDelegate {
            id: horizontalHeaderViewDelegate
            required property int index

            background: Rectangle {
                color: "transparent"
                border.width: 0
            }

            contentItem: Text {
                anchors.fill: parent
                clip: true
                font.family: "Segoe UI"
                font.pointSize: 10
                horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                text: horizontalHeader[horizontalHeaderViewDelegate.index]
            }
        }
    }

    TableView {
        id: tableView
        anchors.top: horizontalHeaderView.bottom; anchors.bottom: parent.bottom
        width: parent.width;
        alternatingRows: false
        clip: true
        editTriggers: TableView.NoEditTriggers
        rowSpacing: 1
        model: threadpoolModel
        contentWidth: width
        property int selectedRow: 0
        property string lifetime: ""

        Rectangle {
            anchors.fill: parent
            color: "#e0e0e0"
            z: -1
        }

        delegate: DelegateChooser {
            DelegateChoice {
                column: 0
                delegate: iconCellDelegate
            }
            DelegateChoice {
                delegate: textCellDelegate
            }
        }

        Component {
            id: iconCellDelegate

            Rectangle {
                implicitWidth: 24
                implicitHeight: 24
                color: "white"

                BusyIndicator {
                    anchors.centerIn: parent
                    running: true
                    width: 20
                    height: 20
                }
            }
        }

        Component {
            id: textCellDelegate

            Rectangle {
                id: textCell
                required property int column
                required property int row

                implicitWidth: {
                    if (textCell.column === tableView.columns - 1) {
                        let usedWidth = 0
                        for (let i = 0; i < tableView.columns - 1; i++) {
                            usedWidth += tableView.columnWidth(i)
                        }
                        return tableView.width - usedWidth
                    }
                    return Math.max(textMetrics.width + 16, 60)
                }
                implicitHeight: 24
                color: "white"

                TextMetrics {
                    id: textMetrics
                    font.family: "Segoe UI"
                    font.pointSize: 10
                    text: model.display || ""
                }

                Text {
                    anchors.fill: parent
                    z: 2
                    font.family: "Segoe UI"
                    font.pointSize: 10
                    horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                    text: model.display
                    elide: Text.ElideRight

                    ToolTip.visible: hoverHandler.hovered
                    ToolTip.text: tableView.lifetime
                }

                Rectangle {
                    id: highlightRect
                    anchors.fill: parent
                    z: 1
                    radius: 2
                    color: "#f5f5f5"
                    opacity: hoverHandler.hovered ? 1 : 0
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 150
                        }
                    }
                }

                HoverHandler {
                    id: hoverHandler

                    onHoveredChanged: {
                        if (hovered) {
                            lifetimeCalc()
                            hoverTimer.start()
                        } else {
                            hoverTimer.stop()
                        }
                    }

                    function lifetimeCalc() {
                        tableView.lifetime = threadpoolModule.lifetimeCalc(textCell.row)
                    }
                }

                Timer {
                    id: hoverTimer
                    interval: 1000
                    repeat: true
                    onTriggered: {
                        hoverHandler.lifetimeCalc()
                    }
                }

                TapHandler {
                    acceptedButtons: Qt.RightButton
                    onTapped: {
                        tableView.selectedRow = textCell.row
                        threadpoolMenu.popup()
                    }
                }

                Menu {
                    id: threadpoolMenu
                    MenuItem {
                        text: qsTr("Terminate")
                        icon.source: "qrc:/icon/stop.svg"
                        icon.width: 16; icon.height: 16
                        onTriggered: {
                            const index = tableView.model.index(tableView.selectedRow, 3);
                            threadpoolModule.threadStop(tableView.model.data(index, Qt.DisplayRole))
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: threadpoolModel

        function onRowsInserted() {
            hintItem.visible = false
            horizontalHeaderView.visible = true
            tableView.visible = true
        }

        function onRowsRemoved() {
            if (threadpoolModel.rowCount() === 0){
                hintItem.visible = true
                horizontalHeaderView.visible = false
                tableView.visible = false
            }
        }
    }
}