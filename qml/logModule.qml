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
            ToolTip.text: checked ? qsTr("Hide Timestamp") : qsTr("Show Timestamp")
            ToolTip.visible: hovered

            onClicked: logModule.timestampToggle(checked)
        }

        Button {
            id: wrapButton
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            checkable: true
            icon.source: "qrc:/icon/wrap.svg"
            icon.width: 16; icon.height: 16
            ToolTip.text: checked ? qsTr("Disable Wrap") : qsTr("Enable Wrap")
            ToolTip.visible: hovered

            onClicked: logModule.wrapToggle(checked)
        }

        Button {
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            icon.source: "qrc:/icon/autoFitHeight.svg"
            icon.width: 16; icon.height: 16
            ToolTip.text: qsTr("Maximum Line Count")
            ToolTip.visible: hovered

            onClicked: heightDialog.open()
        }

        Button {
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            icon.source: "qrc:/icon/save.svg"
            icon.width: 16; icon.height: 16
            ToolTip.text: qsTr("Save")
            ToolTip.visible: hovered

            onClicked: {
                if (logModule.logSaveCheck()) {
                    fileDialog.open()
                }
            }

            FileDialog {
                id: fileDialog
                currentFolder: StandardPaths.standardLocations(StandardPaths.DesktopLocation)[0]
                fileMode: FileDialog.SaveFile
                nameFilters: ["Plain Text (*.txt)", "PDF (*.pdf)", "Rich Text (*.html)"]
                currentFile: currentFolder + "/log_" + Qt.formatDateTime(new Date(), "yyyyMMdd_HHmmss")
                onAccepted: logModule.logSave(selectedFile)
            }
        }

        DelayButton {
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 4; rightPadding: 4; topPadding: 4; bottomPadding: 4
            contentItem: Image {
                source: "qrc:/icon/delete.svg"
                width: 16; height: 16
            }
            delay: 1000
            ToolTip.text: qsTr("Clear")
            ToolTip.visible: hovered

            onActivated: {
                progress = 0
                textArea.clear()
            }
        }
    }

    ScrollView {
        Layout.fillWidth: true; Layout.fillHeight: true
        Layout.topMargin: 4; Layout.rightMargin: 4; Layout.bottomMargin: 4

        TextArea {
            id: textArea
            textFormat: TextEdit.RichText
            verticalAlignment: TextEdit.AlignTop
            wrapMode: wrapButton.checked ? TextEdit.Wrap : TextEdit.NoWrap

            HoverHandler {
                id: hoverHandler
                cursorShape: textArea.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
            }

            ToolTip {
                id: tooltip
                visible: textArea.hoveredLink
                text: "Ctrl+Left Click to open link"
                x: hoverHandler.point.position.x + 10
                y: hoverHandler.point.position.y + 10
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                acceptedModifiers: Qt.ControlModifier
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
            "wrapButton": wrapButton,
            "textArea": textArea
        };
        logModule.propertyGet(objects)
    }
}