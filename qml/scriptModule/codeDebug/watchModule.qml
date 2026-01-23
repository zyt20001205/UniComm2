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
                width: 24; height: parent.height
                syncView: tableView
                clip: true
                interactive: false
                movableRows: true
                delegate: VerticalHeaderViewDelegate {
                    id: verticalHeaderViewDelegate
                    implicitWidth: verticalHeaderView.width; implicitHeight: 24
                    padding: 0

                    contentItem: Rectangle {
                        width: 24; height: 24
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

                Rectangle {
                    anchors.fill: parent
                    color: "#e0e0e0"
                    z: -1
                }

                delegate: Rectangle {
                    color: "white"
                    implicitWidth: {
                        if (column === tableView.columns - 1) {
                            let usedWidth = 0
                            for (let i = 0; i < tableView.columns - 1; i++) {
                                usedWidth += tableView.columnWidth(i)
                            }
                            return tableView.width - usedWidth
                        }
                        return Math.max(textMetrics.width + 12, 60)
                    }
                    implicitHeight: 24
                    required property int column
                    required property int row
                    property bool valueChanged: false

                    Rectangle {
                        anchors.fill: parent
                        radius: 6
                        color: "#ebebeb"
                        opacity: {
                            if (column === 0 && hoverHandler.hovered) {
                                return 1
                            } else if (column === 1 && valueChanged) {
                                return 1
                            } else {
                                return 0
                            }
                        }
                        Behavior on opacity {
                            NumberAnimation { duration: 150 }
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

                        ToolTip.visible: hoverHandler.hovered
                        ToolTip.delay: 500
                        ToolTip.text: model.whatsThis

                        onTextChanged: {
                            if (column === 1) {
                                valueChanged = true
                                highlightTimer.restart()
                            }
                        }
                    }

                    Timer {
                        id: highlightTimer
                        interval: 500

                        onTriggered: valueChanged = false
                    }

                    HoverHandler {
                        id: hoverHandler
                    }

                    TapHandler {
                        acceptedButtons: Qt.RightButton
                        gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                        onTapped: {
                            tableMenu.watchIndex = model.row
                            const index = tableView.index(row, 0);
                            tableMenu.watchUrl = tableView.model.data(index, Qt.WhatsThisRole)
                            tableMenu.watchExpression = tableView.model.data(index, Qt.DisplayRole)
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
