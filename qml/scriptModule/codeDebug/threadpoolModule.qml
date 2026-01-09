import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    anchors.fill: parent
    property bool modelVisible: standardItemModel ? standardItemModel.rowCount() > 0 : false

    Item {
        anchors.fill: parent
        visible: !modelVisible

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
        visible: modelVisible
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
        model: standardItemModel
        visible: modelVisible
        contentWidth: width
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
                    color: "#ebebeb"
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
                        const index = tableView.index(tableView.selectedRow, 3);
                        threadMenu.threadId = tableView.model.data(index, Qt.DisplayRole)
                        threadMenu.popup()
                    }
                }
            }
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
}