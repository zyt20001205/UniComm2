import QtGraphs
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent

    function formatBytes(value) {
        if (value < 1024) return value + " B"
        if (value < 1048576) return (value / 1024).toFixed(value < 10240 ? 1 : 0) + " KB"
        return (value / 1048576).toFixed(value < 10485760 ? 1 : 0) + " MB"
    }

    Rectangle {
        anchors.fill: parent
        color: global.back
    }

    Item {
        anchors.fill: parent
        visible: portModel.empty

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

    Item {
        anchors.fill: parent
        visible: !portModel.empty

        VerticalHeaderView {
            id: verticalHeaderView
            anchors.left: parent.left
            width: 32; height: parent.height
            syncView: tableView
            clip: true
            interactive: false
            movableRows: true
            delegate: VerticalHeaderViewDelegate {
                id: verticalHeaderViewDelegate
                implicitWidth: verticalHeaderView.width; implicitHeight: 32
                padding: 0

                contentItem: Rectangle {
                    width: 32; height: 32
                    color: global.back

                    IconImage {
                        width: 16; height: 16
                        anchors.centerIn: parent
                        color: global.fore
                        source: "qrc:/icon/drag.svg"
                    }
                }

                HoverHandler {
                    onHoveredChanged: cursorShape = Qt.OpenHandCursor
                }
            }
            property var moves: []

            Rectangle {
                anchors.fill: parent
                color: global.stroke
            }

            Timer {
                id: timer
                interval: 10
                onTriggered: {
                    let index = -1
                    let distance = -1
                    let currentDistance;
                    for (let i = 0; i < verticalHeaderView.moves.length; ++i) {
                        let move = verticalHeaderView.moves[i]
                        currentDistance = Math.abs(move.oldVisualIndex - move.newVisualIndex)
                        if (currentDistance > distance) {
                            distance = currentDistance
                            index = i
                        }
                    }
                    let move = verticalHeaderView.moves[index]
                    portModule.portSwap(move.oldVisualIndex, move.newVisualIndex)
                    verticalHeaderView.clearRowReordering()
                    tableView.clearRowReordering()
                    verticalHeaderView.moves = []
                }
            }

            onRowMoved: (logicalIndex, oldVisualIndex, newVisualIndex) => {
                moves.push({oldVisualIndex, newVisualIndex})
                timer.restart()
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
            model: portModel
            contentWidth: width
            property int hoveredRow: -1

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

            delegate: Item {
                id: portDelegate
                implicitWidth: tableView.width

                Rectangle {
                    anchors.fill: parent
                    color: global.back
                }

                Rectangle {
                    anchors.fill: parent
                    radius: 6
                    color: global.backHover
                    opacity: tableView.hoveredRow === row ? 1 : 0
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 150
                        }
                    }
                }

                ColumnLayout {
                    anchors.fill: parent

                    RowLayout {
                        Layout.fillWidth: true; Layout.preferredHeight: 32

                        Label {
                            leftPadding: 6
                            horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                            text: model.display || ""
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        Switch {
                            checked: model.active

                            onClicked: portModule.portToggle(model.row)
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true; Layout.preferredHeight: 80

                        Item {
                            Layout.fillWidth: true; Layout.fillHeight: true

                            GraphsView {
                                anchors.fill: parent
                                theme: GraphsTheme {
                                    theme: GraphsTheme.Theme.QtGreen
                                    // backgroundVisible: false
                                    // plotAreaBackgroundVisible: false
                                    // gridVisible: false
                                }

                                PieSeries {
                                    pieSize: 0.88
                                    holeSize: 0.58

                                    PieSlice {
                                        value: model.used
                                    }

                                    PieSlice {
                                        value: model.capacity - model.used
                                        // borderWidth: 0
                                    }
                                }
                            }
                        }
                    }
                }

                HoverHandler {
                    id: hoverHandler
                }

                TapHandler {
                    acceptedButtons: Qt.RightButton
                    gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                    onSingleTapped: {
                        tableMenu.portIndex = model.row
                        tableMenu.popup()
                    }
                }
            }

            TapHandler {
                acceptedButtons: Qt.RightButton

                onSingleTapped: rootMenu.popup()
            }
        }
    }
}
