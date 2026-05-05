import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent
    property bool modelVisible: standardItemModel.rowCount() > 0

    Rectangle {
        anchors.fill: parent
        color: global.back
    }

    Item {
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
                model: standardItemModel
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

                delegate: SwitchDelegate {
                    implicitWidth: tableView.width
                    checked: model.whatsThis
                    text: model.display
                    background: Rectangle {
                        anchors.fill: parent
                        color: global.back
                    }

                    onClicked: portModule.portToggle(model.row)

                    HoverHandler {
                        onHoveredChanged: cursorShape = Qt.PointingHandCursor
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
