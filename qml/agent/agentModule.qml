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

    ToolTip {
        id: turnToolTip
        parent: Overlay.overlay
        closePolicy: Popup.NoAutoClose
        visible: prompt.length > 0
        width: Math.min(implicitWidth, 320)
        height: implicitHeight
        property string prompt
        property string response

        FontMetrics {
            id: turnToolTipFontMetrics
            font: turnToolTip.font
        }

        contentItem: ColumnLayout {
            spacing: 0

            Label {
                text: turnToolTip.prompt
                elide: Text.ElideRight
                color: global.fore
                font: turnToolTip.font
                Layout.fillWidth: true
            }

            TextArea {
                visible: text.length > 0
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                textMargin: 0
                readOnly: true
                text: turnToolTip.response
                textFormat: TextEdit.MarkdownText
                wrapMode: TextEdit.Wrap
                clip: true
                color: global.stroke
                font: turnToolTip.font
                background: null
                ContextMenu.menu: null
                Layout.fillWidth: true
                Layout.maximumHeight: turnToolTipFontMetrics.lineSpacing * 4
            }
        }

        Behavior on width {
            enabled: turnToolTip.visible
            NumberAnimation {
                duration: 80
                easing.type: Easing.OutCubic
            }
        }

        Behavior on height {
            enabled: turnToolTip.visible
            NumberAnimation {
                duration: 80
                easing.type: Easing.OutCubic
            }
        }

        Behavior on x {
            enabled: turnToolTip.visible
            NumberAnimation {
                duration: 80
                easing.type: Easing.OutCubic
            }
        }

        Behavior on y {
            enabled: turnToolTip.visible
            NumberAnimation {
                duration: 80
                easing.type: Easing.OutCubic
            }
        }
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

            Button {
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                flat: true
                icon.source: "qrc:/icon/settings.svg"
                icon.width: 16; icon.height: 16
                Layout.preferredWidth: 24; Layout.preferredHeight: 24

                onClicked: agentModule.agentManage()
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
                readonly property real viewportPosition: Math.max(0, Math.min(chatScrollBar.position, 1 - chatScrollBar.size))
                readonly property real viewportTop: viewportPosition * chatColumn.height
                readonly property real viewportBottom: viewportTop + chatView.availableHeight
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
                    rootItem.navigateTo(turn.y)
                }

                delegate: Item {
                    id: turnDelegate
                    required property int index
                    implicitWidth: turnTumbler.width
                    implicitHeight: 12
                    readonly property int hoverDistance: turnTumbler.hoveredIndex < 0 ? 4 : Math.min(4, Math.abs(index - turnTumbler.hoveredIndex))
                    readonly property var turn: chatColumn.children[index]
                    readonly property bool turnVisible: turn.y < turnTumbler.viewportBottom && turn.y + turn.height > turnTumbler.viewportTop

                    Rectangle {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        width: 24 - turnDelegate.hoverDistance * 4
                        height: 2
                        radius: 1
                        color: turnTumbler.hoveredIndex < 0
                            ? turnDelegate.turnVisible ? global.fore : global.stroke
                            : hoverHandler.hovered ? global.fore : global.stroke
                        opacity: turnTumbler.hoveredIndex < 0 ? 1 : 1 - turnDelegate.hoverDistance * 0.15

                        Behavior on width {
                            NumberAnimation {
                                duration: 150
                            }
                        }
                    }

                    HoverHandler {
                        id: hoverHandler
                        onHoveredChanged: {
                            if (hovered) {
                                turnTumbler.hoveredIndex = turnDelegate.index
                                const position = turnToolTip.parent.mapFromItem(parent, parent.width, parent.height / 2)
                                turnToolTip.prompt = turnDelegate.turn.prompt
                                turnToolTip.response = turnDelegate.turn.response
                                turnToolTip.x = position.x + 10
                                turnToolTip.y = position.y - turnToolTip.implicitHeight / 2
                            } else if (turnTumbler.hoveredIndex === turnDelegate.index) {
                                turnTumbler.hoveredIndex = -1
                                turnToolTip.prompt = ""
                                turnToolTip.response = ""
                            }
                        }
                    }

                    TapHandler {
                        onTapped: {
                            turnTumbler.currentIndex = turnDelegate.index
                            turnTumbler.positionViewAtIndex(turnDelegate.index, Tumbler.Center)
                            rootItem.navigateTo(turnDelegate.turn.y)
                        }
                    }
                }
            }

            ScrollView {
                id: chatView
                rightPadding: 14
                Layout.fillWidth: true; Layout.fillHeight: true
                property bool followTail: true

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

                    onPositionChanged: {
                        if (followAnimation.running || navigationAnimation.running) return
                        const viewportPosition = turnTumbler.viewportPosition
                        let index = viewportPosition === 0 ? 0 : turnTumbler.count - 1
                        if (viewportPosition > 0 && viewportPosition < 1 - size) {
                            const viewportY = viewportPosition * chatColumn.height + chatView.availableHeight / 2
                            for (let i = 0; i < chatColumn.children.length; ++i) {
                                const turn = chatColumn.children[i]
                                if (viewportY < turn.y + turn.height) {
                                    index = i
                                    break
                                }
                            }
                        }
                        if (index === turnTumbler.currentIndex) return
                        turnTumbler.currentIndex = index
                        turnTumbler.positionViewAtIndex(index, Tumbler.Center)
                    }

                    onPressedChanged: {
                        if (pressed) {
                            chatView.followTail = false
                            rootItem.scrollStop()
                            return
                        }
                        chatView.followTail = chatView.contentItem.atYEnd
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
                        case 8: // ToolCall
                            return global.stroke
                        case 9: // Permission
                            return global.warningBack3
                        case 10: // ToolExec
                        case 11: // Speak
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
                            case 8: // ToolCall
                                return qsTr("Calling Tool")
                            case 9: // Permission
                                return message
                            case 10: // ToolExec
                                return qsTr("Executing Tool")
                            case 11: // Speak
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
                    visible: [6, 7, 8, 10, 11].includes(agentModule.state)
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
            property bool collapsed: false
            property string prompt
            property string response
            property string lastId
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
                    verticalAlignment: Text.AlignVCenter
                    text: (turnItem.running ? "Working for " : "Worked for ") + turnItem.durationText()
                    Layout.preferredHeight: 24
                }

                Button {
                    visible: !turnItem.running
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    flat: true
                    icon.source: turnItem.collapsed ? "qrc:/icon/arrowCollapse.svg" : "qrc:/icon/arrowExpand.svg"
                    icon.width: 16; icon.height: 16
                    Layout.preferredWidth: 24; Layout.preferredHeight: 24

                    onClicked: turnItem.collapsed = !turnItem.collapsed
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
            padding: role === "assistant" ? 0 : 8
            readOnly: true
            textFormat: TextEdit.MarkdownText
            wrapMode: Text.Wrap
            ContextMenu.menu: null
            visible: buffer.length > 0 && (!turn.collapsed || role === "user" || messageId === turn.lastId)
            Layout.preferredWidth: role === "assistant" ? chatView.availableWidth : Math.min(chatView.availableWidth * 0.8, chatMetrics.width + 32)
            Layout.alignment: role === "user" ? Qt.AlignRight : Qt.AlignLeft
            property var turn
            property string messageId
            property string role
            property string reasoningBuffer
            property string contentBuffer
            readonly property string buffer: contentBuffer.length > 0 ? contentBuffer : reasoningBuffer
            background: Rectangle {
                color: chatTextArea.role === "user" ? global.backSelected :
                        chatTextArea.role === "assistant" ? "transparent" :
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

    SmoothedAnimation {
        id: followAnimation
        target: chatView.contentItem
        property: "contentY"
        duration: 160
        velocity: -1
        maximumEasingTime: 60
    }

    NumberAnimation {
        id: navigationAnimation
        target: chatView.contentItem
        property: "contentY"
        duration: 220
        easing.type: Easing.OutCubic
    }

    Connections {
        target: chatView.contentItem

        function onMovementStarted() {
            chatView.followTail = false
            rootItem.scrollStop()
        }

        function onMovementEnded() {
            chatView.followTail = chatView.contentItem.atYEnd
        }

        function onContentHeightChanged() {
            rootItem.followToTail()
        }
    }

    function followToTail() {
        if (!chatView.followTail) return
        const flickable = chatView.contentItem
        if (navigationAnimation.running) navigationAnimation.stop()
        const target = Math.max(flickable.originY, flickable.originY + flickable.contentHeight - flickable.height)
        followAnimation.stop()
        followAnimation.from = flickable.contentY
        followAnimation.to = target
        followAnimation.start()
    }

    function navigateTo(position) {
        scrollStop()
        const flickable = chatView.contentItem
        const bottom = Math.max(flickable.originY, flickable.originY + flickable.contentHeight - flickable.height)
        const target = Math.max(flickable.originY, Math.min(position, bottom))
        chatView.followTail = target === bottom
        navigationAnimation.to = target
        navigationAnimation.restart()
    }

    function scrollStop() {
        followAnimation.stop()
        navigationAnimation.stop()
    }

    function turnCreate(turnId, startedAt) {
        const obj = turnComponent.createObject(chatColumn, {
            turnId: turnId,
            startedAt: startedAt,
        })
        rootItem.turnMap[turnId] = obj
    }

    function turnFinish(turnId, finishedAt) {
        const turn = rootItem.turnMap[turnId]
        turn.finishedAt = finishedAt
        turn.collapsed = true
    }

    function chatClear() {
        scrollStop()
        chatView.followTail = true
        for (let i = chatColumn.children.length - 1; i >= 0; --i) {
            chatColumn.children[i].destroy();
        }
        rootItem.turnMap = ({})
        rootItem.chatMap = ({})
    }

    function chatCreate(turnId, messageId, role) {
        const turn = rootItem.turnMap[turnId]
        const obj = chatComponent.createObject(turn.messages, {
            turn: turn,
            messageId: messageId,
            role: role,
        })
        rootItem.chatMap[messageId] = obj
    }

    function chatAppend(messageId, text) {
        const chat = rootItem.chatMap[messageId]
        chat.contentBuffer += text
        if (chat.role === "user") chat.turn.prompt += text
        else if (chat.role === "assistant" && chat.contentBuffer.length > 0) {
            chat.turn.response = chat.contentBuffer
            chat.turn.lastId = messageId
        }
    }

    function chatReasoningAppend(messageId, text) {
        rootItem.chatMap[messageId].reasoningBuffer += text
    }

    function chatFinish(messageId) {
        rootItem.chatMap[messageId].reasoningBuffer = ""
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
