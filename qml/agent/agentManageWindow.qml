import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: global.back
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal
        handle: Item {
            implicitWidth: 5

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 1
                color: global.stroke
            }
        }

        Rectangle {
            color: global.back
            SplitView.preferredWidth: 180; SplitView.fillHeight: true

            ListView {
                id: navigationView
                anchors.fill: parent
                anchors.margins: 10
                clip: true
                currentIndex: 0
                model: [
                    {"title": qsTr("Models"), "icon": "qrc:/icon/model.svg"},
                    {"title": qsTr("MCP"), "icon": "qrc:/icon/mcp.svg"},
                    {"title": qsTr("Skills"), "icon": "qrc:/icon/skill.svg"},
                    {"title": qsTr("Hooks"), "icon": "qrc:/icon/hook.svg"},
                    {"title": qsTr("Context"), "icon": "qrc:/icon/database.svg"}
                ]
                spacing: 2

                delegate: ItemDelegate {
                    required property int index
                    required property var modelData
                    width: navigationView.width
                    height: 36
                    text: modelData.title
                    icon.source: modelData.icon
                    icon.width: 18
                    icon.height: 18
                    spacing: 10
                    highlighted: navigationView.currentIndex === index

                    onClicked: navigationView.currentIndex = index
                }
            }
        }

        StackLayout {
            currentIndex: navigationView.currentIndex
            SplitView.fillWidth: true; SplitView.fillHeight: true

            SettingsPage {
                title: qsTr("Models")
                description: qsTr("Configure model providers, credentials, and available models.")
                actionText: qsTr("Add Provider")
                actionIcon: "qrc:/icon/add.svg"

                onTriggerAction: providerInsertDialog.openInsert()

                Repeater {
                    model: providerModel

                    delegate: Rectangle {
                        id: providerCard
                        property string providerId: model.id
                        property string providerName: model.display
                        property url providerIcon: model.decoration
                        property string providerApikey: model.apikey
                        property url providerBaseUrl: model.baseUrl
                        property bool providerCustom: model.custom
                        property string providerChatEndpoint: model.chatEndpoint
                        property string providerModelEndpoint: model.modelEndpoint
                        property var providerConfig: model.config
                        property var providerModels: model.models

                        function configGet(): var {
                            const config = {}
                            for (const key in providerConfig) config[key] = providerConfig[key]
                            return config
                        }

                        function edit(key: string, label: string, value: string, allowEmpty: bool): void {
                            providerEditDialog.openEdit(
                                providerId,
                                key,
                                label,
                                value,
                                configGet(),
                                allowEmpty
                            )
                        }

                        function modelRemove(modelId: string): void {
                            const config = configGet()
                            const models = {}
                            const currentModels = config.models || {}
                            for (const id in currentModels) {
                                if (id !== modelId) models[id] = currentModels[id]
                            }
                            config.models = models
                            agentModule.providerEdit(providerId, config)
                        }

                        radius: 6
                        color: global.backHover
                        border.color: global.stroke
                        implicitHeight: providerLayout.implicitHeight + 32
                        Layout.fillWidth: true

                        RowLayout {
                            id: providerLayout
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 12

                            Rectangle {
                                radius: 6
                                color: global.backSelected
                                Layout.preferredWidth: 42
                                Layout.preferredHeight: 42
                                Layout.alignment: Qt.AlignTop

                                IconImage {
                                    anchors.centerIn: parent
                                    width: 24
                                    height: 24
                                    source: providerCard.providerIcon
                                }
                            }

                            ColumnLayout {
                                spacing: 10
                                Layout.fillWidth: true

                                RowLayout {
                                    spacing: 8
                                    Layout.fillWidth: true

                                    Label {
                                        text: providerCard.providerName
                                        font.bold: true
                                    }

                                    Button {
                                        visible: providerCard.providerCustom
                                        leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                        flat: true
                                        icon.source: "qrc:/icon/edit.svg"
                                        icon.width: 16; icon.height: 16
                                        Layout.preferredWidth: 20; Layout.preferredHeight: 20

                                        onClicked: providerCard.edit(
                                            "name",
                                            qsTr("Provider name"),
                                            providerCard.providerName,
                                            false
                                        )
                                    }

                                    Item {
                                        Layout.fillWidth: true
                                    }

                                    Button {
                                        leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                        checkable: true
                                        flat: true
                                        icon.source: checked ? "qrc:/icon/checkmark.svg" : "qrc:/icon/delete.svg"
                                        icon.width: 16; icon.height: 16
                                        Layout.preferredWidth: 24; Layout.preferredHeight: 24

                                        onToggled: {
                                            if (!checked) agentModule.providerRemove(providerCard.providerId)
                                        }

                                        Timer {
                                            interval: 1000
                                            running: parent.checked
                                            onTriggered: parent.checked = false
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true

                                    Label {
                                        text: qsTr("Base URL: %1").arg(providerCard.providerBaseUrl.toString())
                                        color: global.stroke
                                        elide: Text.ElideRight
                                    }

                                    Button {
                                        visible: providerCard.providerCustom
                                        leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                        flat: true
                                        icon.source: "qrc:/icon/edit.svg"
                                        icon.width: 16; icon.height: 16
                                        Layout.preferredWidth: 20; Layout.preferredHeight: 20

                                        onClicked: providerCard.edit(
                                            "api",
                                            qsTr("Base URL"),
                                            providerCard.providerBaseUrl.toString(),
                                            false
                                        )
                                    }

                                    Item {
                                        Layout.fillWidth: true
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true

                                    Label {
                                        text: qsTr("Chat endpoint: %1").arg(providerCard.providerChatEndpoint)
                                        color: global.stroke
                                        elide: Text.ElideRight
                                    }

                                    Button {
                                        visible: providerCard.providerCustom
                                        leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                        flat: true
                                        icon.source: "qrc:/icon/edit.svg"
                                        icon.width: 16; icon.height: 16
                                        Layout.preferredWidth: 20; Layout.preferredHeight: 20

                                        onClicked: providerCard.edit(
                                            "chatEndpoint",
                                            qsTr("Chat endpoint"),
                                            providerCard.providerChatEndpoint,
                                            false
                                        )
                                    }

                                    Item {
                                        Layout.fillWidth: true
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true

                                    Label {
                                        text: qsTr("Model endpoint: %1").arg(providerCard.providerModelEndpoint)
                                        color: global.stroke
                                        elide: Text.ElideRight
                                    }

                                    Button {
                                        visible: providerCard.providerCustom
                                        leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                        flat: true
                                        icon.source: "qrc:/icon/edit.svg"
                                        icon.width: 16; icon.height: 16
                                        Layout.preferredWidth: 20; Layout.preferredHeight: 20

                                        onClicked: providerCard.edit(
                                            "modelsEndpoint",
                                            qsTr("Model endpoint"),
                                            providerCard.providerModelEndpoint,
                                            true
                                        )
                                    }

                                    Item {
                                        Layout.fillWidth: true
                                    }
                                }

                                Rectangle {
                                    color: global.stroke
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 1
                                }

                                Label {
                                    text: qsTr("API Key")
                                    font.bold: true
                                }

                                RowLayout {
                                    spacing: 8
                                    Layout.fillWidth: true

                                    TextField {
                                        id: apikeyTextField
                                        text: providerCard.providerApikey
                                        placeholderText: qsTr("Enter API key")
                                        echoMode: TextInput.Password
                                        Layout.fillWidth: true

                                        onAccepted: agentModule.apikeySet(providerCard.providerId, text)
                                    }

                                    Button {
                                        text: qsTr("Save")
                                        enabled: apikeyTextField.text.length > 0

                                        onClicked: agentModule.apikeySet(providerCard.providerId, apikeyTextField.text)
                                    }
                                }

                                Rectangle {
                                    color: global.stroke
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 1
                                }

                                RowLayout {
                                    Layout.fillWidth: true

                                    Label {
                                        text: qsTr("Available Models")
                                        font.bold: true
                                    }

                                    Item {
                                        Layout.fillWidth: true
                                    }

                                    Button {
                                        visible: providerCard.providerModelEndpoint.length > 0
                                        leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                        flat: true
                                        icon.source: "qrc:/icon/arrowRepeat.svg"
                                        icon.width: 16; icon.height: 16
                                        Layout.preferredWidth: 20; Layout.preferredHeight: 20

                                        onClicked: agentModule.providerModelsGet(providerCard.providerId)
                                    }

                                    Button {
                                        visible: providerCard.providerCustom
                                                 && providerCard.providerModelEndpoint.length === 0
                                        leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                        flat: true
                                        icon.source: "qrc:/icon/add.svg"
                                        icon.width: 16; icon.height: 16
                                        Layout.preferredWidth: 20; Layout.preferredHeight: 20

                                        onClicked: providerModelInsertDialog.openInsert(
                                            providerCard.providerId,
                                            providerCard.configGet()
                                        )
                                    }
                                }

                                Repeater {
                                    model: providerCard.providerModels

                                    delegate: Rectangle {
                                        required property int index
                                        required property string display
                                        required property string modelId
                                        required property int contextWindow
                                        required property int maxOutputTokens

                                        radius: 4
                                        color: index % 2 === 0 ? global.back : "transparent"
                                        implicitHeight: 38
                                        Layout.fillWidth: true

                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.leftMargin: 10
                                            anchors.rightMargin: 10
                                            spacing: 12

                                            ColumnLayout {
                                                spacing: 0
                                                Layout.fillWidth: true

                                                Label {
                                                    text: display
                                                    elide: Text.ElideRight
                                                    Layout.fillWidth: true
                                                }

                                                Label {
                                                    text: modelId
                                                    color: global.stroke
                                                    font.pixelSize: 11
                                                    elide: Text.ElideRight
                                                    Layout.fillWidth: true
                                                }
                                            }

                                            Label {
                                                text: qsTr("Context %1").arg(contextWindow.toLocaleString(Qt.locale(), "f", 0))
                                                color: global.stroke
                                            }

                                            Label {
                                                text: qsTr("Output %1").arg(maxOutputTokens.toLocaleString(Qt.locale(), "f", 0))
                                                color: global.stroke
                                            }

                                            Button {
                                                visible: providerCard.providerCustom
                                                         && providerCard.providerModelEndpoint.length === 0
                                                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                                checkable: true
                                                flat: true
                                                icon.source: checked ? "qrc:/icon/checkmark.svg" : "qrc:/icon/delete.svg"
                                                icon.width: 16; icon.height: 16
                                                Layout.preferredWidth: 20; Layout.preferredHeight: 20

                                                onClicked: {
                                                    if (!checked) providerCard.modelRemove(modelId)
                                                }

                                                Timer {
                                                    interval: 1000
                                                    running: parent.checked
                                                    onTriggered: parent.checked = false
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            SettingsPage {
                title: qsTr("MCP")
                description: qsTr("Add and manage MCP servers exposed to the agent.")
                actionText: qsTr("Add Server")
                actionIcon: "qrc:/icon/add.svg"

                onTriggerAction: mcpInsertDialog.openInsert()

                Repeater {
                    model: mcpModel

                    delegate: Rectangle {
                        id: serverCard
                        property string serverName: model.display || ""
                        property url serverUrl: model.url
                        property bool serverEnabled: model.enabled
                        property string serverVersion: model.version || ""
                        property string serverDescription: model.description || ""
                        property url serverIcon: model.decoration || "qrc:/icon/mcp.svg"

                        radius: 6
                        color: global.backHover
                        border.color: global.stroke
                        Layout.fillWidth: true
                        Layout.preferredHeight: 168

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 12

                            Rectangle {
                                radius: 6
                                color: global.backSelected
                                Layout.preferredWidth: 42
                                Layout.preferredHeight: 42
                                Layout.alignment: Qt.AlignTop

                                IconImage {
                                    anchors.centerIn: parent
                                    width: 24
                                    height: 24
                                    source: serverCard.serverIcon
                                    color: model.decoration ? "transparent" : global.fore
                                }
                            }

                            ColumnLayout {
                                spacing: 4
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                RowLayout {
                                    spacing: 8
                                    Layout.fillWidth: true

                                    Label {
                                        text: serverCard.serverName
                                        font.bold: true
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }

                                    Label {
                                        visible: serverCard.serverVersion.length > 0
                                        text: serverCard.serverVersion
                                        color: global.stroke
                                    }
                                }

                                Label {
                                    text: serverCard.serverUrl.toString()
                                    color: global.stroke
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }

                                Label {
                                    text: serverCard.serverDescription
                                    color: global.stroke
                                    wrapMode: Text.Wrap
                                    maximumLineCount: 3
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }

                                Item {
                                    Layout.fillHeight: true
                                }
                            }

                            RowLayout {
                                spacing: 4
                                Layout.alignment: Qt.AlignTop

                                Switch {
                                    checked: serverCard.serverEnabled

                                    onClicked: agentModule.mcpEnabledSet(serverCard.serverUrl, checked)
                                }

                                Button {
                                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                    checkable: true
                                    flat: true
                                    icon.source: checked ? "qrc:/icon/checkmark.svg" : "qrc:/icon/delete.svg"
                                    icon.width: 16; icon.height: 16
                                    Layout.preferredWidth: 24; Layout.preferredHeight: 24

                                    onToggled: {
                                        if (!checked) agentModule.mcpRemove(serverCard.serverUrl)
                                    }

                                    Timer {
                                        interval: 1000
                                        running: parent.checked
                                        onTriggered: parent.checked = false
                                    }
                                }
                            }
                        }
                    }
                }
            }

            SettingsPage {
                title: qsTr("Skills")
                description: qsTr("Choose the skills loaded while preparing a turn.")
            }

            SettingsPage {
                title: qsTr("Hooks")
                description: qsTr("Configure scripts for agent lifecycle events.")
            }

            SettingsPage {
                title: qsTr("Context")
                description: qsTr("Manage context limits, token budgets, and compaction.")
            }
        }
    }

    Dialog {
        id: providerInsertDialog
        width: 520
        x: (rootItem.width - width) / 2
        y: (rootItem.height - height) / 2
        modal: true
        title: qsTr("Add Provider")
        property bool deepseekAdded: false

        function openInsert(): void {
            deepseekAdded = agentModule.providerExists("deepseek")
            providerNameTextField.clear()
            providerBaseUrlTextField.clear()
            open()
        }

        function submitCustom(): void {
            agentModule.providerInsert("", {
                "name": providerNameTextField.text.trim(),
                "api": providerBaseUrlTextField.text.trim(),
                "chatEndpoint": "/chat/completions",
                "modelsEndpoint": "/models"
            })
            close()
        }

        contentItem: ColumnLayout {
            spacing: 12

            Label {
                text: qsTr("Preset")
                font.bold: true
            }

            Rectangle {
                radius: 4
                color: global.backHover
                border.color: global.stroke
                Layout.fillWidth: true
                Layout.preferredHeight: 56

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    IconImage {
                        source: "qrc:/icon/deepseek.svg"
                        Layout.preferredWidth: 24; Layout.preferredHeight: 24
                    }

                    ColumnLayout {
                        spacing: 0
                        Layout.fillWidth: true

                        Label {
                            text: "DeepSeek"
                            font.bold: true
                        }

                        Label {
                            text: qsTr("Built-in OpenAI-compatible provider")
                            color: global.stroke
                        }
                    }

                    Button {
                        text: providerInsertDialog.deepseekAdded ? qsTr("Added") : qsTr("Add")
                        enabled: !providerInsertDialog.deepseekAdded
                        leftPadding: 12; rightPadding: 12
                        implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding

                        onClicked: {
                            agentModule.providerInsert("deepseek", {})
                            providerInsertDialog.deepseekAdded = true
                        }
                    }
                }
            }

            Rectangle {
                color: global.stroke
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }

            Label {
                text: qsTr("OpenAI-compatible")
                font.bold: true
            }

            TextField {
                id: providerNameTextField
                placeholderText: qsTr("Provider name")
                Layout.fillWidth: true
            }

            TextField {
                id: providerBaseUrlTextField
                placeholderText: qsTr("Base URL, for example http://localhost:8000/v1")
                Layout.fillWidth: true
            }

            RowLayout {
                spacing: 8
                Layout.fillWidth: true

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: qsTr("Cancel")

                    onClicked: providerInsertDialog.close()
                }

                Button {
                    text: qsTr("Add")
                    highlighted: true
                    enabled: providerNameTextField.text.trim().length > 0
                             && providerBaseUrlTextField.text.trim().length > 0

                    onClicked: providerInsertDialog.submitCustom()
                }
            }
        }
    }

    Dialog {
        id: providerEditDialog
        width: 480
        x: (rootItem.width - width) / 2
        y: (rootItem.height - height) / 2
        modal: true
        property string providerId
        property string field
        property var providerConfig
        property bool allowEmpty: false

        function openEdit(id: string, key: string, label: string, value: string, config: var, empty: bool): void {
            providerId = id
            field = key
            providerConfig = config
            title = qsTr("Edit %1").arg(label)
            allowEmpty = empty
            providerEditTextField.text = value
            open()
        }

        function submit(): void {
            const config = {}
            for (const key in providerConfig) config[key] = providerConfig[key]
            config[field] = providerEditTextField.text.trim()
            agentModule.providerEdit(providerId, config)
            close()
        }

        onOpened: {
            providerEditTextField.forceActiveFocus()
            providerEditTextField.selectAll()
        }

        contentItem: ColumnLayout {
            spacing: 12

            TextField {
                id: providerEditTextField
                Layout.fillWidth: true

                onAccepted: {
                    if (providerEditDialog.allowEmpty || text.trim().length > 0) providerEditDialog.submit()
                }
            }

            RowLayout {
                spacing: 8
                Layout.fillWidth: true

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: qsTr("Cancel")

                    onClicked: providerEditDialog.close()
                }

                Button {
                    text: qsTr("Save")
                    highlighted: true
                    enabled: providerEditDialog.allowEmpty || providerEditTextField.text.trim().length > 0

                    onClicked: providerEditDialog.submit()
                }
            }
        }
    }

    Dialog {
        id: providerModelInsertDialog
        width: 480
        x: (rootItem.width - width) / 2
        y: (rootItem.height - height) / 2
        modal: true
        title: qsTr("Add Model")
        property string providerId
        property var providerConfig

        function openInsert(id: string, config: var): void {
            providerId = id
            providerConfig = config
            providerModelIdTextField.clear()
            providerModelNameTextField.clear()
            providerModelContextTextField.clear()
            providerModelOutputTextField.clear()
            open()
        }

        function submit(): void {
            const models = {}
            const currentModels = providerConfig.models || {}
            for (const id in currentModels) models[id] = currentModels[id]
            const modelId = providerModelIdTextField.text.trim()
            models[modelId] = {
                "name": providerModelNameTextField.text.trim(),
                "limit": {
                    "context": Number(providerModelContextTextField.text),
                    "output": Number(providerModelOutputTextField.text)
                }
            }
            providerConfig.models = models
            agentModule.providerEdit(providerId, providerConfig)
            close()
        }

        onOpened: providerModelIdTextField.forceActiveFocus()

        contentItem: ColumnLayout {
            spacing: 12

            TextField {
                id: providerModelIdTextField
                placeholderText: qsTr("Model ID")
                Layout.fillWidth: true
            }

            TextField {
                id: providerModelNameTextField
                placeholderText: qsTr("Display name")
                Layout.fillWidth: true
            }

            TextField {
                id: providerModelContextTextField
                placeholderText: qsTr("Context window")
                validator: IntValidator { bottom: 1 }
                Layout.fillWidth: true
            }

            TextField {
                id: providerModelOutputTextField
                placeholderText: qsTr("Maximum output tokens")
                validator: IntValidator { bottom: 1 }
                Layout.fillWidth: true
            }

            RowLayout {
                spacing: 8
                Layout.fillWidth: true

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: qsTr("Cancel")

                    onClicked: providerModelInsertDialog.close()
                }

                Button {
                    text: qsTr("Add")
                    highlighted: true
                    enabled: providerModelIdTextField.text.trim().length > 0
                             && providerModelNameTextField.text.trim().length > 0
                             && providerModelContextTextField.acceptableInput
                             && providerModelOutputTextField.acceptableInput

                    onClicked: providerModelInsertDialog.submit()
                }
            }
        }
    }

    Dialog {
        id: mcpInsertDialog
        width: 480
        x: (rootItem.width - width) / 2
        y: (rootItem.height - height) / 2
        modal: true
        title: qsTr("Add MCP Server")

        function openInsert(): void {
            urlTextField.clear()
            errorLabel.text = ""
            open()
        }

        function submit(): void {
            const error = agentModule.mcpInsert(urlTextField.text.trim())
            errorLabel.text = error
            if (error.length === 0) close()
        }

        onOpened: urlTextField.forceActiveFocus()

        contentItem: ColumnLayout {
            spacing: 12

            TextField {
                id: urlTextField
                placeholderText: qsTr("https://example.com/mcp")
                Layout.fillWidth: true

                onAccepted: mcpInsertDialog.submit()
            }

            Label {
                id: errorLabel
                visible: text.length > 0
                color: global.dangerFore3
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }

            RowLayout {
                spacing: 8
                Layout.fillWidth: true

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: qsTr("Cancel")

                    onClicked: mcpInsertDialog.close()
                }

                Button {
                    text: qsTr("Add")
                    highlighted: true
                    enabled: urlTextField.text.trim().length > 0

                    onClicked: mcpInsertDialog.submit()
                }
            }
        }
    }

    component SettingsPage: Item {
        id: settingsPage
        default property alias content: pageContent.data
        property string title
        property string description
        property string actionText
        property url actionIcon

        signal triggerAction()

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 8

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: settingsPage.title
                    font.pixelSize: 22
                    font.bold: true
                    Layout.fillWidth: true
                }

                Button {
                    visible: settingsPage.actionText.length > 0
                    text: settingsPage.actionText
                    icon.source: settingsPage.actionIcon
                    icon.width: 16
                    icon.height: 16

                    onClicked: settingsPage.triggerAction()
                }
            }

            Label {
                text: settingsPage.description
                color: global.stroke
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }

            ScrollView {
                id: settingsPageScrollView
                clip: true
                contentWidth: availableWidth
                rightPadding: settingsPageScrollBar.width
                Layout.fillWidth: true
                Layout.fillHeight: true

                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                ScrollBar.vertical: ScrollBar {
                    id: settingsPageScrollBar
                    x: settingsPageScrollView.width - width
                    y: settingsPageScrollView.topPadding
                    height: settingsPageScrollView.availableHeight
                    policy: ScrollBar.AsNeeded
                    palette {
                        mid: global.stroke
                        dark: global.strokePressed
                    }
                }

                ColumnLayout {
                    id: pageContent
                    width: settingsPageScrollView.availableWidth
                }
            }
        }
    }
}
