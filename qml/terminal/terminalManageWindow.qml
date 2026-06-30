import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Dialogs
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: global.back
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        RowLayout {
            id: toolBar
            Layout.fillWidth: true
            spacing: 0

            Item {
                Layout.fillWidth: true
            }

            Button {
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                flat: true
                icon.source: "qrc:/icon/add.svg"
                icon.width: 16; icon.height: 16
                Layout.preferredWidth: 24; Layout.preferredHeight: 24

                onClicked: console.log("add")
            }

            Button {
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                flat: true
                icon.source: "qrc:/icon/subtract.svg"
                icon.width: 16; icon.height: 16
                Layout.preferredWidth: 24; Layout.preferredHeight: 24

                onClicked: console.log("remove")
            }
        }

        TableView {
            id: tableView
            alternatingRows: false
            clip: true
            editTriggers: TableView.NoEditTriggers
            rowSpacing: 1
            model: terminalModel
            contentWidth: width
            Layout.fillWidth: true; Layout.fillHeight: true
            property int hoveredRow: -1
            property int selectedRow: -1

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
                implicitWidth: tableView.width
                implicitHeight: 32

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

                Rectangle {
                    anchors.fill: parent
                    radius: 6
                    color: tableView.selectedRow === row ? global.backSelected : "transparent"
                }

                Label {
                    anchors.fill: parent
                    leftPadding: 6
                    horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                    text: model.display || ""
                    elide: Text.ElideRight
                }

                HoverHandler {
                    onHoveredChanged: {
                        if (hovered) {
                            tableView.hoveredRow = row
                        } else if (tableView.hoveredRow === row) {
                            tableView.hoveredRow = -1
                        }
                    }
                }

                TapHandler {
                    acceptedButtons: Qt.LeftButton
                    gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                    onTapped: {
                        tableView.selectedRow = row
                        const session = model.session
                        programTextField.text = session.program
                        argumentsTextField.text = session.arguments
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Program")
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true

                TextField {
                    id: programTextField
                    Layout.fillWidth: true
                }

                Button {
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    icon.source: "qrc:/icon/folder.svg"
                    icon.width: 16; icon.height: 16
                    Layout.preferredWidth: 32; Layout.preferredHeight: 32

                    onClicked: {
                        const url = programTextField.text
                        fileDialog.selectedFile = url
                        fileDialog.currentFolder = url.substring(0, url.lastIndexOf('/'))
                        fileDialog.open()
                    }
                }

                FileDialog {
                    id: fileDialog
                    fileMode: FileDialog.OpenFile
                    nameFilters: ["Executable files (*.exe)", "All files (*)"]
                    onAccepted: programTextField.text = selectedFile
                }
            }

            Label {
                text: qsTr("Arguments")
                Layout.fillWidth: true
            }

            TextField {
                id: argumentsTextField
                Layout.fillWidth: true

                onEditingFinished: {
                    const index = terminalModel.index(tableView.selectedRow, 0)
                    const session = terminalModel.data(index, Qt.UserRole + 1)
                    session.arguments = argumentsTextField.text
                    terminalModel.setData(index, session, Qt.UserRole + 1)
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignRight

            Button {
                text: qsTr("Apply")
            }

            Button {
                text: qsTr("Save and Exit")

                onClicked: terminalModule.terminalSave()
            }
        }
    }
}
