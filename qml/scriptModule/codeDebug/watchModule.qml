import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent
    property bool modelVisible: standardItemModel.rowCount() > 0

    Item {
        anchors.fill: parent
        visible: !modelVisible

        RowLayout {
            anchors.centerIn: parent

            Label {
                text: qsTr("Nothing watched.")
                font.pixelSize: 16
                Layout.alignment: Qt.AlignVCenter
            }

            Image {
                source: "qrc:/icon/eye.svg"
                Layout.alignment: Qt.AlignVCenter
            }
        }
    }

    Component {
        id: tableComponent

        Item {
            anchors.fill: parent
            visible: modelVisible

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
                property var moves: []

                Rectangle {
                    anchors.fill: parent
                    color: "#e0e0e0"
                    z: -1
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
                        watchModule.watchSwap(move.oldVisualIndex, move.newVisualIndex)
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
                rowSpacing: 1; columnSpacing: 1
                resizableColumns: true
                model: standardItemModel
                contentWidth: width
                delegate: ItemDelegate {
                    implicitWidth: {
                        if (column === 0) {
                            return tableView.width / 3
                        } else {
                            return tableView.width - tableView.columnWidth(0)
                        }
                    }
                    implicitHeight: 32
                    text: model.display
                    font.pixelSize: 16
                    background: Rectangle {
                        color: "white"
                    }
                    ToolTip.text: model.whatsThis
                    ToolTip.visible: hovered

                    onTextChanged: {
                        if (column === 1) {
                            highlightRect.opacity = 1
                            highlightTimer.restart()
                        }
                    }

                    Timer {
                        id: highlightTimer
                        interval: 500

                        onTriggered: highlightRect.opacity = 0
                    }

                    Rectangle {
                        id: highlightRect
                        anchors.fill: parent
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
                    }

                    TapHandler {
                        acceptedButtons: Qt.RightButton
                        gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                        onSingleTapped: {
                            tableMenu.watchIndex = model.row
                            const index = tableView.model.index(row, 0);
                            tableMenu.watchUrl = tableView.model.data(index, Qt.WhatsThisRole)
                            tableMenu.watchKey = tableView.model.data(index, Qt.DisplayRole)
                            tableMenu.popup()
                        }
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    color: "#e0e0e0"
                    z: -1
                }

                TapHandler {
                    acceptedButtons: Qt.RightButton

                    onSingleTapped: rootMenu.popup()
                }
            }
        }
    }

    Loader {
        id: tableLoader
        anchors.fill: parent
        sourceComponent: tableComponent
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
    function reload() {
        tableLoader.active = false
        tableLoader.active = true
    }
}
