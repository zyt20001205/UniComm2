import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts

Item {
    anchors.fill: parent
    property bool modelVisible: standardItemModel ? standardItemModel.rowCount() > 0 : false

    Rectangle {
        anchors.fill: parent
        color: global.back
    }

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

            IconImage {
                source: "qrc:/icon/snooze.svg"
                color: global.fore
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

            contentItem: Label {
                anchors.fill: parent
                elide: Text.ElideRight
                leftPadding: 6
                horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                text: horizontalHeader[horizontalHeaderViewDelegate.index]
            }
        }
    }

    TableView {
        id: tableView
        anchors.top: horizontalHeaderView.bottom; anchors.bottom: parent.bottom
        width: parent.width
        alternatingRows: false
        clip: true
        editTriggers: TableView.NoEditTriggers
        rowSpacing: 1
        model: standardItemModel
        visible: modelVisible
        contentWidth: width

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
            palette {
                mid: global.stroke
                dark: global.strokePressed
            }
        }

        Rectangle {
            anchors.fill: parent
            color: global.stroke
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

            Item {
                implicitWidth: 24; implicitHeight: 24

                Rectangle {
                    anchors.fill: parent
                    color: model.status == 0 ? global.successBack2 :
                            model.status == 1 ? global.warningBack2 : global.dangerBack2
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    running: true
                    width: 20; height: 20
                }
            }
        }

        Component {
            id: textCellDelegate

            Item {
                implicitWidth: {
                    if (column === tableView.columns - 1) {
                        let usedWidth = 0
                        for (let i = 0; i < tableView.columns - 1; i++) {
                            usedWidth += tableView.columnWidth(i)
                        }
                        return tableView.width - usedWidth
                    }
                    return Math.max(textMetrics.width + 16, 60)
                }
                implicitHeight: 24
                required property int column

                Rectangle {
                    anchors.fill: parent
                    color: model.status == 0 ? global.successBack2 :
                            model.status == 1 ? global.warningBack2 : global.dangerBack2
                }

                Rectangle {
                    anchors.fill: parent
                    radius: 6
                    color: global.backHover
                    opacity: hoverHandler.hovered ? 1 : 0
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 150
                        }
                    }
                }

                TextMetrics {
                    id: textMetrics
                    font: label.font
                    text: model.display || ""
                }

                Label {
                    id: label
                    anchors.fill: parent
                    leftPadding: 6
                    horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                    text: model.display
                    elide: Text.ElideRight
                }

                HoverHandler {
                    id: hoverHandler
                }

                TapHandler {
                    acceptedButtons: Qt.RightButton
                    onTapped: {
                        threadMenu.threadId = model.threadId
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