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
            property string role: "assistant"
            property string status: qsTr("Idle")
            Layout.fillWidth: true; Layout.preferredHeight: 32

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border.color: chatStatus.role === "user" ? global.brandBack :
                        chatStatus.role === "assistant" ? global.successBack3 :
                            chatStatus.role === "tool" ? global.warningBack3 : global.dangerBack3
                border.width: 1
                radius: 6
            }

            RowLayout {
                anchors.fill: parent

                Label {
                    text: chatStatus.status
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    Layout.leftMargin: 6
                }

                Button {
                    visible: chatStatus.role === "tool"
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    flat: true
                    icon.source: "qrc:/icon/checkmark.svg"
                    icon.width: 16; icon.height: 16
                    Layout.preferredWidth: 24; Layout.preferredHeight: 24

                    onClicked: llmModule.permissionSet(true)
                }

                Button {
                    visible: chatStatus.role === "tool"
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    flat: true
                    icon.source: "qrc:/icon/dismiss.svg"
                    icon.width: 16; icon.height: 16
                    Layout.preferredWidth: 24; Layout.preferredHeight: 24

                    onClicked: llmModule.permissionSet(false)
                }

                BusyIndicator {
                    visible: chatStatus.role === "user" || chatStatus.role === "tool"
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

        RowLayout {
            Layout.fillWidth: true; Layout.preferredHeight: 20

            Button {
                id: modeButton
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                Layout.preferredWidth: modeButtonTextMetrics.width + 8; Layout.preferredHeight: 20

                onClicked: {
                    const globalPos = modeButton.mapToGlobal(0, modeButton.height);
                    const localPos = modeMenu.parent.mapFromGlobal(globalPos.x, globalPos.y);
                    modeMenu.popup(localPos.x, localPos.y)
                }

                TextMetrics {
                    id: modeButtonTextMetrics
                    text: modeButton.text
                    font: modeButton.font
                }
            }

            Button {
                id: modelButton
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                Layout.preferredWidth: modelButtonTextMetrics.width + 8; Layout.preferredHeight: 20

                onClicked: {
                    const globalPos = modelButton.mapToGlobal(0, modelButton.height);
                    const localPos = modelMenu.parent.mapFromGlobal(globalPos.x, globalPos.y);
                    modelMenu.popup(localPos.x, localPos.y)
                }

                TextMetrics {
                    id: modelButtonTextMetrics
                    text: modelButton.text
                    font: modelButton.font
                }
            }

            Item {
                Layout.fillWidth: true
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
            Layout.alignment: role === "user" ? Qt.AlignRight : Qt.AlignLeft
            property string role

            background: Rectangle {
                color: chatLabel.role === "user" ? global.brandBack :
                        chatLabel.role === "assistant" ? global.stroke :
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

    function append(role, text, status) {
        if (text) {
            const chatLabel = chatComponent.createObject(chatColumn, {
                "role": role,
                "text": text
            })
        }

        chatStatus.role = role
        chatStatus.status = status

        scrollTimer.restart()
    }

    Component.onCompleted: {
        const objects = {
            "textArea": textArea,
            "modeButton": modeButton,
            "modelButton": modelButton
        };
        llmModule.propertyGet(objects)
    }
}