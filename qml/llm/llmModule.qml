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

        Item {
            id: chatStatus
            property string role: "output"
            Layout.fillWidth: true; Layout.preferredHeight: 32

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border.color: chatStatus.role === "input" ? global.brandBack :
                        chatStatus.role === "output" ? global.successBack3 : global.dangerBack3
                border.width: 1
                radius: 6
            }

            RowLayout {
                anchors.fill: parent

                Label {
                    text: chatStatus.role === "input" ? qsTr("Responding...") :
                            chatStatus.role === "output" ? qsTr("Idle") : qsTr("Error")
                    Layout.fillWidth: true
                    Layout.leftMargin: 6
                }

                BusyIndicator {
                    visible: chatStatus.role === "input"
                    running: visible
                    Layout.preferredWidth: 24; Layout.preferredHeight: 24
                    Layout.rightMargin: 4
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
                            textArea.clear()
                        }
                        event.accepted = true
                    }
                }
            }
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
            Layout.alignment: role === "input" ? Qt.AlignRight : Qt.AlignLeft
            property string role

            background: Rectangle {
                color: chatLabel.role === "input" ? global.brandBack :
                        chatLabel.role === "output" ? global.stroke :
                            chatLabel.role === "tool" ? global.warningBack2 : global.dangerBack2
                radius: 6
            }

            TextMetrics {
                id: chatMetrics
                text: chatLabel.text
                font: chatLabel.font
            }
        }
    }

    Timer {
        id: scrollTimer
        interval: 50
        onTriggered: {
            const bottom = Math.max(0, 1.0 - chatView.ScrollBar.vertical.size)
            if (bottom > 0) {
                scrollAnim.to = bottom
                scrollAnim.restart()
            }
        }
    }

    NumberAnimation {
        id: scrollAnim
        target: chatView.ScrollBar.vertical
        property: "position"
        duration: 300
        easing.type: Easing.OutQuad
    }

    function append(role, text) {
        const chatLabel = chatComponent.createObject(chatColumn, {
            "role": role,
            "text": text
        })

        if (["input", "output", "error"].includes(role)) {
            chatStatus.role = role
        }

        scrollTimer.restart()
    }

    Component.onCompleted: {
        const objects = {
            "textArea": textArea
        };
        llmModule.propertyGet(objects)
    }
}