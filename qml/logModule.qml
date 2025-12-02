import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Window

RowLayout {
    Layout.fillWidth: true; Layout.fillHeight: true
    spacing: 0

    ColumnLayout {
        Layout.preferredWidth: 24; Layout.fillHeight: true
        Layout.alignment: Qt.AlignTop
        Layout.margins: 4

        Button {
            id: timestampButton
            objectName: "timestampButton"
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            checkable: true
            icon.source: "qrc:/icon/clock.svg"
            icon.width: 16; icon.height: 16

            onClicked: logModule.timestampToggle(checked)
        }

        Button {
            id: heightButton
            objectName: "heightButton"
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            icon.source: "qrc:/icon/autoFitHeight.svg"
            icon.width: 16; icon.height: 16

            onClicked: heightDialog.open()

            Dialog {
                id: heightDialog
                width: 200; height: 80
                modal: true
                onAccepted: logModule.heightWrite(heightInput.text)

                ColumnLayout {
                    Layout.fillWidth: true; Layout.fillHeight: true

                    Label {
                        Layout.fillWidth: true; Layout.preferredHeight: 24
                        text: qsTr("Max Line Count")
                    }

                    TextField {
                        id: heightInput
                        objectName: "heightInput"
                        Layout.fillWidth: true; Layout.fillHeight: true
                        Component.onCompleted: text = logModule.heightRead()
                        Keys.onReturnPressed: heightDialog.accept()
                    }
                }
            }
        }

        Button {
            id: saveButton
            objectName: "saveButton"
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            icon.source: "qrc:/icon/save.svg"
            icon.width: 16; icon.height: 16

            onClicked: fileDialog.open()

            FileDialog {
                id: fileDialog
                currentFolder: StandardPaths.standardLocations(StandardPaths.DesktopLocation)[0]
                fileMode: FileDialog.SaveFile
                nameFilters: ["Plain Text (*.txt)", "PDF (*.pdf)", "Rich Text (*.html)"]
                selectedFile: "log_" + Qt.formatDateTime(new Date(), "yyyyMMdd_HHmmss")
                onAccepted: logModule.logSave(selectedFile)
            }
        }

        Button {
            id: clearButton
            objectName: "clearButton"
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16

            onClicked: logTextArea.clear()
        }
    }

    ScrollView {
        id: view
        Layout.fillWidth: true; Layout.fillHeight: true
        Layout.topMargin: 4; Layout.rightMargin: 4; Layout.bottomMargin: 4

        TextArea {
            id: logTextArea
            objectName: "logTextArea"
            textFormat: TextEdit.RichText
            verticalAlignment: TextEdit.AlignTop
            property url fileUrl: ""
            property alias logTextDocument: logTextArea.textDocument

            HoverHandler {
                cursorShape: logTextArea.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onTapped: {
                    if (logTextArea.hoveredLink) {
                        Qt.openUrlExternally(logTextArea.hoveredLink)
                    }
                }
            }

            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: {
                    if (logTextArea.hoveredLink) {
                        logTextArea.fileUrl = logTextArea.hoveredLink
                        linkMenu.popup()
                    }
                }
            }

            Menu {
                id: linkMenu
                MenuItem {
                    text: qsTr("Copy URL")
                    icon.source: "qrc:/icon/copy.svg"
                    icon.width: 16; icon.height: 16
                    onTriggered: logModule.urlCopy(logTextArea.fileUrl)
                }

                Menu {
                    id: openSubMenu
                    title: qsTr("Open In")
                    icon.source: "qrc:/icon/open.svg"
                    icon.width: 16; icon.height: 16

                    MenuItem {
                        text: qsTr("Explorer")
                        onTriggered: logModule.openInExplorer(logTextArea.fileUrl)
                    }
                    MenuItem {
                        text: qsTr("Application")
                        onTriggered: logModule.openInApplication(logTextArea.fileUrl)
                    }
                }
            }
        }
    }
}