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
            Label {
                text: turnToolTip.prompt
                elide: Text.ElideRight
                color: global.fore
                font.bold: true
                Layout.fillWidth: true
            }

            TextArea {
                visible: text.length > 0
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                readOnly: true
                text: turnToolTip.response
                textFormat: TextEdit.MarkdownText
                wrapMode: TextEdit.Wrap
                clip: true
                color: global.stroke
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
            id: planCard
            visible: steps.length > 0
            Layout.fillWidth: true
            Layout.preferredHeight: visible ? planLayout.implicitHeight + 20 : 0
            property string explanation
            property var steps: []
            readonly property int completedCount: {
                let count = 0
                for (let i = 0; i < steps.length; ++i) {
                    if (steps[i].status === "completed") ++count
                }
                return count
            }

            Rectangle {
                anchors.fill: parent
                color: global.backSelected
                border.color: global.stroke
                border.width: 1
                radius: 6
            }

            ColumnLayout {
                id: planLayout
                anchors.fill: parent
                anchors.margins: 10
                spacing: 4

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    IconImage {
                        color: global.fore
                        source: "qrc:/icon/taskList.svg"
                        sourceSize.width: 16; sourceSize.height: 16
                        Layout.preferredWidth: 16; Layout.preferredHeight: 16
                    }

                    Label {
                        text: qsTr("Plan")
                        font.bold: true
                        Layout.fillWidth: true
                    }

                    Label {
                        text: planCard.completedCount + " / " + planCard.steps.length
                        color: global.fore
                    }
                }

                RowLayout {
                    visible: planCard.explanation.length > 0
                    Layout.fillWidth: true
                    spacing: 8

                    Item {
                        Layout.preferredWidth: 16
                    }

                    Label {
                        text: planCard.explanation
                        color: global.fore
                        opacity: 0.72
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    Repeater {
                        model: planCard.steps

                        delegate: RowLayout {
                            id: stepDelegate
                            required property var modelData
                            readonly property string stepStatus: modelData.status
                            Layout.fillWidth: true
                            spacing: 8

                            Item {
                                Layout.preferredWidth: 16
                            }

                            Item {
                                Layout.preferredWidth: 16; Layout.preferredHeight: 16

                                IconImage {
                                    anchors.fill: parent
                                    color: stepDelegate.stepStatus === "in_progress" ? global.fore : global.stroke
                                    source: stepDelegate.stepStatus === "completed" ? "qrc:/icon/taskCompleted.svg" :
                                            stepDelegate.stepStatus === "in_progress" ? "qrc:/icon/taskInProgress.svg" :
                                            "qrc:/icon/taskPending.svg"
                                    sourceSize.width: 16; sourceSize.height: 16
                                }
                            }

                            Label {
                                text: stepDelegate.modelData.step
                                color: stepDelegate.stepStatus === "in_progress" ? global.fore : global.stroke
                                font.bold: stepDelegate.stepStatus === "in_progress"
                                wrapMode: Text.Wrap
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }
        }

        Item {
            id: permissionCard
            visible: agentModule.state === 11
            Layout.fillWidth: true
            Layout.preferredHeight: visible ? permissionLayout.implicitHeight + 20 : 0

            Rectangle {
                anchors.fill: parent
                color: global.backSelected
                border.color: global.stroke
                border.width: 1
                radius: 6
            }

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border.color: global.fore
                border.width: 1
                radius: 6
                opacity: 0.2

                SequentialAnimation on opacity {
                    running: permissionCard.visible
                    loops: Animation.Infinite

                    NumberAnimation {
                        from: 0.2; to: 0.8
                        duration: 450
                        easing.type: Easing.InOutSine
                    }

                    NumberAnimation {
                        from: 0.8; to: 0.2
                        duration: 850
                        easing.type: Easing.InOutSine
                    }

                    PauseAnimation {
                        duration: 250
                    }
                }
            }

            ColumnLayout {
                id: permissionLayout
                anchors.fill: parent
                anchors.margins: 10
                spacing: 4

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    IconImage {
                        color: global.fore
                        source: "qrc:/icon/shield.svg"
                        sourceSize.width: 16; sourceSize.height: 16
                        Layout.preferredWidth: 16; Layout.preferredHeight: 16
                    }

                    Label {
                        text: qsTr("Allow this action?")
                        font.bold: true
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    Button {
                        id: denyButton
                        text: qsTr("Deny")
                        leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                        Layout.preferredWidth: 80; Layout.preferredHeight: 28

                        onClicked: agentModule.permissionSet(false)
                    }

                    Button {
                        id: allowButton
                        text: qsTr("Allow")
                        highlighted: true
                        leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                        Layout.preferredWidth: 80; Layout.preferredHeight: 28

                        onClicked: agentModule.permissionSet(true)
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Item {
                        Layout.preferredWidth: 16
                    }

                    Label {
                        id: messageLabel
                        property string message
                        text: message
                        color: global.fore
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }
                }
            }
        }

        Item {
            id: userInputCard
            property var request: ({"question": "", "options": []})
            visible: agentModule.state === 12
            Layout.fillWidth: true
            Layout.preferredHeight: visible ? userInputLayout.implicitHeight + 20 : 0

            function submit() {
                const answer = answerTextField.text.trim()
                if (answer.length === 0) return
                agentModule.userInputSet(answer)
                answerTextField.clear()
            }

            onVisibleChanged: {
                if (!visible) return
                answerTextField.clear()
                answerTextField.forceActiveFocus()
            }

            Rectangle {
                anchors.fill: parent
                color: global.backSelected
                border.color: global.stroke
                border.width: 1
                radius: 6
            }

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border.color: global.fore
                border.width: 1
                radius: 6
                opacity: 0.2

                SequentialAnimation on opacity {
                    running: userInputCard.visible
                    loops: Animation.Infinite

                    NumberAnimation {
                        from: 0.2; to: 0.8
                        duration: 450
                        easing.type: Easing.InOutSine
                    }

                    NumberAnimation {
                        from: 0.8; to: 0.2
                        duration: 850
                        easing.type: Easing.InOutSine
                    }

                    PauseAnimation {
                        duration: 250
                    }
                }
            }

            ColumnLayout {
                id: userInputLayout
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    IconImage {
                        color: global.fore
                        source: "qrc:/icon/chat.svg"
                        sourceSize.width: 16; sourceSize.height: 16
                        Layout.preferredWidth: 16; Layout.preferredHeight: 16
                    }

                    Label {
                        text: qsTr("Input required")
                        font.bold: true
                        Layout.fillWidth: true
                    }
                }

                Label {
                    text: userInputCard.request.question || ""
                    color: global.fore
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    Layout.leftMargin: 24
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 24
                    spacing: 8

                    Repeater {
                        model: userInputCard.request.options || []

                        delegate: Button {
                            id: optionButton
                            required property var modelData
                            text: modelData.description
                                  ? modelData.label + " — " + modelData.description
                                  : modelData.label
                            Layout.fillWidth: true
                            Layout.preferredHeight: 28

                            contentItem: Label {
                                text: optionButton.text
                                color: optionButton.palette.buttonText
                                font: optionButton.font
                                elide: Text.ElideRight
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                answerTextField.text = modelData.label
                                answerTextField.forceActiveFocus()
                                answerTextField.selectAll()
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 24
                    spacing: 8

                    TextField {
                        id: answerTextField
                        placeholderText: qsTr("Enter your answer")
                        selectByMouse: true
                        Layout.fillWidth: true
                        Layout.preferredHeight: 28

                        onAccepted: userInputCard.submit()
                    }

                    Button {
                        text: qsTr("Don't ask")
                        leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                        Layout.preferredWidth: 80; Layout.preferredHeight: 28

                        onClicked: agentModule.userInputDisable()
                    }

                    Button {
                        text: qsTr("Submit")
                        highlighted: true
                        enabled: answerTextField.text.trim().length > 0
                        leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                        Layout.preferredWidth: 80; Layout.preferredHeight: 28

                        onClicked: userInputCard.submit()
                    }
                }
            }
        }

        Item {
            clip: true
            Layout.fillWidth: true; Layout.preferredHeight: 118

            Rectangle {
                anchors.fill: parent
                color: global.backSelected
                border.color: global.stroke
                border.width: 1
                radius: 6
            }

            ScrollView {
                anchors.fill: parent
                bottomPadding: 42

                ScrollBar.vertical: ScrollBar {
                    x: parent.mirrored ? 0 : parent.width - width
                    y: parent.topPadding
                    height: parent.availableHeight
                    policy: ScrollBar.AsNeeded
                    palette {
                        mid: global.stroke
                        dark: global.strokePressed
                    }
                }

                TextArea {
                    id: textArea
                    padding: 10
                    topPadding: 12
                    placeholderText: qsTr("Ask a question or describe a task")
                    textFormat: TextEdit.PlainText
                    verticalAlignment: TextEdit.AlignTop
                    wrapMode: TextEdit.Wrap
                    background: null
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
                anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
                anchors.leftMargin: 7; anchors.rightMargin: 7; anchors.bottomMargin: 7
                spacing: 4

                Button {
                    id: modeButton
                    property int mode: 0
                    leftPadding: 7; rightPadding: 7; topPadding: 0; bottomPadding: 0
                    enabled: agentModule.state === 0 && mode >= 0
                    Layout.preferredWidth: modeButtonContent.implicitWidth + leftPadding + rightPadding
                    Layout.preferredHeight: 28

                    contentItem: RowLayout {
                        id: modeButtonContent
                        spacing: 5

                        IconImage {
                            color: modeButton.mode === 3 ? global.warningFore3 : global.fore
                            source: modeButton.mode === 0 ? "qrc:/icon/chat.svg" :
                                    modeButton.mode === 1 ? "qrc:/icon/eye.svg" :
                                    modeButton.mode === 2 ? "qrc:/icon/edit.svg" :
                                    modeButton.mode === 3 ? "qrc:/icon/lockOpen.svg" : ""
                            sourceSize.width: 16; sourceSize.height: 16
                            Layout.preferredWidth: 16; Layout.preferredHeight: 16
                        }

                        Label {
                            text: modeButton.mode === 0 ? qsTr("Chat") :
                                  modeButton.mode === 1 ? qsTr("Read") :
                                  modeButton.mode === 2 ? qsTr("Write") :
                                  modeButton.mode === 3 ? qsTr("Full access") : ""
                            color: modeButton.mode === 3 ? global.warningFore3 : global.fore
                        }

                    }

                    background: Rectangle {
                        color: modeButton.down ? global.backPressed : modeButton.hovered ? global.backHover : "transparent"
                        radius: 6
                    }

                    onClicked: {
                        const globalPos = modeButton.mapToGlobal(0, -modeMenu.implicitHeight - 4);
                        const localPos = modeMenu.parent.mapFromGlobal(globalPos.x, globalPos.y);
                        modeMenu.popup(localPos.x, localPos.y)
                    }
                }

                Button {
                    id: modelButton
                    leftPadding: 7; rightPadding: 7; topPadding: 0; bottomPadding: 0
                    enabled: agentModule.state === 0
                    Layout.preferredWidth: modelButtonContent.implicitWidth + leftPadding + rightPadding
                    Layout.preferredHeight: 28

                    contentItem: RowLayout {
                        id: modelButtonContent
                        spacing: 5

                        Label {
                            text: modelButton.text.length > 0 ? modelButton.text : qsTr("Select model")
                            color: global.fore
                        }
                    }

                    background: Rectangle {
                        color: modelButton.down ? global.backPressed : modelButton.hovered ? global.backHover : "transparent"
                        radius: 6
                    }

                    onClicked: {
                        const globalPos = modelButton.mapToGlobal(0, -modelMenu.implicitHeight - 4);
                        const localPos = modelMenu.parent.mapFromGlobal(globalPos.x, globalPos.y);
                        modelMenu.popup(localPos.x, localPos.y)
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                RowLayout {
                    id: usageLayout
                    property double currentUsage: 0
                    property double contextWindow: 0
                    property double promptTokens: 0
                    property double completionTokens: 0
                    property double cacheHitTokens: 0
                    property double reasoningTokens: 0
                    visible: contextWindow > 0
                    spacing: 0
                    Layout.leftMargin: 4; Layout.rightMargin: 4
                    Layout.preferredHeight: 28

                    function formatTokens(tokens) {
                        if (tokens >= 1000000) return (tokens / 1000000).toFixed(tokens % 1000000 === 0 ? 0 : 1) + "M"
                        if (tokens >= 1000) return (tokens / 1000).toFixed(tokens % 1000 === 0 ? 0 : 1) + "k"
                        return tokens.toString()
                    }

                    Flipable {
                        id: contextLabel
                        property string frontText
                        property string backText
                        Layout.preferredWidth: Math.max(contextFrontLabel.implicitWidth, contextBackLabel.implicitWidth)
                        Layout.preferredHeight: 28

                        function updateText(text) {
                            if (contextFlipAnimation.running) contextFlipAnimation.complete()
                            if (text.length === 0) {
                                contextRotation.angle = 0
                                frontText = ""
                                backText = ""
                            } else if (frontText.length === 0) {
                                frontText = text
                                backText = text
                            } else if (frontText !== text) {
                                backText = text
                                contextFlipAnimation.restart()
                            }
                        }

                        front: Label {
                            id: contextFrontLabel
                            anchors.fill: parent
                            text: contextLabel.frontText
                            color: global.stroke
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        back: Label {
                            id: contextBackLabel
                            anchors.fill: parent
                            text: contextLabel.backText
                            color: global.stroke
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter

                            transform: Rotation {
                                origin.x: contextBackLabel.width / 2
                                origin.y: contextBackLabel.height / 2
                                axis.x: 1; axis.y: 0; axis.z: 0
                                angle: 180
                            }
                        }

                        transform: Rotation {
                            id: contextRotation
                            origin.x: contextLabel.width / 2
                            origin.y: contextLabel.height / 2
                            axis.x: 1; axis.y: 0; axis.z: 0
                            angle: 0
                        }

                        NumberAnimation {
                            id: contextFlipAnimation
                            target: contextRotation
                            property: "angle"
                            from: 0; to: -180
                            duration: 200
                            easing.type: Easing.InOutCubic

                            onFinished: {
                                contextLabel.frontText = contextLabel.backText
                                contextRotation.angle = 0
                            }
                        }
                    }

                    Label {
                        text: " / " + usageLayout.formatTokens(usageLayout.contextWindow)
                        color: global.stroke
                        verticalAlignment: Text.AlignVCenter
                        Layout.preferredHeight: 28
                    }

                    HoverHandler {
                        onHoveredChanged: {
                            if (!hovered) mainToolTip.text = ""
                        }
                        onPointChanged: {
                            mainToolTip.position = parent.mapToGlobal(point.position)
                            mainToolTip.text = qsTr("Context: %1 / %2\nTurn input: %3\nCached: %4\nTurn output: %5\nReasoning: %6")
                                                   .arg(usageLayout.formatTokens(usageLayout.currentUsage))
                                                   .arg(usageLayout.formatTokens(usageLayout.contextWindow))
                                                   .arg(usageLayout.promptTokens)
                                                   .arg(usageLayout.cacheHitTokens)
                                                   .arg(usageLayout.completionTokens)
                                                   .arg(usageLayout.reasoningTokens)
                        }
                    }
                }

                Button {
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    enabled: agentModule.state === 0 && chatColumn.children.length > 0
                    flat: true
                    icon.source: "qrc:/icon/undo.svg"
                    icon.width: 16; icon.height: 16
                    Layout.preferredWidth: 28; Layout.preferredHeight: 28

                    onClicked: agentModule.conversationRollback()
                }

                Button {
                    id: micButton
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    checkable: true
                    flat: true
                    icon.source: "qrc:/icon/mic.svg"
                    icon.width: 16; icon.height: 16
                    Layout.preferredWidth: 28; Layout.preferredHeight: 28

                    onClicked: {
                        if (checked) {
                            agentModule.state = 0
                        }
                    }
                }

                Button {
                    id: sendButton
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    enabled: agentModule.state !== 0 || textArea.text.trim().length > 0
                    icon.source: agentModule.state === 0 ? "qrc:/icon/send.svg" : "qrc:/icon/stop.svg"
                    icon.width: 16; icon.height: 16
                    Layout.preferredWidth: 28; Layout.preferredHeight: 28


                    onClicked: agentModule.state === 0 ? agentModule.state = 4 : agentModule.state = 7
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
            padding: role === "assistant" ? 0 : 6
            leftPadding: padding; rightPadding: padding; topPadding: padding; bottomPadding: padding
            readOnly: true
            textFormat: TextEdit.MarkdownText
            wrapMode: Text.Wrap
            ContextMenu.menu: null
            visible: buffer.length > 0 && (!turn.collapsed || role === "user" || messageId === turn.lastId)
            Layout.preferredWidth: role === "assistant" ? chatView.availableWidth : Math.min(chatView.availableWidth * 0.8, implicitWidth)
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
        planCard.explanation = ""
        planCard.steps = []
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
        planCard.explanation = ""
        planCard.steps = []
        usageUpdate({})
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

    function planUpdate(plan) {
        planCard.explanation = plan.explanation ? plan.explanation : ""
        planCard.steps = plan.plan ? plan.plan : []
    }

    function usageUpdate(usage) {
        const currentUsage = usage.currentUsage || 0
        const promptTokens = usage.promptTokens || 0
        const cacheHitTokens = usage.cacheHitTokens || 0
        contextLabel.updateText(usageLayout.formatTokens(currentUsage))
        usageLayout.currentUsage = currentUsage
        usageLayout.promptTokens = promptTokens
        usageLayout.completionTokens = usage.completionTokens || 0
        usageLayout.cacheHitTokens = cacheHitTokens
        usageLayout.reasoningTokens = usage.reasoningTokens || 0
    }

    function modelUpdate(contextWindow) {
        usageLayout.contextWindow = contextWindow || 0
    }

    Component.onCompleted: {
        const objects = {
            "conversationComboBox": conversationComboBox,
            "textArea": textArea,
            "messageLabel": messageLabel,
            "userInputCard": userInputCard,
            "modeButton": modeButton,
            "modelButton": modelButton,
            "micButton": micButton
        };
        agentModule.propertyGet(objects)
    }
}
