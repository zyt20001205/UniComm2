import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent
    property var chatMap: ({})
    property var lastChatLabel: null

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
            property string status: "idle"
            property string text: qsTr("Idle")
            Layout.fillWidth: true; Layout.preferredHeight: 32

            Rectangle {
                id: chatStatusRect
                anchors.fill: parent
                color: "transparent"
                border.color: chatStatus.status === "busy" ? global.brandBack :
                            chatStatus.status === "idle" ? global.successBack3 :
                            chatStatus.status === "waiting" ? global.warningBack3 : global.dangerBack3
                border.width: 1
                radius: 6

                SequentialAnimation on border.color {
                    running: chatStatus.status === "waiting"
                    loops: Animation.Infinite
                    ColorAnimation {
                        to: global.back
                        duration: 1000
                    }
                    ColorAnimation {
                        to: chatStatus.status === "busy" ? global.brandBack :
                            chatStatus.status === "idle" ? global.successBack3 :
                            chatStatus.status === "waiting" ? global.warningBack3 : global.dangerBack3
                        duration: 1000
                    }
                }
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 6; anchors.rightMargin: 6

                Label {
                    text: chatStatus.text
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                Button {
                    visible: chatStatus.status === "waiting"
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    flat: true
                    icon.source: "qrc:/icon/checkmark.svg"
                    icon.width: 16; icon.height: 16
                    Layout.preferredWidth: 24; Layout.preferredHeight: 24

                    onClicked: {
                        chatAppend(null, " ✓")
                        llmModule.permissionSet(true)
                    }
                }

                Button {
                    visible: chatStatus.status === "waiting"
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    flat: true
                    icon.source: "qrc:/icon/dismiss.svg"
                    icon.width: 16; icon.height: 16
                    Layout.preferredWidth: 24; Layout.preferredHeight: 24

                    onClicked: {
                        chatAppend(null, " ✗")
                        llmModule.permissionSet(false)
                    }
                }

                BusyIndicator {
                    visible: chatStatus.status === "busy"
                    running: visible
                    Layout.preferredWidth: 16; Layout.preferredHeight: 16
                }

                IconImage {
                    visible: chatStatus.status === "idle"
                    color: global.successBack3
                    source: "qrc:/icon/checkmark.svg"
                    Layout.preferredWidth: 16; Layout.preferredHeight: 16
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
                        }
                        event.accepted = true
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true; Layout.preferredHeight: 24

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

            Button {
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                enabled: llmModule.active || textArea.text.trim().length > 0
                flat: true
                icon.source: llmModule.active ? "qrc:/icon/stop.svg" : "qrc:/icon/send.svg"
                icon.width: 16; icon.height: 16
                Layout.preferredWidth: 24; Layout.preferredHeight: 24

                onClicked: llmModule.active ? llmModule.requestCancel() : llmModule.requestSend()
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
                            chatLabel.role === "tool" ? global.backSelected : global.dangerBack2
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

    function chatCreate(id, role, text) {
        const obj = chatComponent.createObject(chatColumn, {
            role: role,
            text: text
        })
        rootItem.lastChatLabel = obj
        rootItem.chatMap[id] = obj
        scrollTimer.restart()
    }

    function chatAppend(id, text) {
        if (id === null) {
            rootItem.lastChatLabel.text += text
        } else {
            rootItem.chatMap[id].text += text
        }
        scrollTimer.restart()
    }

    function chatVisible(id, status) {
        rootItem.chatMap[id].visible = status
    }

    function statusSet(status, text) {
        chatStatus.status = status
        chatStatus.text = text
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