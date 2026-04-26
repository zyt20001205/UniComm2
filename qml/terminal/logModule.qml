import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Window

Item {
    id: rootItem
    anchors.fill: parent

    RowLayout {
        anchors.fill: parent
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

                HoverHandler {
                    onHoveredChanged: {
                        if (!hovered) {
                            mainToolTip.text = ""
                        }
                    }
                    onPointChanged: {
                        mainToolTip.position = parent.mapToGlobal(point.position)
                        mainToolTip.text = parent.checked ? qsTr("Hide Timestamp") : qsTr("Show Timestamp")
                    }
                }
            }

            Button {
                id: wrapButton
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                checkable: true
                icon.source: "qrc:/icon/wrap.svg"
                icon.width: 16; icon.height: 16

                onClicked: logModule.wrapToggle(checked)

                HoverHandler {
                    onHoveredChanged: {
                        if (!hovered) {
                            mainToolTip.text = ""
                        }
                    }
                    onPointChanged: {
                        mainToolTip.position = parent.mapToGlobal(point.position)
                        mainToolTip.text = parent.checked ? qsTr("Disable Wrap") : qsTr("Enable Wrap")
                    }
                }
            }

            Button {
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                icon.source: "qrc:/icon/autoFitHeight.svg"
                icon.width: 16; icon.height: 16

                onClicked: heightDialog.open()

                HoverHandler {
                    onHoveredChanged: {
                        if (!hovered) {
                            mainToolTip.text = ""
                        }
                    }
                    onPointChanged: {
                        mainToolTip.position = parent.mapToGlobal(point.position)
                        mainToolTip.text = qsTr("Maximum Line Count")
                    }
                }
            }

            Button {
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                icon.source: "qrc:/icon/save.svg"
                icon.width: 16; icon.height: 16

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

                HoverHandler {
                    onHoveredChanged: {
                        if (!hovered) {
                            mainToolTip.text = ""
                        }
                    }
                    onPointChanged: {
                        mainToolTip.position = parent.mapToGlobal(point.position)
                        mainToolTip.text = qsTr("Save")
                    }
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

                onActivated: {
                    progress = 0
                    textArea.clear()
                }

                HoverHandler {
                    onHoveredChanged: {
                        if (!hovered) {
                            mainToolTip.text = ""
                        }
                    }
                    onPointChanged: {
                        mainToolTip.position = parent.mapToGlobal(point.position)
                        mainToolTip.text = qsTr("Clear")
                    }
                }
            }
        }

        ScrollView {
            Layout.fillWidth: true; Layout.fillHeight: true
            Layout.topMargin: 4; Layout.rightMargin: 4; Layout.bottomMargin: 4

            TextArea {
                id: textArea
                readOnly: true
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
                    text: qsTr("Ctrl + Click")
                    x: hoverHandler.point.position.x + 10
                    y: hoverHandler.point.position.y + 10
                }

                TapHandler {
                    acceptedButtons: Qt.LeftButton
                    acceptedModifiers: Qt.ControlModifier
                    onTapped: {
                        if (textArea.hoveredLink) {
                            const url = textArea.hoveredLink
                            if (url.startsWith("request.expand://")) {
                                logModule.linkClick(url)
                            } else {
                                Qt.openUrlExternally(url)
                            }
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
}

