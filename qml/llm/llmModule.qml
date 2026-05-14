import QtCore
import QtQuick
import QtQuick.Controls
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
        anchors.margins: 6

        ScrollView {
            id: chatView
            Layout.fillWidth: true; Layout.fillHeight: true
            rightPadding: 14

            ScrollBar.vertical: ScrollBar {
                x: parent.mirrored ? 0 : parent.width - width
                y: parent.topPadding
                height: parent.availableHeight
                active: parent.ScrollBar.horizontal.active
                policy: ScrollBar.AsNeeded
                palette {
                    mid: global.stroke
                    dark: global.strokePressed
                }
            }

            ColumnLayout {
                id: chatColumn
                width: chatView.availableWidth
            }
        }

        Component {
            id: chatComponent

            Label {
                id: chatLabel
                padding: 6
                text: parent.text
                textFormat: TextEdit.MarkdownText
                wrapMode: Text.Wrap
                Layout.preferredWidth: Math.min(chatView.availableWidth, chatMetrics.width + 20)
                Layout.alignment: level === "input" ? Qt.AlignRight : Qt.AlignLeft
                property string level

                background: Rectangle {
                    color: chatLabel.level === "input" ? global.brandBack :
                        chatLabel.level === "output" ? global.stroke :
                        chatLabel.level === "tool" ? global.warningBack2 : global.dangerBack2
                    radius: 6
                }

                TextMetrics {
                    id: chatMetrics
                    text: chatLabel.text
                    font: chatLabel.font
                }
            }
        }

        ScrollView {
            Layout.fillWidth: true; Layout.preferredHeight: 100

            ScrollBar.vertical: ScrollBar {
                x: parent.mirrored ? 0 : parent.width - width
                y: parent.topPadding
                height: parent.availableHeight
                active: parent.ScrollBar.horizontal.active
                policy: ScrollBar.AsNeeded
                palette {
                    mid: global.stroke
                    dark: global.strokePressed
                }
            }

            ScrollBar.horizontal: ScrollBar {
                x: parent.leftPadding
                y: parent.height - height
                width: parent.availableWidth
                active: parent.ScrollBar.vertical.active
                policy: ScrollBar.AsNeeded
                palette {
                    mid: global.stroke
                    dark: global.strokePressed
                }
            }

            TextArea {
                id: textArea
                textFormat: TextEdit.PlainText
                verticalAlignment: TextEdit.AlignTop
                wrapMode: TextEdit.Wrap
                ContextMenu.menu: null

                Keys.onPressed: (event) => {
                    if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && !(event.modifiers & Qt.ShiftModifier)) {
                        if (textArea.text.trim().length > 0) {
                            llmModule.requestSend()
                            rootItem.append(textArea.text, "input")
                            textArea.clear()
                        }
                        event.accepted = true
                    }
                }
            }
        }
    }

    function append(text, level) {
        const chatLabel = chatComponent.createObject(chatColumn, {
            "text": text,
            "level": level
        })

        Qt.callLater(function() {
            chatView.ScrollBar.vertical.position = 1.0 - chatView.ScrollBar.vertical.size
        })
    }

    Component.onCompleted: {
        const objects = {
            "textArea": textArea
        };
        llmModule.propertyGet(objects)
    }
}