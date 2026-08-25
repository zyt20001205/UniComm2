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
    property var subagentMap: ({})

    ListModel {
        id: permissionModel
    }

    ListModel {
        id: userInputModel
    }

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
            visible: steps.length > 0 && !minimized
            Layout.fillWidth: true
            Layout.preferredHeight: visible ? planLayout.implicitHeight + 20 : 0
            property string explanation
            property var steps: []
            property bool minimized: true
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
            id: compactCard
            property bool completed: false
            visible: agentModule.state === 6 || completed
            Layout.fillWidth: true
            Layout.preferredHeight: visible ? compactLayout.implicitHeight + 20 : 0

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
                visible: !compactCard.completed
                opacity: 0.2

                SequentialAnimation on opacity {
                    running: compactCard.visible && !compactCard.completed
                    loops: Animation.Infinite

                    NumberAnimation {
                        from: 0.2
                        to: 0.8
                        duration: 450
                        easing.type: Easing.InOutSine
                    }

                    NumberAnimation {
                        from: 0.8
                        to: 0.2
                        duration: 450
                        easing.type: Easing.InOutSine
                    }

                    PauseAnimation {
                        duration: 250
                    }
                }
            }

            ColumnLayout {
                id: compactLayout
                anchors.fill: parent
                anchors.margins: 10
                spacing: 4

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    IconImage {
                        color: compactCard.completed ? global.stroke : global.fore
                        source: compactCard.completed ? "qrc:/icon/taskCompleted.svg" : "qrc:/icon/arrowMinimize.svg"
                        sourceSize.width: 16; sourceSize.height: 16
                        Layout.preferredWidth: 16; Layout.preferredHeight: 16
                    }

                    Label {
                        text: compactCard.completed ? qsTr("Context compacted") : qsTr("Compacting context")
                        font.bold: true
                        Layout.fillWidth: true
                    }

                    Label {
                        text: compactCard.completed ? qsTr("Done") :
                                usageLayout.currentUsage > 0 ? usageLayout.formatTokens(usageLayout.currentUsage) + " " + qsTr("tokens") : ""
                        color: global.stroke
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Item {
                        Layout.preferredWidth: 16
                    }

                    Label {
                        text: compactCard.completed ? qsTr("Earlier turns were summarized into a compact context.") :
                            qsTr("Summarizing earlier turns to free context space.")
                        color: global.stroke
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }
                }
            }

            Connections {
                target: agentModule

                function onChangeState(): void {
                    if (agentModule.state !== 6) return
                    compactStatusTimer.stop()
                    compactCard.completed = false
                }
            }

            Timer {
                id: compactStatusTimer
                interval: 5000

                onTriggered: compactCard.completed = false
            }
        }

        Item {
            id: permissionCards
            visible: permissionModel.count > 0
            Layout.fillWidth: true
            Layout.preferredHeight: visible && permissionSwipeView.currentItem
                ? permissionSwipeView.currentItem.cardImplicitHeight
                    + (permissionSwipeView.count > 1 ? permissionIndicator.implicitHeight : 0)
                : 0

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
                    running: permissionCards.visible
                    loops: Animation.Infinite

                    NumberAnimation {
                        from: 0.2
                        to: 0.8
                        duration: 450
                        easing.type: Easing.InOutSine
                    }

                    NumberAnimation {
                        from: 0.8
                        to: 0.2
                        duration: 850
                        easing.type: Easing.InOutSine
                    }

                    PauseAnimation {
                        duration: 250
                    }
                }
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                SwipeView {
                    id: permissionSwipeView
                    clip: true
                    interactive: true
                    orientation: Qt.Horizontal
                    Layout.fillWidth: true
                    Layout.preferredHeight: currentItem ? currentItem.cardImplicitHeight : 0

                    Repeater {
                        model: permissionModel

                        delegate: Item {
                            id: permissionPage
                            required property string runtimeId
                            required property string role
                            required property string message
                            readonly property real cardImplicitHeight: permissionLayout.implicitHeight + 20

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

                                    Label {
                                        text: permissionPage.role === "hardware" ? qsTr("Hardware") :
                                              permissionPage.role === "software" ? qsTr("Software") :
                                              permissionPage.role === "supervisor" ? qsTr("Supervisor") : qsTr("Agent")
                                        color: global.stroke
                                    }

                                    Button {
                                        text: qsTr("Deny")
                                        leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                        Layout.preferredWidth: 80; Layout.preferredHeight: 28

                                        onClicked: {
                                            rootItem.permissionResponse(permissionPage.runtimeId, false)
                                        }
                                    }

                                    Button {
                                        text: qsTr("Allow")
                                        highlighted: true
                                        leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                        Layout.preferredWidth: 80; Layout.preferredHeight: 28

                                        onClicked: {
                                            rootItem.permissionResponse(permissionPage.runtimeId, true)
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Item {
                                        Layout.preferredWidth: 16
                                    }

                                    Label {
                                        text: permissionPage.message
                                        color: global.fore
                                        wrapMode: Text.Wrap
                                        Layout.fillWidth: true
                                    }
                                }
                            }
                        }
                    }
                }

                PageIndicator {
                    id: permissionIndicator
                    visible: permissionSwipeView.count > 1
                    count: permissionSwipeView.count
                    currentIndex: permissionSwipeView.currentIndex
                    Layout.fillWidth: false
                    Layout.preferredHeight: visible ? implicitHeight : 0
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                    background: Rectangle {
                        color: "transparent"
                    }
                }
            }
        }

        Item {
            id: userInputCards
            visible: userInputModel.count > 0
            Layout.fillWidth: true
            Layout.preferredHeight: visible && userInputSwipeView.currentItem
                ? userInputSwipeView.currentItem.cardImplicitHeight
                    + (userInputSwipeView.count > 1 ? userInputIndicator.implicitHeight : 0)
                : 0

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
                    running: userInputCards.visible
                    loops: Animation.Infinite

                    NumberAnimation {
                        from: 0.2
                        to: 0.8
                        duration: 450
                        easing.type: Easing.InOutSine
                    }

                    NumberAnimation {
                        from: 0.8
                        to: 0.2
                        duration: 850
                        easing.type: Easing.InOutSine
                    }

                    PauseAnimation {
                        duration: 250
                    }
                }
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                SwipeView {
                    id: userInputSwipeView
                    clip: true
                    interactive: true
                    orientation: Qt.Horizontal
                    Layout.fillWidth: true
                    Layout.preferredHeight: currentItem ? currentItem.cardImplicitHeight : 0

                    Repeater {
                        model: userInputModel

                        delegate: Item {
                            id: userInputPage
                            required property string runtimeId
                            required property string role
                            required property string question
                            required property var options
                            readonly property real cardImplicitHeight: userInputLayout.implicitHeight + 20

                            function submit(): void {
                                const answer = answerTextField.text.trim()
                                if (answer.length === 0) return
                                rootItem.userInputResponse(userInputPage.runtimeId, answer)
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

                                    Label {
                                        text: userInputPage.role === "hardware" ? qsTr("Hardware") :
                                              userInputPage.role === "software" ? qsTr("Software") :
                                              userInputPage.role === "supervisor" ? qsTr("Supervisor") : qsTr("Agent")
                                        color: global.stroke
                                    }
                                }

                                Label {
                                    text: userInputPage.question
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
                                        model: userInputPage.options

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

                                        onAccepted: userInputPage.submit()
                                    }

                                    Button {
                                        text: qsTr("Don't ask")
                                        leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                        Layout.preferredWidth: 80; Layout.preferredHeight: 28

                                        onClicked: {
                                            rootItem.userInputResponse(userInputPage.runtimeId, "")
                                        }
                                    }

                                    Button {
                                        text: qsTr("Submit")
                                        highlighted: true
                                        enabled: answerTextField.text.trim().length > 0
                                        leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                        Layout.preferredWidth: 80; Layout.preferredHeight: 28

                                        onClicked: userInputPage.submit()
                                    }
                                }
                            }
                        }
                    }
                }

                PageIndicator {
                    id: userInputIndicator
                    visible: userInputSwipeView.count > 1
                    count: userInputSwipeView.count
                    currentIndex: userInputSwipeView.currentIndex
                    Layout.fillWidth: false
                    Layout.preferredHeight: visible ? implicitHeight : 0
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                    background: Rectangle {
                        color: "transparent"
                    }
                }
            }
        }

        Item {
            id: overviewCard
            visible: planCard.steps.length > 0
            Layout.preferredWidth: visible ? overviewLayout.implicitWidth + 12 : 0
            Layout.preferredHeight: visible ? overviewLayout.implicitHeight + 12 : 0
            Layout.alignment: Qt.AlignHCenter

            Rectangle {
                anchors.fill: parent
                color: global.backSelected
                border.color: global.stroke
                border.width: 1
                radius: 6
            }

            RowLayout {
                id: overviewLayout
                anchors.fill: parent
                anchors.margins: 6

                Button {
                    id: planButton
                    leftPadding: 4; rightPadding: 4; topPadding: 0; bottomPadding: 0
                    flat: true
                    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding
                    Layout.preferredHeight: 24

                    contentItem: RowLayout {
                        spacing: 8

                        IconImage {
                            color: global.fore
                            source: "qrc:/icon/taskList.svg"
                            sourceSize.width: 16; sourceSize.height: 16
                            Layout.preferredWidth: 16; Layout.preferredHeight: 16
                        }

                        Label {
                            text: planCard.completedCount + " / " + planCard.steps.length
                            Layout.preferredWidth: implicitWidth
                        }
                    }

                    onClicked: planCard.minimized = !planCard.minimized
                }
            }
        }

        Item {
            clip: true
            Layout.fillWidth: true
            Layout.minimumHeight: 84
            Layout.maximumHeight: 220
            Layout.preferredHeight: Math.max(Layout.minimumHeight, Math.min(Layout.maximumHeight,
                textArea.contentHeight + textArea.topPadding + textArea.bottomPadding + 42))

            Behavior on Layout.preferredHeight {
                NumberAnimation {
                    duration: 120
                    easing.type: Easing.OutCubic
                }
            }

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
                                agentModule.pre()
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
                    id: strategyButton
                    property int strategy: 0
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    checkable: true
                    checked: strategy === 1
                    enabled: agentModule.state === 0
                    flat: true
                    icon.source: checked ? "qrc:/icon/team.svg" : "qrc:/icon/solo.svg"
                    icon.width: 16; icon.height: 16
                    Layout.preferredWidth: 28; Layout.preferredHeight: 28

                    onClicked: agentModule.conversationStrategySet(checked ? 1 : 0)

                    HoverHandler {
                        onHoveredChanged: {
                            if (!hovered) mainToolTip.text = ""
                        }
                        onPointChanged: {
                            mainToolTip.position = parent.mapToGlobal(point.position)
                            mainToolTip.text = strategyButton.strategy === 0
                                ? qsTr("Solo mode\nA general agent completes the task directly.\nClick to switch to Team mode.")
                                : qsTr("Team mode\nA supervisor coordinates specialized agents.\nClick to switch to Solo mode.")
                        }
                    }
                }

                Button {
                    id: modeButton
                    property int mode: 0
                    enabled: agentModule.state === 0 && mode >= 0
                    flat: true
                    text: mode === 0 ? qsTr("Chat") :
                          mode === 1 ? qsTr("Read") :
                          mode === 2 ? qsTr("Write") :
                          mode === 3 ? qsTr("Full access") : ""
                    icon.source: mode === 0 ? "qrc:/icon/chat.svg" :
                                 mode === 1 ? "qrc:/icon/eye.svg" :
                                 mode === 2 ? "qrc:/icon/edit.svg" :
                                 mode === 3 ? "qrc:/icon/lockOpen.svg" : ""
                    icon.color: mode === 3 ? global.warningFore3 : global.fore
                    icon.width: 16; icon.height: 16
                    palette.buttonText: mode === 3 ? global.warningFore3 : global.fore
                    Layout.preferredHeight: 28

                    onClicked: {
                        const globalPos = modeButton.mapToGlobal(0, -modeMenu.implicitHeight - 4);
                        const localPos = modeMenu.parent.mapFromGlobal(globalPos.x, globalPos.y);
                        modeMenu.popup(localPos.x, localPos.y)
                    }
                }

                Button {
                    id: modelButton
                    enabled: agentModule.state === 0
                    flat: true
                    text: qsTr("Select model")
                    Layout.preferredHeight: 28

                    onClicked: {
                        const globalPos = modelButton.mapToGlobal(0, -modelMenu.implicitHeight - 4);
                        const localPos = modelMenu.parent.mapFromGlobal(globalPos.x, globalPos.y);
                        modelMenu.popup(localPos.x, localPos.y)
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                Flipable {
                    id: usageLayout
                    property double currentUsage: 0
                    property double contextWindow: 0
                    property bool compactPending: false
                    visible: contextWindow > 0
                    Layout.leftMargin: 4; Layout.rightMargin: 4
                    Layout.preferredWidth: Math.max(usageContent.implicitWidth, compactButton.implicitWidth)
                    Layout.preferredHeight: 28

                    function formatTokens(tokens: double): string {
                        if (tokens >= 1000000) return (tokens / 1000000).toFixed(tokens % 1000000 === 0 ? 0 : 1) + "M"
                        if (tokens >= 1000) return (tokens / 1000).toFixed(tokens % 1000 === 0 ? 0 : 1) + "k"
                        return tokens.toString()
                    }

                    function exactTokens(tokens: double): string {
                        return tokens > 0 ? Number(tokens).toLocaleString(Qt.locale(), "f", 0) : "-"
                    }

                    front: Item {
                        anchors.fill: parent

                        RowLayout {
                            id: usageContent
                            anchors.centerIn: parent
                            spacing: 4

                            Flipable {
                                id: contextLabel
                                property string frontText
                                property string backText
                                Layout.preferredWidth: Math.max(contextFrontLabel.implicitWidth, contextBackLabel.implicitWidth)
                                Layout.preferredHeight: 28

                                function updateText(text: string): void {
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
                                    horizontalAlignment: Text.AlignRight
                                    verticalAlignment: Text.AlignVCenter
                                }

                                back: Label {
                                    id: contextBackLabel
                                    anchors.fill: parent
                                    text: contextLabel.backText
                                    color: global.stroke
                                    horizontalAlignment: Text.AlignRight
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
                                    from: 0
                                    to: -180
                                    duration: 200
                                    easing.type: Easing.InOutCubic

                                    onFinished: {
                                        contextLabel.frontText = contextLabel.backText
                                        contextRotation.angle = 0
                                    }
                                }
                            }

                            Label {
                                text: "/"
                                color: global.stroke
                                verticalAlignment: Text.AlignVCenter
                                Layout.preferredHeight: 28
                            }

                            Label {
                                text: usageLayout.formatTokens(usageLayout.contextWindow)
                                color: global.stroke
                                verticalAlignment: Text.AlignVCenter
                                Layout.preferredHeight: 28
                            }
                        }
                    }

                    back: RowLayout {
                        id: compactButton
                        anchors.centerIn: parent
                        spacing: 5

                        IconImage {
                            color: global.warningFore3
                            source: "qrc:/icon/arrowMinimize.svg"
                            sourceSize.width: 16; sourceSize.height: 16
                            Layout.preferredWidth: 16; Layout.preferredHeight: 16
                        }

                        Label {
                            id: compactLabel
                            text: qsTr("Compact")
                            color: global.warningFore3
                        }
                    }

                    transform: Rotation {
                        origin.x: usageLayout.width / 2
                        origin.y: usageLayout.height / 2
                        axis.x: 0; axis.y: 1; axis.z: 0
                        angle: usageLayout.compactPending ? -180 : 0

                        Behavior on angle {
                            NumberAnimation {
                                duration: 200
                                easing.type: Easing.InOutCubic
                            }
                        }
                    }

                    TapHandler {
                        enabled: agentModule.state === 0 && usageLayout.currentUsage > 0

                        onTapped: {
                            if (!usageLayout.compactPending) {
                                usageLayout.compactPending = true
                                compactTimer.restart()
                            } else {
                                compactTimer.stop()
                                usageLayout.compactPending = false
                                agentModule.compact()
                            }
                        }
                    }

                    Timer {
                        id: compactTimer
                        interval: 1500

                        onTriggered: usageLayout.compactPending = false
                    }

                    HoverHandler {
                        onHoveredChanged: {
                            if (!hovered) mainToolTip.text = ""
                        }
                        onPointChanged: {
                            mainToolTip.position = parent.mapToGlobal(point.position)
                            const action = usageLayout.currentUsage > 0 ? qsTr("Click to compact context") : qsTr("Context will update after the next response")
                            mainToolTip.text = qsTr("Context: %1 / %2\n%3")
                                .arg(usageLayout.exactTokens(usageLayout.currentUsage))
                                .arg(usageLayout.exactTokens(usageLayout.contextWindow))
                                .arg(action)
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
                    visible: false
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    checkable: true
                    flat: true
                    icon.source: "qrc:/icon/mic.svg"
                    icon.width: 16; icon.height: 16
                    Layout.preferredWidth: 28; Layout.preferredHeight: 28
                }

                Button {
                    id: sendButton
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    enabled: agentModule.state !== 0 || textArea.text.trim().length > 0
                    icon.source: agentModule.state === 0 ? "qrc:/icon/send.svg" : "qrc:/icon/stop.svg"
                    icon.width: 16; icon.height: 16
                    Layout.preferredWidth: 28; Layout.preferredHeight: 28

                    onClicked: agentModule.state === 0 ? agentModule.pre() : agentModule.abort()
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
            property bool collapsed: true
            property string prompt
            property string response
            property string lastId
            property alias subagents: subagentColumn
            property alias messages: messageColumn
            readonly property bool running: finishedAt === 0

            function elapsedUpdate(): void {
                const end = finishedAt === 0 ? Date.now() : finishedAt
                elapsedSeconds = Math.max(0, Math.floor((end - startedAt) / 1000))
            }

            function durationText(): string {
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
                id: subagentColumn
                Layout.fillWidth: true; Layout.preferredWidth: chatColumn.width
                spacing: 6
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
        id: subagentComponent

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            property string runtimeId
            property string role
            property string message

            IconImage {
                color: global.fore
                source: role === "hardware" ? "qrc:/icon/hardware.svg" : "qrc:/icon/software.svg"
                sourceSize.width: 24; sourceSize.height: 24
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                Layout.alignment: Qt.AlignTop
            }

            Label {
                text: message
                color: global.fore
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.NoWrap
                Layout.fillWidth: true; Layout.preferredHeight: 24
            }
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
            readonly property bool reasoning: role === "assistant" && contentBuffer.length === 0 && reasoningBuffer.length > 0
            readonly property string displayBuffer: reasoning && turn.collapsed ? qsTr("Thinking...") : buffer
            background: Rectangle {
                color: chatTextArea.role === "user" ? global.backSelected :
                        chatTextArea.role === "assistant" ? "transparent" :
                            chatTextArea.role === "tool" ? global.backSelected : global.dangerBack2
                radius: 6
            }

            onDisplayBufferChanged: {
                if (!timer.running) {
                    timer.start()
                }
            }

            Timer {
                id: timer
                interval: 16

                onTriggered: {
                    chatTextArea.text = chatTextArea.displayBuffer
                }
            }

            HoverHandler {
                cursorShape: chatTextArea.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton

                onTapped: {
                    if (chatTextArea.hoveredLink) {
                        documentModule.documentOpen(chatTextArea.hoveredLink)
                    }
                }
            }

            TapHandler {
                acceptedButtons: Qt.RightButton

                onTapped: {
                    if (chatTextArea.hoveredLink) {
                        mainLinkMenu.url = chatTextArea.hoveredLink
                        mainLinkMenu.popup()
                    }
                }
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

        function onMovementStarted(): void {
            chatView.followTail = false
            rootItem.scrollStop()
        }

        function onMovementEnded(): void {
            chatView.followTail = chatView.contentItem.atYEnd
        }

        function onContentHeightChanged(): void {
            rootItem.followToTail()
        }
    }

    function followToTail(): void {
        if (!chatView.followTail) return
        const flickable = chatView.contentItem
        if (navigationAnimation.running) navigationAnimation.stop()
        const target = Math.max(flickable.originY, flickable.originY + flickable.contentHeight - flickable.height)
        followAnimation.stop()
        followAnimation.from = flickable.contentY
        followAnimation.to = target
        followAnimation.start()
    }

    function navigateTo(position: double): void {
        scrollStop()
        const flickable = chatView.contentItem
        const bottom = Math.max(flickable.originY, flickable.originY + flickable.contentHeight - flickable.height)
        const target = Math.max(flickable.originY, Math.min(position, bottom))
        chatView.followTail = target === bottom
        navigationAnimation.to = target
        navigationAnimation.restart()
    }

    function scrollStop(): void {
        followAnimation.stop()
        navigationAnimation.stop()
    }

    function turnCreate(turnId: string, startedAt: double): void {
        planCard.explanation = ""
        planCard.steps = []
        planCard.minimized = true
        const obj = turnComponent.createObject(chatColumn, {
            turnId: turnId,
            startedAt: startedAt,
        })
        rootItem.turnMap[turnId] = obj
    }

    function turnFinish(turnId: string, finishedAt: double): void {
        const turn = rootItem.turnMap[turnId]
        turn.finishedAt = finishedAt
    }

    function chatClear(): void {
        scrollStop()
        chatView.followTail = true
        planCard.explanation = ""
        planCard.steps = []
        planCard.minimized = true
        requestsClear()
        compactStatusTimer.stop()
        compactCard.completed = false
        usageUpdate(0)
        for (let i = chatColumn.children.length - 1; i >= 0; --i) {
            chatColumn.children[i].destroy();
        }
        rootItem.turnMap = ({})
        rootItem.chatMap = ({})
        rootItem.subagentMap = ({})
    }

    function chatCreate(turnId: string, messageId: string, role: string): void {
        const turn = rootItem.turnMap[turnId]
        const obj = chatComponent.createObject(turn.messages, {
            turn: turn,
            messageId: messageId,
            role: role,
        })
        rootItem.chatMap[messageId] = obj
    }

    function chatAppend(messageId: string, text: string): void {
        const chat = rootItem.chatMap[messageId]
        chat.contentBuffer += text
        if (chat.role === "user") chat.turn.prompt += text
        else {
            chat.turn.lastId = messageId
            if (chat.role === "assistant") chat.turn.response = chat.contentBuffer
        }
    }

    function chatReasoningAppend(messageId: string, text: string): void {
        const chat = rootItem.chatMap[messageId]
        chat.reasoningBuffer += text
        chat.turn.lastId = messageId
    }

    function chatFinish(messageId: string): void {
        rootItem.chatMap[messageId].reasoningBuffer = ""
    }

    function subagentCreate(turnId: string, runtimeId: string, role: string, message: string): void {
        const turn = rootItem.turnMap[turnId]
        const obj = subagentComponent.createObject(turn.subagents, {
            runtimeId: runtimeId,
            role: role,
            message: message,
        })
        rootItem.subagentMap[runtimeId] = obj
    }

    function subagentUpdate(runtimeId: string, message: string): void {
        rootItem.subagentMap[runtimeId].message = message
    }

    function permissionRequest(runtimeId: string, role: string, message: string): void {
        permissionModel.append({
            "runtimeId": runtimeId,
            "role": role,
            "message": message
        })
        permissionSwipeView.currentIndex = permissionModel.count - 1
    }

    function permissionResponse(runtimeId: string, status: bool): void {
        agentModule.permission(runtimeId, status)
        for (let index = 0; index < permissionModel.count; ++index) {
            if (permissionModel.get(index).runtimeId !== runtimeId) continue
            permissionModel.remove(index)
            permissionSwipeView.currentIndex = Math.min(permissionSwipeView.currentIndex, permissionModel.count - 1)
            return
        }
    }

    function userInputRequest(runtimeId: string, role: string, request): void {
        const question = request && request.question ? request.question : ""
        const options = request && request.options ? request.options : []
        userInputModel.append({
            "runtimeId": runtimeId,
            "role": role,
            "question": question,
            "options": options
        })
        userInputSwipeView.currentIndex = userInputModel.count - 1
    }

    function userInputResponse(runtimeId: string, answer: string): void {
        agentModule.userInput(runtimeId, answer)
        for (let index = 0; index < userInputModel.count; ++index) {
            if (userInputModel.get(index).runtimeId !== runtimeId) continue
            userInputModel.remove(index)
            userInputSwipeView.currentIndex = Math.min(userInputSwipeView.currentIndex, userInputModel.count - 1)
            return
        }
    }

    function requestsClear(): void {
        permissionModel.clear()
        userInputModel.clear()
    }

    function planUpdate(plan): void {
        planCard.explanation = plan.explanation ? plan.explanation : ""
        planCard.steps = plan.plan ? plan.plan : []
    }

    function compactFinish(): void {
        compactCard.completed = true
        compactStatusTimer.restart()
    }

    function usageUpdate(totalTokens: double): void {
        const usage = totalTokens || 0
        contextLabel.updateText(usage > 0 ? usageLayout.formatTokens(usage) : "-")
        usageLayout.currentUsage = usage
    }

    function modelUpdate(contextWindow: double): void {
        usageLayout.contextWindow = contextWindow || 0
    }

    Component.onCompleted: {
        const objects = {
            "conversationComboBox": conversationComboBox,
            "textArea": textArea,
            "strategyButton": strategyButton,
            "modeButton": modeButton,
            "modelButton": modelButton
        };
        agentModule.propertyGet(objects)
    }
}
