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
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            checkable: true
            icon.source: "qrc:/icon/clock.svg"
            icon.width: 16; icon.height: 16

            onClicked: logModule.timestampToggle(checked)
        }

        Button {
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            icon.source: "qrc:/icon/autoFitHeight.svg"
            icon.width: 16; icon.height: 16

            onClicked: heightDialog.open()
        }

        Button {
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
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16

            onClicked: textArea.clear()
        }
    }

    ScrollView {
        id: view
        Layout.fillWidth: true; Layout.fillHeight: true
        Layout.topMargin: 4; Layout.rightMargin: 4; Layout.bottomMargin: 4

        TextArea {
            id: textArea
            textFormat: TextEdit.RichText
            verticalAlignment: TextEdit.AlignTop
            property alias logTextDocument: textArea.textDocument

            HoverHandler {
                cursorShape: textArea.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onTapped: {
                    if (textArea.hoveredLink) {
                        Qt.openUrlExternally(textArea.hoveredLink)
                    }
                }
            }

            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: {
                    if (textArea.hoveredLink) {
                        linkMenu.url = textArea.hoveredLink
                        linkMenu.popup()
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        const objects = {
            "timestampButton": timestampButton,
            "textArea": textArea
        };
        logModule.propertyGet(objects)
    }
}