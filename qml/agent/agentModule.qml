import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent
    property var chatMap: ({})
    property var lastChatTextArea: null

    Rectangle {
        anchors.fill: parent
        color: global.back
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 6

        RowLayout {
            Layout.fillWidth: true; Layout.preferredHeight: 30

            ComboBox {
                id: topicComboBox
                enabled: agentModule.state === 0
                model: topicStandardItemModel
                textRole: "display"
                valueRole: "display"
                Layout.fillWidth: true; Layout.preferredHeight: 30

                onCurrentTextChanged: agentModule.conversationLoad(topicComboBox.currentText)
            }

            Button {
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                enabled: agentModule.state === 0
                flat: true
                icon.source: "qrc:/icon/rename.svg"
                icon.width: 16; icon.height: 16
                Layout.preferredWidth: 24; Layout.preferredHeight: 24

                onClicked: {
                    renameDialog.oldTopic = topicComboBox.currentText
                    renameDialog.open()
                }
            }

            Button {
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                enabled: agentModule.state === 0
                flat: true
                icon.source: "qrc:/icon/add.svg"
                icon.width: 16; icon.height: 16
                Layout.preferredWidth: 24; Layout.preferredHeight: 24

                onClicked: agentModule.conversationCreate()
            }

            Button {
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                checkable: true
                enabled: topicComboBox.currentText && agentModule.state === 0
                flat: true
                icon.source: checked ? "qrc:/icon/checkmark.svg" : "qrc:/icon/delete.svg"
                icon.width: 16; icon.height: 16
                Layout.preferredWidth: 24; Layout.preferredHeight: 24

                onToggled: {
                    if (!checked) {
                        agentModule.conversationDelete(topicComboBox.currentText)
                    }
                }

                Timer {
                    interval: 1000
                    running: parent.checked
                    onTriggered: parent.checked = false
                }
            }
        }

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
            property string text: qsTr("Ready")
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
                        chatAppend("", " ✓")
                        agentModule.permissionSet(true)
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
                        chatAppend("", " ✗")
                        agentModule.permissionSet(false)
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
                            agentModule.state = 1
                        }
                        event.accepted = true
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true; Layout.preferredHeight: 24

            Button {
                id: mcpButton
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                enabled: agentModule.state === 0
                flat: true
                icon.source: "qrc:/icon/mcp.svg"
                icon.width: 16; icon.height: 16
                Layout.preferredWidth: 24; Layout.preferredHeight: 24

                onClicked: {
                    const globalPos = mcpButton.mapToGlobal(0, mcpButton.height);
                    const localPos = modeMenu.parent.mapFromGlobal(globalPos.x, globalPos.y);
                    mcpMenu.popup(localPos.x, localPos.y)
                }
            }

            Button {
                id: modeButton
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                enabled: agentModule.state === 0
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
                enabled: agentModule.state === 0
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
                enabled: agentModule.state === 0 && chatColumn.children.length > 0
                flat: true
                icon.source: "qrc:/icon/undo.svg"
                icon.width: 16; icon.height: 16
                Layout.preferredWidth: 24; Layout.preferredHeight: 24

                onClicked: agentModule.conversationUndo()
            }

            Button {
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                enabled: agentModule.state !== 0 || textArea.text.trim().length > 0
                flat: true
                icon.source: agentModule.state !== 0 ? "qrc:/icon/stop.svg" : "qrc:/icon/send.svg"
                icon.width: 16; icon.height: 16
                Layout.preferredWidth: 24; Layout.preferredHeight: 24

                onClicked: agentModule.state !== 0 ? agentModule.state = 2 : agentModule.state = 1
            }

            Button {
                id: micButton
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                enabled: agentModule.state === 0
                checkable: true
                flat: true
                icon.source: "qrc:/icon/mic.svg"
                icon.width: 16; icon.height: 16
                Layout.preferredWidth: 24; Layout.preferredHeight: 24

                onClicked: {
                    if (checked) {
                        agentModule.state = 0
                    }
                }
            }
        }
    }

    Component {
        id: chatComponent

        TextArea {
            id: chatTextArea
            padding: 6
            readOnly: true
            textFormat: TextEdit.MarkdownText
            wrapMode: Text.Wrap
            Layout.preferredWidth: Math.min(chatView.availableWidth, chatMetrics.width + 28)
            Layout.alignment: role === "user" ? Qt.AlignRight : Qt.AlignLeft
            property string messageId
            property string role
            property string buffer

            background: Rectangle {
                color: chatTextArea.role === "user" ? global.brandBack :
                        chatTextArea.role === "assistant" ? global.stroke :
                            chatTextArea.role === "tool" ? global.backSelected : global.dangerBack2
                radius: 6
            }

            onBufferChanged: {
                if(!timer.running) {
                    timer.start()
                }
            }

            Timer {
                id: timer
                interval: 16

                onTriggered: {
                    chatTextArea.text = chatTextArea.buffer
                }
            }

            TextMetrics {
                id: chatMetrics
                text: chatTextArea.text
                font: chatTextArea.font
            }

            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: console.log(chatTextArea.messageId)
            }
        }
    }

    Timer {
        id: scrollTimer
        interval: 50
        onTriggered: {
            scrollAnim.to = 1.0 - chatView.ScrollBar.vertical.size
            scrollAnim.restart()
        }
    }

    NumberAnimation {
        id: scrollAnim
        target: chatView.ScrollBar.vertical
        property: "position"
        duration: 300
        easing.type: Easing.OutQuad
    }

    function chatClear() {
        for (let i = chatColumn.children.length - 1; i >= 0; --i) {
            chatColumn.children[i].destroy();
        }
        rootItem.chatMap = ({})
        rootItem.lastChatTextArea = null
    }

    function chatCreate(messageId, role, text) {
        const obj = chatComponent.createObject(chatColumn, {
            messageId: messageId,
            role: role,
            buffer: text,
        })
        rootItem.chatMap[messageId] = obj
        rootItem.lastChatTextArea = obj
        scrollTimer.restart()
    }

    function chatAppend(messageId, text) {
        if (messageId === "") {
            rootItem.lastChatTextArea.buffer += text
        } else {
            rootItem.chatMap[messageId].buffer += text
        }
        scrollTimer.restart()
    }

    function chatVisible(messageId, status) {
        rootItem.chatMap[messageId].visible = status
    }

    function monitorSet(status, text) {
        chatStatus.status = status
        chatStatus.text = text
    }

    Component.onCompleted: {
        const objects = {
            "topicComboBox": topicComboBox,
            "textArea": textArea,
            "modeButton": modeButton,
            "modelButton": modelButton,
            "micButton": micButton
        };
        agentModule.propertyGet(objects)
    }
}