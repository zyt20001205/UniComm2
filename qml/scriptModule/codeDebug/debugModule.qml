import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 5
        spacing: 5

        RowLayout {
            Layout.fillWidth: true; Layout.preferredHeight: 24
            Layout.alignment: Qt.AlignTop

            Button {
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                icon.source: "qrc:/icon/stop.svg"
                icon.width: 16; icon.height: 16
                ToolTip.text: qsTr("Terminate")
                ToolTip.visible: hovered
                onClicked: debugModule.stateSet(combobox.currentText, 0)
            }

            Button {
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                icon.source: "qrc:/icon/play.svg"
                icon.width: 16; icon.height: 16
                ToolTip.text: qsTr("Resume")
                ToolTip.visible: hovered
                onClicked: debugModule.stateSet(combobox.currentText, 1)
            }

            Button {
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                icon.source: "qrc:/icon/pause.svg"
                icon.width: 16; icon.height: 16
                ToolTip.text: qsTr("Pause")
                ToolTip.visible: hovered
                onClicked: debugModule.stateSet(combobox.currentText, 2)
            }

            Button {
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                icon.source: "qrc:/icon/debugStepOver.svg"
                icon.width: 16; icon.height: 16
                ToolTip.text: qsTr("Step Over")
                ToolTip.visible: hovered
                onClicked: debugModule.stateSet(combobox.currentText, 3)
            }

            Button {
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                icon.source: "qrc:/icon/debugStepInto.svg"
                icon.width: 16; icon.height: 16
                ToolTip.text: qsTr("Step Into")
                ToolTip.visible: hovered
                onClicked: debugModule.stateSet(combobox.currentText, 4)
            }

            Button {
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                icon.source: "qrc:/icon/debugStepOut.svg"
                icon.width: 16; icon.height: 16
                ToolTip.text: qsTr("Step Out")
                ToolTip.visible: hovered
                onClicked: debugModule.stateSet(combobox.currentText, 5)
            }

            Button {
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                icon.source: "qrc:/icon/debugContinue.svg"
                icon.width: 16; icon.height: 16
                ToolTip.text: qsTr("Run To Cursor")
                ToolTip.visible: hovered
                onClicked: debugModule.stateSet(combobox.currentText, 6)
            }
        }

        ComboBox {
            id: combobox
            Layout.fillWidth: true;
            Layout.alignment: Qt.AlignTop
            model: stringListModel
            textRole: "display"
        }

        TableView {
            id: tableView
            Layout.fillWidth: true; Layout.fillHeight: true;
            alternatingRows: false
            clip: true
            editTriggers: TableView.NoEditTriggers
            rowSpacing: 1
            model: standardItemModel
            contentWidth: width

            Rectangle {
                anchors.fill: parent
                color: "#e0e0e0"
                z: -1
            }

            delegate: Rectangle {
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
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        text: model.display
                        elide: Text.ElideRight

                        ToolTip.visible: hoverHandler.hovered
                        ToolTip.delay: 500
                        ToolTip.text: model.display
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
                    }

                    TapHandler {
                        acceptedButtons: Qt.LeftButton
                        onTapped: tableView.indicatorInsert(textCell.row)
                    }
                }
            }
    }

    Connections {
        target: stringListModel

        function onRowsInserted() {
            combobox.currentIndex = stringListModel.rowCount() - 1
        }

        function onRowsRemoved() {
            combobox.currentIndex = stringListModel.rowCount() - 1
        }
    }
}