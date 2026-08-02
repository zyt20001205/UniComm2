import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent
    property var turnMap: ({})
    property var chatMap: ({})

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
                id: conversationComboBox
                enabled: agentModule.state === 0
                model: conversationModel
                textRole: "display"
                valueRole: "id"
                Layout.fillWidth: true; Layout.preferredHeight: 30

                onCurrentValueChanged: agentModule.conversationGet(conversationComboBox.currentValue)
            }

            Button {
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                enabled: agentModule.state === 0
                flat: true
                icon.source: "qrc:/icon/rename.svg"
                icon.width: 16; icon.height: 16
                Layout.preferredWidth: 24; Layout.preferredHeight: 24

                onClicked: {
                    renameDialog.oldTitle = conversationComboBox.currentText
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

                onClicked: agentModule.conversationInsert()
            }

            Button {
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                checkable: true
                enabled: conversationComboBox.currentIndex >= 0 && agentModule.state === 0
                flat: true
                icon.source: checked ? "qrc:/icon/checkmark.svg" : "qrc:/icon/delete.svg"
                icon.width: 16; icon.height: 16
                Layout.preferredWidth: 24; Layout.preferredHeight: 24

                onToggled: {
                    if (!checked) {
                        agentModule.conversationDelete()
                    }
                }

                Timer {
                    interval: 1000
                    running: parent.checked
                    onTriggered: parent.checked = false
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true
            spacing: 4

            Tumbler {
                id: turnTumbler
                visible: count > 0
                model: chatColumn.children.length
                visibleItemCount: Math.max(1, Math.floor(availableHeight / 12))
                property int hoveredIndex: -1
                wrap: false
                padding: 0
                background: null
                Layout.preferredWidth: 24; Layout.fillHeight: true

                onCountChanged: {
                    if (count > 0) currentIndex = count - 1
                }

                onMovingChanged: {
                    if (moving) return
                    const turn = chatColumn.children[currentIndex]
                    scrollAnim.to = Math.min(turn.y / chatColumn.height, 1 - chatScrollBar.size)
                    scrollAnim.restart()
                }

                delegate: Item {
                    id: turnDelegate
                    required property int index
                    implicitWidth: turnTumbler.width
                    implicitHeight: 12
                    readonly property int hoverDistance: turnTumbler.hoveredIndex < 0 ? 4 : Math.min(4, Math.abs(index - turnTumbler.hoveredIndex))

                    Rectangle {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        width: 24 - turnDelegate.hoverDistance * 4
                        height: 2
                        radius: 1
                        color: turnTumbler.hoveredIndex < 0
                               ? turnDelegate.index === turnTumbler.currentIndex ? global.fore : global.stroke
                               : hoverHandler.hovered ? global.fore : global.stroke
                        opacity: turnTumbler.hoveredIndex < 0 ? 1 : 1 - turnDelegate.hoverDistance * 0.15

                        Behavior on width {
                            NumberAnimation { duration: 150 }
                        }
                    }

                    HoverHandler {
                        id: hoverHandler
                        onHoveredChanged: {
                            if (hovered) turnTumbler.hoveredIndex = turnDelegate.index
                            else if (turnTumbler.hoveredIndex === turnDelegate.index) turnTumbler.hoveredIndex = -1
                        }
                    }
                }
            }

            ScrollView {
                id: chatView
                Layout.fillWidth: true; Layout.fillHeight: true
                rightPadding: 14

                ScrollBar.vertical: ScrollBar {
                    id: chatScrollBar
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
        }

        Item {
            property string text
            Layout.fillWidth: true; Layout.preferredHeight: 32

            Rectangle {
                id: chatStatusRect
                anchors.fill: parent
                color: "transparent"
                border.color: {
                    switch (agentModule.state) {
                        case 0: // Ready
                            return global.successBack3
                        case 1: // Error
                            return global.dangerBack3
                        case 2: // Listen
                        case 3: // STT
                        case 4: // Request
                            return global.brandBack
                        case 5: // Abort
                            return global.dangerBack3
                        case 6: // Think
                        case 7: // Response
                        case 8: // Toolcall
                            return global.stroke
                        case 9: // Permission
                            return global.warningBack3
                        case 10: // Speak
                            return global.stroke
                        default:
                            return global.stroke
                    }
                }
                border.width: 1
                radius: 6

                SequentialAnimation on border.color {
                    running: agentModule.state === 9
                    loops: Animation.Infinite
                    ColorAnimation {
                        to: global.back
                        duration: 1000
                    }
                    ColorAnimation {
                        to: global.warningBack3
                        duration: 1000
                    }
                }
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 6; anchors.rightMargin: 6

                Label {
                    id: messageLabel
                    property string message
                    text: {
                        switch (agentModule.state) {
                            case 0: // Ready
                                return qsTr("Ready")
                            case 1: // Error
                                return message
                            case 2: // Listen
                                return qsTr("Listening")
                            case 3: // STT
                                return qsTr("Processing")
                            case 4: // Request
                                return qsTr("Requesting")
                            case 5: // Abort
                                return qsTr("Aborting")
                            case 6: // Think
                                return qsTr("Thinking")
                            case 7: // Response
                                return qsTr("Responding")
                            case 8: // Toolcall
                                return qsTr("Calling Tool")
                            case 9: // Permission
                                return message
                            case 10: // Speak
                                return qsTr("Speaking")
                            default:
                                return ""
                        }
                    }
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                Button {
                    visible: agentModule.state === 9
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    flat: true
                    icon.source: "qrc:/icon/checkmark.svg"
                    icon.width: 16; icon.height: 16
                    Layout.preferredWidth: 24; Layout.preferredHeight: 24

                    onClicked: agentModule.permissionSet(true)
                }

                Button {
                    visible: agentModule.state === 9
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    flat: true
                    icon.source: "qrc:/icon/dismiss.svg"
                    icon.width: 16; icon.height: 16
                    Layout.preferredWidth: 24; Layout.preferredHeight: 24

                    onClicked: agentModule.permissionSet(false)
                }

                BusyIndicator {
                    visible: [6, 7, 8, 10].includes(agentModule.state)
                    running: visible
                    Layout.preferredWidth: 16; Layout.preferredHeight: 16
                }

                IconImage {
                    visible: agentModule.state === 0
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
                            agentModule.state = 4
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

                onClicked: agentModule.conversationRollback()
            }

            Button {
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                enabled: agentModule.state !== 0 || textArea.text.trim().length > 0
                flat: true
                icon.source: agentModule.state === 0 ? "qrc:/icon/send.svg" : "qrc:/icon/stop.svg"
                icon.width: 16; icon.height: 16
                Layout.preferredWidth: 24; Layout.preferredHeight: 24

                onClicked: agentModule.state === 0 ? agentModule.state = 4 : agentModule.state = 5
            }

            Button {
                id: micButton
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
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
        id: turnComponent

        ColumnLayout {
            id: turnItem
            Layout.fillWidth: true
            Layout.preferredWidth: chatColumn.width
            spacing: 6
            property string turnId
            property double startedAt
            property double finishedAt: 0
            property int elapsedSeconds: 0
            property alias messages: messageColumn
            readonly property bool running: finishedAt === 0

            function elapsedUpdate() {
                const end = finishedAt === 0 ? Date.now() : finishedAt
                elapsedSeconds = Math.max(0, Math.floor((end - startedAt) / 1000))
            }

            function durationText() {
                if (elapsedSeconds < 60) return elapsedSeconds + "s"
                return Math.floor(elapsedSeconds / 60) + "m " + elapsedSeconds % 60 + "s"
            }

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: (turnItem.running ? "Working for " : "Worked for ") + turnItem.durationText()
                }

                Item {
                    Layout.fillWidth: true
                }
            }

            Rectangle {
                color: global.stroke
                Layout.fillWidth: true; Layout.preferredHeight: 1
            }

            ColumnLayout {
                id: messageColumn
                Layout.fillWidth: true; Layout.preferredWidth: chatColumn.width
            }

            Timer {
                interval: 1000
                repeat: true
                running: turnItem.running
                triggeredOnStart: true
                onTriggered: turnItem.elapsedUpdate()
            }

            onFinishedAtChanged: elapsedUpdate()
            Component.onCompleted: elapsedUpdate()
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
            ContextMenu.menu: null
            visible: buffer.length > 0
            Layout.preferredWidth: Math.min(chatView.availableWidth, chatMetrics.width + 28)
            Layout.alignment: role === "user" ? Qt.AlignRight : Qt.AlignLeft
            property string messageId
            property string role
            property string reasoningBuffer
            property string contentBuffer
            readonly property string buffer: contentBuffer.length > 0 ? contentBuffer : reasoningBuffer
            background: Rectangle {
                color: chatTextArea.role === "user" ? global.brandBack :
                        chatTextArea.role === "assistant" ? global.stroke :
                            chatTextArea.role === "tool" ? global.backSelected : global.dangerBack2
                radius: 6
            }

            onBufferChanged: {
                if (!timer.running) {
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

    function turnCreate(turnId, startedAt) {
        const obj = turnComponent.createObject(chatColumn, {
            turnId: turnId,
            startedAt: startedAt,
        })
        rootItem.turnMap[turnId] = obj
        scrollTimer.restart()
    }

    function turnFinish(turnId, finishedAt) {
        rootItem.turnMap[turnId].finishedAt = finishedAt
    }

    function chatClear() {
        for (let i = chatColumn.children.length - 1; i >= 0; --i) {
            chatColumn.children[i].destroy();
        }
        rootItem.turnMap = ({})
        rootItem.chatMap = ({})
    }

    function chatCreate(turnId, messageId, role) {
        const obj = chatComponent.createObject(rootItem.turnMap[turnId].messages, {
            messageId: messageId,
            role: role,
        })
        rootItem.chatMap[messageId] = obj
        scrollTimer.restart()
    }

    function chatAppend(messageId, text) {
        rootItem.chatMap[messageId].contentBuffer += text
        scrollTimer.restart()
    }

    function chatReasoningAppend(messageId, text) {
        rootItem.chatMap[messageId].reasoningBuffer += text
        scrollTimer.restart()
    }

    function chatFinish(messageId) {
        rootItem.chatMap[messageId].reasoningBuffer = ""
        scrollTimer.restart()
    }

    Component.onCompleted: {
        const objects = {
            "conversationComboBox": conversationComboBox,
            "textArea": textArea,
            "messageLabel": messageLabel,
            "modeButton": modeButton,
            "modelButton": modelButton,
            "micButton": micButton
        };
        agentModule.propertyGet(objects)
    }
}
