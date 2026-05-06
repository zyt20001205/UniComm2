import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Controls.FluentWinUI3.impl
import QtQuick.Dialogs
import QtQuick.Layouts

Item {
    id: mainItem
    anchors.fill: parent

    // overlay control
    property int widgetCount: 0

    Item {
        id: debugConsole
        anchors.fill: parent
        // visible: true
        visible: false

        Rectangle {
            anchors.fill: parent
            color: "transparent"
            border.color: global.dangerFore3
            border.width: 3
        }

        ColumnLayout {
            anchors.centerIn: parent

            Label {
                text: Application.styleHints.colorScheme === Qt.Light ? "Light" : "Dark"
                color: global.dangerFore3
                font.pointSize: 16
            }

            Label {
                id: debugWidgetCount
                color: global.dangerFore3
                font.pointSize: 16
            }
        }
    }

    onWidgetCountChanged: {
        if (widgetCount < 0) {
            console.log("severe error occurred!!! widget count: " + widgetCount)
            widgetCount = 0
        }
        if (widgetCount === 0) {
            mainWindow.overlayFlagSet(true, false)
        }

        if (debugConsole.visible) {
            debugWidgetCount.text = "widget count: " + widgetCount
        }
    }

    // main window
    Dialog {
        id: mainWindowBusyDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        standardButtons: Dialog.Abort
        topPadding: 30; bottomPadding: 20

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onRejected: fileModule.processTerminate()

        ProgressBar {
            width: parent.width
            indeterminate: true
        }
    }

    Dialog {
        id: mainWindowCloseDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        title: qsTr("Save and Exit?")
        standardButtons: Dialog.Yes | Dialog.No

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAccepted: {
            mainWindowCloseDialog.close()
            mainWindow.quit()
        }
    }

    Dialog {
        id: mainWindowMessageDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        standardButtons: Dialog.Ok
        property string text
        topPadding: 30; bottomPadding: 20

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        Label {
            text: mainWindowMessageDialog.text
        }
    }

    Dialog {
        id: mainWindowQuitDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        title: qsTr("Exiting application...")
        standardButtons: Dialog.Abort
        topPadding: 30; bottomPadding: 20
        property real primaryProgress
        property string primaryLog
        property real secondaryProgress
        property string secondaryLog

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onRejected: mainWindow.terminate()

        ColumnLayout {
            width: parent.width

            ProgressBar {
                value: mainWindowQuitDialog.primaryProgress
                Layout.fillWidth: true
            }

            Label {
                text: mainWindowQuitDialog.primaryLog
                horizontalAlignment: Text.AlignRight
                Layout.fillWidth: true
            }

            Item {
                Layout.fillWidth: true; Layout.preferredHeight: 10
            }

            ProgressBar {
                value: mainWindowQuitDialog.secondaryProgress
                Layout.fillWidth: true
            }

            Label {
                text: mainWindowQuitDialog.secondaryLog
                horizontalAlignment: Text.AlignRight
                Layout.fillWidth: true
            }
        }
    }

    Component {
        id: mainWindowMessageComponent

        Dialog {
            id: mainWindowMessageDialog
            parent: Overlay.overlay
            anchors.centerIn: parent
            width: 600
            modal: true
            standardButtons: Dialog.Ok
            property string text
            topPadding: 30; bottomPadding: 20

            onOpened: {
                mainWindow.overlayFlagSet(undefined, true)
                widgetCount += 1
            }
            onClosed: {
                widgetCount -= 1
                destroy()
            }

            Label {
                text: mainWindowMessageDialog.text
            }
        }
    }

    function messageDialogNew(eventloop, title, text) {
        const messageDialog = mainWindowMessageComponent.createObject(mainItem, {
            "title": title,
            "text": text
        });
        messageDialog.open()
        messageDialog.closed.connect(() => {
            eventloop.quit()
        });
    }

    ToolTip {
        id: mainWindowTextView
        parent: Overlay.overlay
        closePolicy: Popup.CloseOnPressOutside | Popup.CloseOnReleaseOutside
        property point position
        property string data

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: {
            widgetCount -= 1
            mainWindowTextView.data = ""
        }
        onAboutToShow: {
            x = position.x + 10
            y = position.y + 10
        }

        contentItem: ColumnLayout {
            RowLayout {
                id: mainWindowTextViewLayout

                Item {
                    Layout.fillWidth: true; Layout.preferredHeight: 12

                    MouseArea {
                        anchors.fill: parent
                        preventStealing: true
                        property real lastX
                        property real lastY

                        onPressed: (mouse) => {
                            lastX = mouse.x
                            lastY = mouse.y
                        }

                        onPositionChanged: (mouse) => {
                            mainWindowTextView.x += mouse.x - lastX
                            mainWindowTextView.y += mouse.y - lastY
                        }
                    }
                }
            }

            ScrollView {
                Layout.minimumWidth: 600; Layout.maximumWidth: 1200
                Layout.minimumHeight: 200; Layout.maximumHeight: 400

                TextArea {
                    id: mainWindowTextViewTextArea
                    readOnly: true
                    text: mainWindowTextView.data
                    textFormat: TextEdit.AutoText
                    verticalAlignment: TextEdit.AlignTop
                    wrapMode: TextEdit.Wrap
                }
            }
        }
    }

    ToolTip {
        id: mainWindowToolTip
        parent: Overlay.overlay
        closePolicy: Popup.NoAutoClose
        visible: text
        property point position

        onPositionChanged: {
            let p = Overlay.overlay.mapFromGlobal(position)
            x = p.x + 10
            y = p.y + 10
        }

        Behavior on x {
            enabled: mainWindowToolTip.visible
            NumberAnimation {
                duration: 50
                easing.type: Easing.Linear
            }
        }
        Behavior on y {
            enabled: mainWindowToolTip.visible
            NumberAnimation {
                duration: 50
                easing.type: Easing.Linear
            }
        }
    }

    // luals
    Dialog {
        id: lualsProgressDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        closePolicy: Popup.NoAutoClose
        modal: true
        title: qsTr("Lua language server initializing...")
        topPadding: 30; bottomPadding: 20
        visible: !(done2 && done3)
        property real create2
        property real create3
        property bool done2: false
        property bool done3: false

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        ColumnLayout {
            width: parent.width

            ProgressBar {
                value: lualsProgressDialog.create2
                Layout.fillWidth: true
            }

            Item {
                Layout.fillWidth: true; Layout.preferredHeight: 10
            }

            ProgressBar {
                value: lualsProgressDialog.create3
                Layout.fillWidth: true
            }
        }
    }

    // breakpoint module
    Dialog {
        id: breakpointModuleEditDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        title: qsTr("Breakpoint Setting")
        standardButtons: Dialog.Ok
        property url documentUrl
        property int line

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            breakpointModuleEnabledCheckBox.checkState = breakpointModule.enabledGet(breakpointModuleEditDialog.documentUrl, breakpointModuleEditDialog.line) ? Qt.Checked : Qt.Unchecked
            breakpointModuleConditionTextField.text = breakpointModule.conditionGet(breakpointModuleEditDialog.documentUrl, breakpointModuleEditDialog.line)
            breakpointModuleConditionTextField.forceActiveFocus()
            breakpointModuleConditionTextField.selectAll()
        }
        onAccepted: {
            breakpointModule.enabledSet(breakpointModuleEditDialog.documentUrl, breakpointModuleEditDialog.line, breakpointModuleEnabledCheckBox.checked)
            breakpointModule.conditionSet(breakpointModuleEditDialog.documentUrl, breakpointModuleEditDialog.line, breakpointModuleConditionTextField.text)
            breakpointModule.breakpointReload(breakpointModuleEditDialog.documentUrl, breakpointModuleEditDialog.line)
        }

        ColumnLayout {
            width: parent.width

            Label {
                text: breakpointModuleEditDialog.documentUrl + ":" + breakpointModuleEditDialog.line
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }

            CheckBox {
                id: breakpointModuleEnabledCheckBox
                text: qsTr("Enabled")
            }

            TextField {
                id: breakpointModuleConditionTextField
                placeholderText: qsTr("true")
                Layout.fillWidth: true

                onAccepted: breakpointModuleEditDialog.accept()
                Keys.onEscapePressed: breakpointModuleEditDialog.reject()
            }
        }
    }

    Menu {
        id: breakpointModuleLineMenu
        property url documentUrl
        property int line
        property var treeView

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("View")
            icon.source: "qrc:/icon/eye.svg"
            icon.width: 16; icon.height: 16

            onTriggered: breakpointModule.markerAdd(breakpointModuleLineMenu.documentUrl, breakpointModuleLineMenu.line)
        }

        MenuItem {
            text: qsTr("Setting")
            icon.source: "qrc:/icon/settings.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                breakpointModuleEditDialog.documentUrl = breakpointModuleLineMenu.documentUrl
                breakpointModuleEditDialog.line = breakpointModuleLineMenu.line
                breakpointModuleEditDialog.open()
            }
        }

        MenuItem {
            text: qsTr("Delete")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16

            onTriggered: breakpointModule.breakpointDelete(breakpointModuleLineMenu.documentUrl, breakpointModuleLineMenu.line)
        }

        MenuSeparator {
        }

        Menu {
            title: qsTr("Folding")
            icon.source: "qrc:/icon/fold.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Collapse All")
                icon.source: "qrc:/icon/collapse.svg"
                icon.width: 16; icon.height: 16

                onTriggered: breakpointModuleLineMenu.treeView.collapseRecursively()
            }

            MenuItem {
                text: qsTr("Expand All")
                icon.source: "qrc:/icon/expand.svg"
                icon.width: 16; icon.height: 16

                onTriggered: breakpointModuleLineMenu.treeView.expandRecursively()
            }
        }
    }

    Menu {
        id: breakpointModuleFileMenu
        property url documentUrl
        property var treeView

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        Menu {
            title: qsTr("Delete Breakpoints")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16

            DelayButton {
                delay: 1000
                text: qsTr("Confirm")

                onActivated: {
                    breakpointModule.breakpointsDelete(breakpointModuleFileMenu.documentUrl)
                    progress = 0
                    breakpointModuleFileMenu.close()
                }
            }
        }

        MenuSeparator {
        }

        Menu {
            title: qsTr("Folding")
            icon.source: "qrc:/icon/fold.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Collapse All")
                icon.source: "qrc:/icon/collapse.svg"
                icon.width: 16; icon.height: 16

                onTriggered: breakpointModuleFileMenu.treeView.collapseRecursively()
            }

            MenuItem {
                text: qsTr("Expand All")
                icon.source: "qrc:/icon/expand.svg"
                icon.width: 16; icon.height: 16

                onTriggered: breakpointModuleFileMenu.treeView.expandRecursively()
            }
        }
    }

    Menu {
        id: breakpointModuleRootMenu
        property var treeView

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        Menu {
            title: qsTr("Delete All")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16

            DelayButton {
                delay: 1000
                text: qsTr("Confirm")

                onActivated: {
                    breakpointModule.allDelete()
                    progress = 0
                    breakpointModuleRootMenu.close()
                }
            }
        }

        MenuSeparator {
        }

        Menu {
            title: qsTr("Folding")
            icon.source: "qrc:/icon/fold.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Collapse All")
                icon.source: "qrc:/icon/collapse.svg"
                icon.width: 16; icon.height: 16

                onTriggered: breakpointModuleRootMenu.treeView.collapseRecursively()
            }

            MenuItem {
                text: qsTr("Expand All")
                icon.source: "qrc:/icon/expand.svg"
                icon.width: 16; icon.height: 16

                onTriggered: breakpointModuleRootMenu.treeView.expandRecursively()
            }
        }
    }

    // database module
    Dialog {
        id: databaseModuleEditDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        title: qsTr("Enter Key Name")
        standardButtons: Dialog.Ok
        property int databaseIndex
        property string databaseKey

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            databaseModuleNameTextField.text = databaseModuleEditDialog.databaseKey
            databaseModuleNameTextField.forceActiveFocus()
            databaseModuleNameTextField.selectAll()
        }
        onAccepted: {
            if (databaseModuleEditDialog.databaseIndex === -1 || !databaseModuleEditDialog.databaseKey) {
                databaseModule.databaseInsert(databaseModuleEditDialog.databaseIndex, databaseModuleNameTextField.text)
            } else {
                databaseModule.databaseRename(databaseModuleEditDialog.databaseIndex, databaseModuleNameTextField.text)
            }
        }

        TextField {
            id: databaseModuleNameTextField
            width: parent.width
            placeholderText: qsTr("Enter key:")

            onAccepted: databaseModuleEditDialog.accept()
            Keys.onEscapePressed: databaseModuleEditDialog.reject()
        }
    }

    Menu {
        id: databaseModuleTableMenu
        property int databaseIndex
        property string databaseKey

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Insert")
            icon.source: "qrc:/icon/add.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                databaseModuleEditDialog.databaseIndex = databaseModuleTableMenu.databaseIndex
                databaseModuleEditDialog.databaseKey = ""
                databaseModuleEditDialog.open()
            }
        }

        MenuItem {
            text: qsTr("Rename")
            icon.source: "qrc:/icon/rename.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                databaseModuleEditDialog.databaseIndex = databaseModuleTableMenu.databaseIndex
                databaseModuleEditDialog.databaseKey = databaseModuleTableMenu.databaseKey
                databaseModuleEditDialog.open()
            }
        }

        MenuItem {
            text: qsTr("Clear")
            icon.source: "qrc:/icon/eraser.svg"
            icon.width: 16; icon.height: 16

            onTriggered: databaseModule.databaseClear(databaseModuleTableMenu.databaseIndex)
        }

        MenuItem {
            text: qsTr("Delete")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16

            onTriggered: databaseModule.databaseRemove(databaseModuleTableMenu.databaseIndex)
        }
    }

    Menu {
        id: databaseModuleRootMenu

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("New")
            icon.source: "qrc:/icon/add.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                databaseModuleEditDialog.databaseIndex = -1
                databaseModuleEditDialog.databaseKey = ""
                databaseModuleEditDialog.open()
            }
        }

        Menu {
            title: qsTr("Clear")
            icon.source: "qrc:/icon/eraser.svg"
            icon.width: 16; icon.height: 16

            DelayButton {
                delay: 1000
                text: qsTr("Confirm")

                onActivated: {
                    databaseModule.databaseClear(-1)
                    progress = 0
                    databaseModuleRootMenu.close()
                }
            }
        }
    }

    // datatable module
    Dialog {
        id: datatableModuleEditDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        title: qsTr("Enter Key Name")
        standardButtons: Dialog.Ok
        property int datatableIndex
        property string datatableKey

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            datatableModuleNameTextField.text = datatableModuleEditDialog.datatableKey
            datatableModuleNameTextField.forceActiveFocus()
            datatableModuleNameTextField.selectAll()
        }
        onAccepted: {
            if (datatableModuleEditDialog.datatableIndex === -1 || !datatableModuleEditDialog.datatableKey) {
                datatableModule.datatableInsert(datatableModuleEditDialog.datatableIndex, datatableModuleNameTextField.text)
            } else {
                datatableModule.datatableRename(datatableModuleEditDialog.datatableIndex, datatableModuleNameTextField.text)
            }
        }

        TextField {
            id: datatableModuleNameTextField
            width: parent.width
            placeholderText: qsTr("Enter key:")

            onAccepted: datatableModuleEditDialog.accept()
            Keys.onEscapePressed: datatableModuleEditDialog.reject()
        }
    }

    Menu {
        id: datatableModuleTableMenu
        property int datatableIndex
        property string datatableKey

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Insert")
            icon.source: "qrc:/icon/add.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                datatableModuleEditDialog.datatableIndex = datatableModuleTableMenu.datatableIndex
                datatableModuleEditDialog.datatableKey = ""
                datatableModuleEditDialog.open()
            }
        }

        MenuItem {
            text: qsTr("Rename")
            icon.source: "qrc:/icon/rename.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                datatableModuleEditDialog.datatableIndex = datatableModuleTableMenu.datatableIndex
                datatableModuleEditDialog.datatableKey = datatableModuleTableMenu.datatableKey
                datatableModuleEditDialog.open()
            }
        }

        MenuItem {
            text: qsTr("Delete")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16

            onTriggered: datatableModule.datatableRemove(datatableModuleTableMenu.datatableIndex)
        }
    }

    Menu {
        id: datatableModuleRootMenu

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("New")
            icon.source: "qrc:/icon/add.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                datatableModuleEditDialog.datatableIndex = -1
                datatableModuleEditDialog.datatableKey = ""
                datatableModuleEditDialog.open()
            }
        }

        MenuItem {
            text: qsTr("Export")
            icon.source: "qrc:/icon/share.svg"
            icon.width: 16; icon.height: 16

            onTriggered: datatableModule.datatableExport("")
        }

        Menu {
            title: qsTr("Clear")
            icon.source: "qrc:/icon/eraser.svg"
            icon.width: 16; icon.height: 16

            DelayButton {
                delay: 1000
                text: qsTr("Confirm")

                onActivated: {
                    datatableModule.datatableClear()
                    progress = 0
                    datatableModuleRootMenu.close()
                }
            }
        }
    }

    // dataplot module
    Menu {
        id: dataplotModuleRootMenu
        property var drawer
        property var rootItem

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Setting\tSwipe")
            icon.source: "qrc:/icon/settings.svg"
            icon.width: 16; icon.height: 16

            onTriggered: dataplotModuleRootMenu.drawer.open()
        }

        MenuItem {
            text: qsTr("Resize\tMB3")
            icon.source: "qrc:/icon/resize.svg"
            icon.width: 16; icon.height: 16

            onTriggered: dataplotModuleRootMenu.rootItem.graphResize()
        }

        MenuItem {
            text: qsTr("Auto Resize")
            icon.source: "qrc:/icon/autoResize.svg"
            icon.width: 16; icon.height: 16
            checkable: true
            checked: dataplotModuleRootMenu.rootItem ? dataplotModuleRootMenu.rootItem.resize : true

            onToggled: {
                dataplotModuleRootMenu.rootItem.resize = checked
                dataplotModuleRootMenu.rootItem.graphResize()
            }
        }

        Menu {
            title: qsTr("Clear")
            icon.source: "qrc:/icon/eraser.svg"
            icon.width: 16; icon.height: 16

            DelayButton {
                delay: 1000
                text: qsTr("Confirm")

                onActivated: {
                    dataplotModuleRootMenu.rootItem.graphClear()
                    progress = 0
                    dataplotModuleRootMenu.close()
                }
            }
        }
    }

    // debug module
    Dialog {
        id: debugModuleErrorDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        title: qsTr("Select a Thread First")
        standardButtons: Dialog.Ok

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
    }

    // diagnostics module
    Menu {
        id: diagnosticsModuleDiagnosticMenu
        property string diagnostic
        property var position

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Copy")
            icon.source: "qrc:/icon/copy.svg"
            icon.width: 16; icon.height: 16

            onTriggered: fileModule.copyToClipboard(diagnosticsModuleDiagnosticMenu.diagnostic)
        }

        MenuItem {
            text: qsTr("View")
            icon.source: "qrc:/icon/eye.svg"
            icon.width: 16; icon.height: 16

            onTriggered: diagnosticsModule.indicatorFill(diagnosticsModuleDiagnosticMenu.position)
        }
    }

    // document module
    Dialog {
        id: documentModuleSaveDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        title: qsTr("Save and Exit")
        standardButtons: Dialog.Yes | Dialog.No
        property string documentUrl
        property string documentName

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAccepted: {
            documentModuleSaveDialog.close()
            documentModule.documentSave(documentUrl)
        }

        Label {
            text: qsTr("Do you want to save changes to " + documentModuleSaveDialog.documentName + "?")
        }
    }

    Menu {
        id: documentModuleEditorMenu
        focus: false
        property var menuSession

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            documentModuleEditorMenu.menuSession = documentModule.menuGet("editor")
            documentModuleEditorMenuRunHereItem.enabled = threadpoolModule.debugging()
        }

        MenuItem {
            text: documentModuleEditorMenu.menuSession ? documentModuleEditorMenu.menuSession.text ? qsTr("Run Selected") : qsTr("Run") : false
            icon.source: "qrc:/icon/play.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                if (documentModuleEditorMenu.menuSession.text) {
                    threadpoolModule.threadStart(documentModuleEditorMenu.menuSession.documentUrl, 0, documentModuleEditorMenu.menuSession.startLine, documentModuleEditorMenu.menuSession.startCharacter, documentModuleEditorMenu.menuSession.endLine, documentModuleEditorMenu.menuSession.endCharacter)
                } else {
                    threadpoolModule.threadStart(documentModuleEditorMenu.menuSession.documentUrl, 0)
                }
            }
        }

        MenuItem {
            text: qsTr("Debug")
            icon.source: "qrc:/icon/bug.svg"
            icon.width: 16; icon.height: 16

            onTriggered: threadpoolModule.threadStart(documentModuleEditorMenu.menuSession.documentUrl, 1)
        }

        MenuSeparator {
        }

        Menu {
            title: qsTr("Folding")
            icon.source: "qrc:/icon/fold.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Contract Top")
                icon.source: "qrc:/icon/collapse.svg"
                icon.width: 16; icon.height: 16

                onTriggered: documentModule.foldContractTop(documentModuleEditorMenu.menuSession.documentUrl)
            }

            MenuItem {
                text: qsTr("Contract Recursively")
                icon.source: "qrc:/icon/collapse.svg"
                icon.width: 16; icon.height: 16

                onTriggered: documentModule.foldContractRecursively(documentModuleEditorMenu.menuSession.documentUrl)
            }

            MenuItem {
                text: qsTr("Expand Recursively")
                icon.source: "qrc:/icon/expand.svg"
                icon.width: 16; icon.height: 16

                onTriggered: documentModule.foldExpandRecursively(documentModuleEditorMenu.menuSession.documentUrl)
            }
        }

        MenuItem {
            text: documentModuleEditorMenu.menuSession ? documentModuleEditorMenu.menuSession.text ? qsTr("Reformat Selected") : qsTr("Reformat") : false
            icon.source: "qrc:/icon/brush.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                if (documentModuleEditorMenu.menuSession.text) {
                    documentModule.rangeFormattingRequest(documentModuleEditorMenu.menuSession.documentUrl, documentModuleEditorMenu.menuSession.startLine, documentModuleEditorMenu.menuSession.startCharacter, documentModuleEditorMenu.menuSession.endLine, documentModuleEditorMenu.menuSession.endCharacter)
                } else {
                    documentModule.formattingRequest(documentModuleEditorMenu.menuSession.documentUrl)
                }
            }
        }

        Menu {
            title: qsTr("Navigation")
            icon.source: "qrc:/icon/location.svg"
            icon.width: 16; icon.height: 16
            enabled: documentModuleEditorMenu.menuSession ? documentModuleEditorMenu.menuSession.navigation : false

            MenuItem {
                text: qsTr("Definition(s)")

                onTriggered: documentModule.definitionRequest(documentModuleEditorMenu.menuSession.documentUrl, documentModuleEditorMenu.menuSession.line, documentModuleEditorMenu.menuSession.character)
            }

            MenuItem {
                text: qsTr("References(s)")

                onTriggered: documentModule.referencesRequest(documentModuleEditorMenu.menuSession.documentUrl, documentModuleEditorMenu.menuSession.line, documentModuleEditorMenu.menuSession.character)
            }

            MenuItem {
                text: qsTr("Implementation(s)")

                onTriggered: documentModule.implementationRequest(documentModuleEditorMenu.menuSession.documentUrl, documentModuleEditorMenu.menuSession.line, documentModuleEditorMenu.menuSession.character)
            }

            MenuItem {
                text: qsTr("Type Definition(s)")

                onTriggered: documentModule.typeDefinitionRequest(documentModuleEditorMenu.menuSession.documentUrl, documentModuleEditorMenu.menuSession.line, documentModuleEditorMenu.menuSession.character)
            }
        }

        MenuSeparator {
        }

        MenuItem {
            text: qsTr("Add Watch")
            icon.source: "qrc:/icon/eye.svg"
            icon.width: 16; icon.height: 16
            enabled: documentModuleEditorMenu.menuSession ? documentModuleEditorMenu.menuSession.text : false
            ToolTip.visible: hovered && !enabled
            ToolTip.text: qsTr("Nothing selected")

            onTriggered: {
                watchModuleExpressionDialog.watchIndex = -1
                watchModuleExpressionDialog.watchUrl = documentModuleEditorMenu.menuSession.documentUrl
                watchModuleExpressionDialog.watchExpression = documentModuleEditorMenu.menuSession.text
                watchModuleExpressionDialog.open()
            }
        }

        MenuItem {
            id: documentModuleEditorMenuRunHereItem
            text: qsTr("Run Here")
            icon.source: "qrc:/icon/debugContinue.svg"
            icon.width: 16; icon.height: 16
            enabled: false
            ToolTip.visible: hovered && !enabled
            ToolTip.text: qsTr("No debug sessions")

            onTriggered: debugModule.stateSet("", 6)
        }

        MenuItem {
            text: qsTr("Assembly")
            icon.source: "qrc:/icon/assembly.svg"
            icon.width: 16; icon.height: 16

            onTriggered: documentModule.assemblyToggle(documentModuleEditorMenu.menuSession.documentUrl, !documentModuleEditorMenu.menuSession.assembly)
        }

        MenuSeparator {
        }

        Menu {
            title: qsTr("Open In")
            icon.source: "qrc:/icon/open.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Explorer")
                icon.source: "qrc:/icon/folder.svg"
                icon.width: 16; icon.height: 16

                onTriggered: fileModule.fileOpenInExplorer(documentModuleEditorMenu.menuSession.documentUrl)
            }

            MenuItem {
                text: qsTr("Application")
                icon.source: "qrc:/icon/apps.svg"
                icon.width: 16; icon.height: 16

                onTriggered: fileModule.fileOpenInApplication(documentModuleEditorMenu.menuSession.documentUrl)
            }
        }

        MenuItem {
            text: qsTr("Property")
            icon.source: "qrc:/icon/property.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                fileModulePropertyDialog.documentUrl = documentModuleEditorMenu.menuSession.documentUrl
                fileModulePropertyDialog.open()
            }
        }
    }

    ToolTip {
        id: documentModuleCompletionToolTip
        parent: Overlay.overlay
        closePolicy: Popup.CloseOnPressOutside | Popup.CloseOnReleaseOutside
        property point position
        property var completionWidget
        property int typed

        onOpened: {
            mainWindow.overlayFlagSet(false, false)
            widgetCount += 1
        }
        onClosed: {
            widgetCount -= 1
            documentModuleCompletionDetailTimer.stop()
            completionWidget.completionHide()
        }
        onAboutToShow: {
            position = Overlay.overlay.mapFromGlobal(position)
            x = position.x - 30
            y = position.y
            documentModuleCompletionDetailToolTip.open()
            documentModuleCompletionDetailTimer.restart()
        }

        contentItem: TableView {
            id: documentModuleCompletionTableView
            anchors.fill: parent
            anchors.margins: 6
            implicitWidth: Math.max(idealWidth, 200); implicitHeight: Math.min(idealHeight, 150)
            alternatingRows: false
            clip: true
            editTriggers: TableView.NoEditTriggers
            flickableDirection: Flickable.VerticalFlick
            property int idealWidth; property int idealHeight
            property int hoveredRow: -1
            property int selectedRow: -1

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                palette {
                    mid: global.stroke
                    dark: global.strokePressed
                }
            }

            delegate: Item {
                implicitWidth: documentModuleCompletionTableView.width; implicitHeight: textMetrics.height + 4
                required property int row

                Rectangle {
                    anchors.fill: parent
                    radius: 6
                    color: global.backHover
                    opacity: hoverHandler.hovered ? 1 : 0
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 150
                        }
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    radius: 6
                    color: documentModuleCompletionTableView.selectedRow === row ? global.backSelected : "transparent"
                }

                TextMetrics {
                    id: textMetrics
                    font: documentModuleCompletionToolTip.font
                    text: model.display
                }

                RowLayout {
                    anchors.fill: parent
                    spacing: 0

                    Item {
                        Layout.preferredWidth: 24; Layout.preferredHeight: 24

                        IconImage {
                            anchors.centerIn: parent
                            width: 16; height: 16
                            source: model.decoration
                            color: global.fore
                        }
                    }

                    Label {
                        font: documentModuleCompletionToolTip.font
                        horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                        text: model.display ? "<span style='color: " + global.brandBack + ";'>" + model.display.substring(0, documentModuleCompletionToolTip.typed) + "</span>" + model.display.substring(documentModuleCompletionToolTip.typed) : ""
                        textFormat: Text.RichText
                        elide: Text.ElideRight
                        Layout.fillWidth: true; Layout.preferredHeight: 24
                    }
                }

                HoverHandler {
                    id: hoverHandler

                    onPointChanged: documentModuleCompletionTableView.hoveredRow = row
                    onHoveredChanged: {
                        if (!hovered) {
                            documentModuleCompletionTableView.hoveredRow = -1
                            documentModuleCompletionDetailTimer.restart()
                        }
                    }
                }

                TapHandler {
                    acceptedButtons: Qt.LeftButton

                    onTapped: documentModuleCompletionTableView.selectedRow = row
                    onDoubleTapped: documentModuleCompletionToolTip.completionWidget.textReplace()
                }

                Component.onCompleted: {
                    documentModuleCompletionTableView.idealWidth = Math.max(24 + textMetrics.width + 4 + 10, documentModuleCompletionTableView.idealWidth)
                    documentModuleCompletionTableView.idealHeight = textMetrics.height + 4 + documentModuleCompletionTableView.idealHeight
                }
            }

            onHoveredRowChanged: documentModuleCompletionDetailTimer.restart()

            onSelectedRowChanged: {
                positionViewAtRow(selectedRow, TableView.Contain, 0, Qt.rect(0, 0, 0, 0))
                documentModuleCompletionDetailTimer.restart()
            }

            Timer {
                id: documentModuleCompletionDetailTimer
                interval: 150

                onTriggered: {
                    var interestRow
                    if (documentModuleCompletionTableView.hoveredRow !== -1) {
                        interestRow = documentModuleCompletionTableView.hoveredRow
                    } else {
                        interestRow = documentModuleCompletionTableView.selectedRow
                    }
                    documentModuleCompletionToolTip.completionWidget.detailReload(interestRow)
                    const index = documentModuleCompletionTableView.index(interestRow, 0);
                    const item = documentModuleCompletionTableView.itemAtIndex(index);
                    if (item) {
                        let idealY = item.mapToItem(documentModuleCompletionTableView, 0, 0).y
                        idealY = Math.max(0, idealY)
                        idealY = Math.min(documentModuleCompletionTableView.height - item.height, idealY)
                        documentModuleCompletionDetailToolTip.y = idealY - 6
                    }
                }
            }

            function completionPrev() {
                if (selectedRow > 0) {
                    selectedRow = selectedRow - 1
                }
                // else {
                //     selectedRow = model.rowCount() - 1
                // }
            }

            function completionNext() {
                if (selectedRow < model.rowCount() - 1) {
                    selectedRow = selectedRow + 1
                }
                // else {
                //     selectedRow = 0
                // }
            }

            Connections {
                target: documentModuleCompletionTableView.model

                function onModelReset() {
                    documentModuleCompletionTableView.idealWidth = 0
                    documentModuleCompletionTableView.idealHeight = 0
                    documentModuleCompletionDetailTimer.restart()
                }
            }
        }

        ToolTip {
            id: documentModuleCompletionDetailToolTip
            x: documentModuleCompletionToolTip.width - 5

            contentItem: TableView {
                id: documentModuleCompletionDetailTableView
                anchors.fill: parent
                anchors.margins: 6
                implicitWidth: idealWidth; implicitHeight: idealHeight
                alternatingRows: false
                clip: true
                editTriggers: TableView.NoEditTriggers
                flickableDirection: Flickable.VerticalFlick
                property int idealWidth; property int idealHeight

                delegate: Item {
                    implicitWidth: documentModuleCompletionDetailTableView.width; implicitHeight: textMetrics.height + 4

                    TextMetrics {
                        id: textMetrics
                        font: documentModuleCompletionToolTip.font
                        text: model.display
                    }

                    Label {
                        id: documentModuleCompletionDetailLabel
                        anchors.fill: parent
                        font: documentModuleCompletionToolTip.font
                        horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                        text: model.display
                        elide: Text.ElideRight
                    }

                    Component.onCompleted: {
                        documentModuleCompletionDetailTableView.idealWidth = Math.max(textMetrics.width + 4, documentModuleCompletionDetailTableView.idealWidth)
                        documentModuleCompletionDetailTableView.idealHeight = textMetrics.height + 4 + documentModuleCompletionDetailTableView.idealHeight
                    }
                }

                Connections {
                    target: documentModuleCompletionDetailTableView.model

                    function onModelReset() {
                        documentModuleCompletionDetailTableView.idealWidth = 0
                        documentModuleCompletionDetailTableView.idealHeight = 0
                    }
                }
            }
        }
    }

    ToolTip {
        id: documentModuleDwellToolTip
        parent: Overlay.overlay
        closePolicy: Popup.CloseOnPressOutside | Popup.CloseOnReleaseOutside
        property point position
        property var dwellWidget
        property var codeActions
        property var suggestions

        onOpened: {
            mainWindow.overlayFlagSet(false, false)
            widgetCount += 1
        }
        onClosed: {
            widgetCount -= 1
            dwellWidget.dwellHide()
        }
        onAboutToShow: {
            position = Overlay.overlay.mapFromGlobal(position)
            x = position.x + 10
            y = position.y + 10
        }

        contentItem: ColumnLayout {

            ScrollView {
                visible: documentModuleDwellDiagnosticTextArea.length > 0
                Layout.minimumWidth: 400; Layout.maximumWidth: 800

                TextArea {
                    id: documentModuleDwellDiagnosticTextArea
                    background: null
                    readOnly: true
                    textFormat: TextEdit.RichText
                    verticalAlignment: TextEdit.AlignTop
                    wrapMode: Text.Wrap

                    HoverHandler {
                        id: diagnosticHoverHandler
                        cursorShape: documentModuleDwellDiagnosticTextArea.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
                    }

                    TapHandler {
                        acceptedButtons: Qt.LeftButton
                        onTapped: {
                            if (documentModuleDwellDiagnosticTextArea.hoveredLink) {
                                documentModuleDwellToolTip.dwellWidget.linkClick(documentModuleDwellDiagnosticTextArea.hoveredLink)
                            }
                        }
                    }
                }
            }

            ScrollView {
                visible: documentModuleDwellHoverTextArea.length > 0
                Layout.minimumWidth: 400; Layout.maximumWidth: 800

                TextArea {
                    id: documentModuleDwellHoverTextArea
                    background: null
                    readOnly: true
                    textFormat: TextEdit.MarkdownText
                    verticalAlignment: TextEdit.AlignTop
                    wrapMode: Text.Wrap

                    HoverHandler {
                        id: dwellHoverHandler
                        cursorShape: documentModuleDwellHoverTextArea.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
                    }

                    ToolTip {
                        visible: documentModuleDwellHoverTextArea.hoveredLink
                        text: "Click to open link"
                        x: dwellHoverHandler.point.position.x + 10
                        y: dwellHoverHandler.point.position.y + 10
                    }

                    TapHandler {
                        acceptedButtons: Qt.LeftButton
                        onTapped: {
                            if (documentModuleDwellHoverTextArea.hoveredLink) {
                                Qt.openUrlExternally(documentModuleDwellHoverTextArea.hoveredLink)
                            }
                        }
                    }
                }
            }
        }

        Menu {
            id: documentModuleDwellCodeActionMenu
            x: documentModuleDwellToolTip.width - 5; y: -8

            onAboutToShow: {
                for (var i = documentModuleDwellCodeActionMenu.count - 1; i >= 0; --i) {
                    var item = documentModuleDwellCodeActionMenu.itemAt(i)
                    documentModuleDwellCodeActionMenu.removeItem(item)
                    item.destroy()
                }
                if (documentModuleDwellToolTip.codeActions.length) {
                    for (let i = 0; i < documentModuleDwellToolTip.codeActions.length; ++i) {
                        const menuItem = documentModuleDwellCodeActionMenuComponent.createObject(
                            null,
                            {
                                text: documentModuleDwellToolTip.codeActions[i]["title"],
                                codeAction: documentModuleDwellToolTip.codeActions[i]["edit"]
                            }
                        )
                        documentModuleDwellCodeActionMenu.addItem(menuItem)
                    }
                } else {
                    const menuItem = documentModuleDwellCodeActionMenuComponent.createObject(
                        null,
                        {
                            text: qsTr("No fixes available")
                        }
                    )
                    documentModuleDwellCodeActionMenu.addItem(menuItem)
                }
            }

            Component {
                id: documentModuleDwellCodeActionMenuComponent

                MenuItem {
                    property var codeAction

                    onTriggered: {
                        documentModuleDwellToolTip.dwellWidget.codeActionAccept(codeAction)
                        documentModuleDwellToolTip.close()
                    }

                    ToolTip.visible: hovered
                    ToolTip.text: text
                }
            }
        }

        Menu {
            id: documentModuleDwellSuggestionMenu
            x: documentModuleDwellToolTip.width - 5; y: -8

            MenuItem {
                text: documentModuleDwellToolTip.suggestions ? documentModuleDwellToolTip.suggestions[0] : ""
                visible: text

                onTriggered: {
                    documentModuleDwellToolTip.dwellWidget.suggestionAccept(text)
                    documentModuleDwellToolTip.close()
                }
            }

            MenuItem {
                text: documentModuleDwellToolTip.suggestions ? documentModuleDwellToolTip.suggestions[1] : ""
                visible: text

                onTriggered: {
                    documentModuleDwellToolTip.dwellWidget.suggestionAccept(text)
                    documentModuleDwellToolTip.close()
                }
            }

            MenuItem {
                text: documentModuleDwellToolTip.suggestions ? documentModuleDwellToolTip.suggestions[2] : ""
                visible: text

                onTriggered: {
                    documentModuleDwellToolTip.dwellWidget.suggestionAccept(text)
                    documentModuleDwellToolTip.close()
                }
            }

            MenuItem {
                text: documentModuleDwellToolTip.suggestions ? documentModuleDwellToolTip.suggestions[3] : ""
                visible: text

                onTriggered: {
                    documentModuleDwellToolTip.dwellWidget.suggestionAccept(text)
                    documentModuleDwellToolTip.close()
                }
            }

            MenuItem {
                text: documentModuleDwellToolTip.suggestions ? documentModuleDwellToolTip.suggestions[4] : ""
                visible: text

                onTriggered: {
                    documentModuleDwellToolTip.dwellWidget.suggestionAccept(text)
                    documentModuleDwellToolTip.close()
                }
            }
        }
    }

    ToolTip {
        id: documentModuleNavigationToolTip
        parent: Overlay.overlay
        closePolicy: Popup.CloseOnPressOutside | Popup.CloseOnReleaseOutside
        property point position
        property var navigationWidget

        onOpened: {
            mainWindow.overlayFlagSet(false, false)
            widgetCount += 1
        }
        onClosed: {
            widgetCount -= 1
            documentModuleNavigationDetailTimer.stop()
            navigationWidget.navigationHide()
        }
        onAboutToShow: {
            position = Overlay.overlay.mapFromGlobal(position)
            x = position.x - 30
            y = position.y
            documentModuleNavigationDetailToolTip.open()
            documentModuleNavigationDetailTimer.restart()
        }

        contentItem: TableView {
            id: documentModuleNavigationTableView
            anchors.fill: parent
            anchors.margins: 6
            implicitWidth: Math.max(idealWidth, 200); implicitHeight: Math.min(idealHeight, 150)
            alternatingRows: false
            clip: true
            editTriggers: TableView.NoEditTriggers
            flickableDirection: Flickable.VerticalFlick
            property int idealWidth; property int idealHeight
            property int hoveredRow: -1
            property int selectedRow: -1

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            delegate: Item {
                implicitWidth: documentModuleNavigationTableView.width; implicitHeight: textMetrics.height + 4
                required property int row

                Rectangle {
                    anchors.fill: parent
                    radius: 6
                    color: global.backHover
                    opacity: hoverHandler.hovered ? 1 : 0
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 150
                        }
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    radius: 6
                    color: documentModuleNavigationTableView.selectedRow === row ? global.backSelected : "transparent"
                }

                TextMetrics {
                    id: textMetrics
                    font: documentModuleNavigationToolTip.font
                    text: model.display
                }

                RowLayout {
                    anchors.fill: parent
                    spacing: 0

                    Label {
                        font: documentModuleNavigationToolTip.font
                        horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                        text: model.display
                        elide: Text.ElideRight
                        Layout.fillWidth: true; Layout.preferredHeight: 24
                    }
                }

                HoverHandler {
                    id: hoverHandler

                    onPointChanged: documentModuleNavigationTableView.hoveredRow = row
                    onHoveredChanged: {
                        if (!hovered) {
                            documentModuleNavigationTableView.hoveredRow = -1
                            documentModuleNavigationDetailTimer.restart()
                        }
                    }
                }

                TapHandler {
                    acceptedButtons: Qt.LeftButton

                    onTapped: documentModuleNavigationTableView.selectedRow = row
                    onDoubleTapped: documentModuleNavigationToolTip.navigationWidget.indicatorInsert()
                }

                Component.onCompleted: {
                    documentModuleNavigationTableView.idealWidth = Math.max(24 + textMetrics.width + 4 + 10, documentModuleNavigationTableView.idealWidth)
                    documentModuleNavigationTableView.idealHeight = textMetrics.height + 4 + documentModuleNavigationTableView.idealHeight
                }
            }

            onHoveredRowChanged: documentModuleNavigationDetailTimer.restart()

            onSelectedRowChanged: {
                positionViewAtRow(selectedRow, TableView.Contain, 0, Qt.rect(0, 0, 0, 0))
                documentModuleNavigationDetailTimer.restart()
            }

            Timer {
                id: documentModuleNavigationDetailTimer
                interval: 150

                onTriggered: {
                    var interestRow
                    if (documentModuleNavigationTableView.hoveredRow !== -1) {
                        interestRow = documentModuleNavigationTableView.hoveredRow
                    } else {
                        interestRow = documentModuleNavigationTableView.selectedRow
                    }
                    documentModuleNavigationToolTip.navigationWidget.detailReload(interestRow)
                    const index = documentModuleNavigationTableView.index(interestRow, 0);
                    const item = documentModuleNavigationTableView.itemAtIndex(index);
                    if (item) {
                        let idealY = item.mapToItem(documentModuleNavigationTableView, 0, 0).y
                        idealY = Math.max(0, idealY)
                        idealY = Math.min(documentModuleNavigationTableView.height - item.height, idealY)
                        documentModuleNavigationDetailToolTip.y = idealY - 6
                    }
                }
            }

            function navigationPrev() {
                if (selectedRow > 0) {
                    selectedRow = selectedRow - 1
                }
                // else {
                //     selectedRow = model.rowCount() - 1
                // }
            }

            function navigationNext() {
                if (selectedRow < model.rowCount() - 1) {
                    selectedRow = selectedRow + 1
                }
                // else {
                //     selectedRow = 0
                // }
            }

            Connections {
                target: documentModuleNavigationTableView.model

                function onModelReset() {
                    documentModuleNavigationTableView.idealWidth = 0
                    documentModuleNavigationTableView.idealHeight = 0
                    documentModuleNavigationDetailTimer.restart()
                }
            }
        }

        ToolTip {
            id: documentModuleNavigationDetailToolTip
            x: documentModuleNavigationToolTip.width - 5

            contentItem: Label {
                id: documentModuleNavigationDetailLabel
                font: documentModuleNavigationToolTip.font
                textFormat: Text.RichText
            }
        }
    }

    ToolTip {
        id: documentModulePositionTooltip
        parent: Overlay.overlay
        x: position.x + 10; y: position.y + 10
        closePolicy: Popup.CloseOnPressOutside | Popup.CloseOnReleaseOutside
        property point position

        Behavior on x {
            enabled: documentModulePositionTooltip.visible
            NumberAnimation {
                duration: 50
                easing.type: Easing.Linear
            }
        }
        Behavior on y {
            enabled: documentModulePositionTooltip.visible
            NumberAnimation {
                duration: 50
                easing.type: Easing.Linear
            }
        }
    }

    ToolTip {
        id: documentModuleSignatureToolTip
        parent: Overlay.overlay
        closePolicy: Popup.CloseOnPressOutside | Popup.CloseOnReleaseOutside
        property point position
        property var signatureWidget

        onOpened: {
            mainWindow.overlayFlagSet(false, false)
            widgetCount += 1
        }
        onClosed: {
            widgetCount -= 1
            signatureWidget.signatureHide()
        }
        onAboutToShow: {
            position = Overlay.overlay.mapFromGlobal(position)
            x = position.x - 6
            y = position.y - documentModuleSignatureToolTip.implicitHeight
        }

        contentItem: Label {
            id: documentModuleSignatureLabel
            textFormat: Text.RichText
        }
    }

    // explorer module
    Dialog {
        id: explorerModuleFileNewDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        title: qsTr("New File")
        standardButtons: Dialog.Ok | Dialog.Cancel
        property string documentUrl

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            explorerModuleFileNameTextField.clear()
            explorerModuleFileNameTextField.forceActiveFocus()
        }
        onAccepted: fileModule.fileNew(explorerModuleFileNewDialog.documentUrl + "/" + explorerModuleFileNameTextField.text)

        TextField {
            id: explorerModuleFileNameTextField
            width: parent.width
            placeholderText: qsTr("Enter file name:")

            onAccepted: explorerModuleFileNewDialog.accept()
            Keys.onEscapePressed: explorerModuleFileNewDialog.reject()
        }
    }

    Dialog {
        id: explorerModuleFolderNewDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        title: qsTr("New Folder")
        standardButtons: Dialog.Ok | Dialog.Cancel
        property string documentUrl

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            explorerModuleFolderNameTextField.clear()
            explorerModuleFolderNameTextField.forceActiveFocus()
        }
        onAccepted: fileModule.fileNew(explorerModuleFolderNewDialog.documentUrl + "/" + explorerModuleFolderNameTextField.text)

        TextField {
            id: explorerModuleFolderNameTextField
            width: parent.width
            placeholderText: qsTr("Enter folder name:")

            onAccepted: explorerModuleFolderNewDialog.accept()
            Keys.onEscapePressed: explorerModuleFolderNewDialog.reject()
        }
    }

    Menu {
        id: explorerModuleFileMenu
        property bool gitEnabled
        property bool gitUntracked
        property bool gitIgnored
        property string documentUrl
        property var treeView

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Run")
            icon.source: "qrc:/icon/play.svg"
            icon.width: 16; icon.height: 16

            onTriggered: explorerModule.scriptRun(explorerModuleFileMenu.documentUrl)
        }

        MenuItem {
            text: qsTr("Debug")
            icon.source: "qrc:/icon/bug.svg"
            icon.width: 16; icon.height: 16

            onTriggered: explorerModule.scriptDebug(explorerModuleFileMenu.documentUrl)
        }

        MenuSeparator {
        }

        Menu {
            title: qsTr("Open In")
            icon.source: "qrc:/icon/open.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Explorer")
                icon.source: "qrc:/icon/folder.svg"
                icon.width: 16; icon.height: 16

                onTriggered: fileModule.fileOpenInExplorer(explorerModuleFileMenu.documentUrl)
            }

            MenuItem {
                text: qsTr("Application")
                icon.source: "qrc:/icon/apps.svg"
                icon.width: 16; icon.height: 16

                onTriggered: fileModule.fileOpenInApplication(explorerModuleFileMenu.documentUrl)
            }
        }

        MenuItem {
            text: qsTr("Rename")
            icon.source: "qrc:/icon/rename.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                fileModulePropertyDialog.documentUrl = explorerModuleFileMenu.documentUrl
                fileModulePropertyDialog.open()
                fileModulePropertyNameTextField.forceActiveFocus()
                fileModulePropertyNameTextField.selectAll()
            }
        }

        Menu {
            title: qsTr("Delete")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16

            DelayButton {
                delay: 1000
                text: qsTr("Confirm")

                onActivated: {
                    fileModule.fileDelete(explorerModuleFileMenu.documentUrl)
                    progress = 0
                    explorerModuleFileMenu.close()
                }
            }
        }

        MenuSeparator {
        }

        Menu {
            title: qsTr("Git")
            enabled: explorerModuleFileMenu.gitEnabled
            icon.source: "qrc:/icon/fileTypeGit.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Add")
                enabled: explorerModuleFileMenu.gitUntracked
                icon.source: "qrc:/icon/add.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    gitModule.gitAdd(explorerModuleFileMenu.documentUrl)
                }
            }

            MenuItem {
                text: qsTr("Reset")
                enabled: !explorerModuleFileMenu.gitUntracked && !explorerModuleFileMenu.gitIgnored
                icon.source: "qrc:/icon/reset.svg"
                icon.width: 16; icon.height: 16

                onTriggered: gitModule.gitReset(explorerModuleFileMenu.documentUrl)
            }

            MenuItem {
                text: explorerModuleFileMenu.gitIgnored ? qsTr("Unignore") : qsTr("Ignore")
                icon.source: "qrc:/icon/prohibited.svg"
                icon.width: 16; icon.height: 16

                onTriggered: gitModule.gitIgnore(explorerModuleFileMenu.documentUrl, !explorerModuleFileMenu.gitIgnored)
            }
        }

        MenuSeparator {
        }

        MenuItem {
            text: qsTr("Toggle hidden")
            icon.source: "qrc:/icon/hide.svg"
            icon.width: 16; icon.height: 16

            onTriggered: explorerModule.toggleHidden()
        }

        Menu {
            title: qsTr("Folding")
            icon.source: "qrc:/icon/fold.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Collapse All")
                icon.source: "qrc:/icon/collapse.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    for (let i = 0; i < explorerModuleFileMenu.treeView.rows; ++i) {
                        explorerModuleFileMenu.treeView.collapseRecursively(i)
                    }
                }
            }

            MenuItem {
                text: qsTr("Expand All")
                icon.source: "qrc:/icon/expand.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    for (let i = 0; i < explorerModuleFileMenu.treeView.rows; ++i) {
                        explorerModuleFileMenu.treeView.expandRecursively(i)
                    }
                }
            }
        }
    }

    Menu {
        id: explorerModuleFolderMenu
        property bool gitEnabled
        property string documentUrl
        property var treeView

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Open In Explorer")
            icon.source: "qrc:/icon/open.svg"
            icon.width: 16; icon.height: 16

            onTriggered: fileModule.fileOpenInExplorer(explorerModuleFolderMenu.documentUrl)
        }

        Menu {
            title: qsTr("New")
            icon.source: "qrc:/icon/add.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("File")
                icon.source: "qrc:/icon/document.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    explorerModuleFileNewDialog.documentUrl = explorerModuleFolderMenu.documentUrl
                    explorerModuleFileNewDialog.open()
                }
            }

            MenuItem {
                text: qsTr("Folder")
                icon.source: "qrc:/icon/folder.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    explorerModuleFolderNewDialog.documentUrl = explorerModuleFolderMenu.documentUrl
                    explorerModuleFolderNewDialog.open()
                }
            }
        }

        MenuItem {
            text: qsTr("Rename")
            icon.source: "qrc:/icon/rename.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                fileModulePropertyDialog.documentUrl = explorerModuleFolderMenu.documentUrl
                fileModulePropertyDialog.open()
                fileModulePropertyNameTextField.forceActiveFocus()
                fileModulePropertyNameTextField.selectAll()
            }
        }

        Menu {
            title: qsTr("Delete")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16

            DelayButton {
                delay: 1000
                text: qsTr("Confirm")

                onActivated: {
                    fileModule.fileDelete(explorerModuleFolderMenu.documentUrl)
                    progress = 0
                    explorerModuleFolderMenu.close()
                }
            }
        }

        MenuSeparator {
        }

        Menu {
            title: qsTr("Git")
            enabled: explorerModuleFolderMenu.gitEnabled
            icon.source: "qrc:/icon/fileTypeGit.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Add")
                icon.source: "qrc:/icon/add.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    gitModule.gitAdd(explorerModuleFolderMenu.documentUrl)
                }
            }

            MenuItem {
                text: qsTr("Reset")
                icon.source: "qrc:/icon/reset.svg"
                icon.width: 16; icon.height: 16

                onTriggered: gitModule.gitReset(explorerModuleFolderMenu.documentUrl)
            }

            MenuItem {
                text: qsTr("Ignore")
                icon.source: "qrc:/icon/prohibited.svg"
                icon.width: 16; icon.height: 16

                onTriggered: gitModule.gitIgnore(explorerModuleFolderMenu.documentUrl, true)
            }
        }

        MenuSeparator {
        }

        MenuItem {
            text: qsTr("Toggle hidden")
            icon.source: "qrc:/icon/hide.svg"
            icon.width: 16; icon.height: 16

            onTriggered: explorerModule.toggleHidden()
        }

        Menu {
            title: qsTr("Folding")
            icon.source: "qrc:/icon/fold.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Collapse All")
                icon.source: "qrc:/icon/collapse.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    for (let i = 0; i < explorerModuleFolderMenu.treeView.rows; ++i) {
                        explorerModuleFolderMenu.treeView.collapseRecursively(i)
                    }
                }
            }

            MenuItem {
                text: qsTr("Expand All")
                icon.source: "qrc:/icon/expand.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    for (let i = 0; i < explorerModuleFolderMenu.treeView.rows; ++i) {
                        explorerModuleFolderMenu.treeView.expandRecursively(i)
                    }
                }
            }
        }
    }

    Menu {
        id: explorerModuleRootMenu
        property bool gitEnabled
        property string documentUrl
        property var treeView

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Open In Explorer")
            icon.source: "qrc:/icon/open.svg"
            icon.width: 16; icon.height: 16

            onTriggered: fileModule.fileOpenInExplorer(explorerModuleRootMenu.documentUrl)
        }

        Menu {
            title: qsTr("New")
            icon.source: "qrc:/icon/add.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("File")
                icon.source: "qrc:/icon/document.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    explorerModuleFileNewDialog.documentUrl = explorerModuleRootMenu.documentUrl
                    explorerModuleFileNewDialog.open()
                }
            }

            MenuItem {
                text: qsTr("Folder")
                icon.source: "qrc:/icon/folder.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    explorerModuleFolderNewDialog.documentUrl = explorerModuleRootMenu.documentUrl
                    explorerModuleFolderNewDialog.open()
                }
            }
        }

        MenuSeparator {
        }

        Menu {
            title: qsTr("Git")
            enabled: explorerModuleFolderMenu.gitEnabled
            icon.source: "qrc:/icon/fileTypeGit.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Add all")
                icon.source: "qrc:/icon/add.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    gitModule.gitAddAll()
                }
            }

            MenuItem {
                text: qsTr("Reset all")
                icon.source: "qrc:/icon/reset.svg"
                icon.width: 16; icon.height: 16

                onTriggered: gitModule.gitResetAll()
            }
        }

        MenuSeparator {
        }

        MenuItem {
            text: qsTr("Toggle hidden")
            icon.source: "qrc:/icon/hide.svg"
            icon.width: 16; icon.height: 16

            onTriggered: explorerModule.toggleHidden()
        }

        Menu {
            title: qsTr("Folding")
            icon.source: "qrc:/icon/fold.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Collapse All")
                icon.source: "qrc:/icon/collapse.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    for (let i = 0; i < explorerModuleRootMenu.treeView.rows; ++i) {
                        explorerModuleRootMenu.treeView.collapseRecursively(i)
                    }
                }
            }

            MenuItem {
                text: qsTr("Expand All")
                icon.source: "qrc:/icon/expand.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    for (let i = 0; i < explorerModuleRootMenu.treeView.rows; ++i) {
                        explorerModuleRootMenu.treeView.expandRecursively(i)
                    }
                }
            }
        }
    }

    // file module
    Dialog {
        id: fileModulePropertyDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        title: qsTr("File Property")
        standardButtons: Dialog.Ok | Dialog.Cancel
        property string documentUrl
        property var infoSession

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            fileModulePropertyDialog.infoSession = fileModule.fileInfo(fileModulePropertyDialog.documentUrl)
            fileModulePropertyImage.source = fileModulePropertyDialog.infoSession.source
            fileModulePropertyNameTextField.text = fileModulePropertyDialog.infoSession.baseName
            fileModulePropertySizeLabel.text = fileModulePropertyDialog.infoSession.size
            fileModulePropertyAbsolutePathLabel.text = fileModulePropertyDialog.infoSession.absolutePath
            fileModulePropertyBirthTimeLabel.text = fileModulePropertyDialog.infoSession.birthTime
            fileModulePropertyLastModifiedLabel.text = fileModulePropertyDialog.infoSession.lastModified
            fileModulePropertyLastReadLabel.text = fileModulePropertyDialog.infoSession.lastRead
            fileModulePropertyReadableCheckBox.checked = fileModulePropertyDialog.infoSession.readable
            fileModulePropertyWritableCheckBox.checked = fileModulePropertyDialog.infoSession.writable
            fileModulePropertyHiddenCheckBox.checked = fileModulePropertyDialog.infoSession.hidden
        }

        onAccepted: {
            if (fileModulePropertyNameTextField.text !== fileModulePropertyDialog.infoSession.baseName) {
                fileModule.fileRename(fileModulePropertyDialog.documentUrl, fileModulePropertyNameTextField.text)
            }
            if (fileModulePropertyWritableCheckBox.checked !== fileModulePropertyDialog.infoSession.writable) {
                fileModule.fileWritable(fileModulePropertyDialog.documentUrl, fileModulePropertyWritableCheckBox.checked)
            }
        }

        ColumnLayout {
            width: parent.width

            RowLayout {
                Layout.preferredHeight: 64

                Image {
                    id: fileModulePropertyImage
                    sourceSize.width: 48
                    sourceSize.height: 48
                    horizontalAlignment: Image.AlignLeft
                    fillMode: Image.PreserveAspectFit
                    Layout.preferredWidth: 150
                }

                TextField {
                    id: fileModulePropertyNameTextField
                    placeholderText: qsTr("Enter new name:")
                    Layout.fillWidth: true

                    onAccepted: fileModulePropertyDialog.accept()
                    Keys.onEscapePressed: fileModulePropertyDialog.reject()
                }
            }

            Rectangle {
                color: global.stroke
                Layout.fillWidth: true; Layout.preferredHeight: 1
            }

            RowLayout {
                Layout.preferredHeight: 32

                Label {
                    text: qsTr("Absolute Path:")
                    Layout.preferredWidth: 150
                }

                Label {
                    id: fileModulePropertyAbsolutePathLabel
                }
            }

            RowLayout {
                Layout.preferredHeight: 32

                Label {
                    text: qsTr("Size:")
                    Layout.preferredWidth: 150
                }

                Label {
                    id: fileModulePropertySizeLabel
                }
            }

            Rectangle {
                color: global.stroke
                Layout.fillWidth: true; Layout.preferredHeight: 1
            }

            RowLayout {
                Layout.preferredHeight: 32

                Label {
                    text: qsTr("Birth Time:")
                    Layout.preferredWidth: 150
                }

                Label {
                    id: fileModulePropertyBirthTimeLabel
                }
            }

            RowLayout {
                Layout.preferredHeight: 32

                Label {
                    text: qsTr("Last Modified:")
                    Layout.preferredWidth: 150
                }

                Label {
                    id: fileModulePropertyLastModifiedLabel
                }
            }

            RowLayout {
                Layout.preferredHeight: 32

                Label {
                    text: qsTr("Last Read:")
                    Layout.preferredWidth: 150
                }

                Label {
                    id: fileModulePropertyLastReadLabel
                }
            }

            Rectangle {
                color: global.stroke
                Layout.fillWidth: true; Layout.preferredHeight: 1
            }

            RowLayout {
                Layout.preferredHeight: 32

                Label {
                    text: qsTr("Attributes:")
                    Layout.preferredWidth: 150
                }

                CheckBox {
                    id: fileModulePropertyReadableCheckBox
                    text: qsTr("Readable")
                }

                CheckBox {
                    id: fileModulePropertyWritableCheckBox
                    text: qsTr("Writable")
                }

                CheckBox {
                    id: fileModulePropertyHiddenCheckBox
                    text: qsTr("Hidden")
                }
            }
        }
    }

    // log module
    Dialog {
        id: logModuleHeightDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        title: qsTr("Set Max Line Count")
        standardButtons: Dialog.Ok | Dialog.Cancel

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            logModuleHeightSpinBox.value = logModule.heightGet()
            logModuleHeightSpinBox.forceActiveFocus()
        }
        onAccepted: logModule.heightSet(logModuleHeightSpinBox.value)

        SpinBox {
            id: logModuleHeightSpinBox
            width: parent.width
            from: 1000
            to: 10000
            stepSize: 1000

            Keys.onReturnPressed: logModuleHeightDialog.accept()
            Keys.onEnterPressed: logModuleHeightDialog.accept()
            Keys.onEscapePressed: logModuleHeightDialog.reject()
        }
    }

    Menu {
        id: logModuleLinkMenu
        property url url

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Copy URL")
            icon.source: "qrc:/icon/copy.svg"
            icon.width: 16; icon.height: 16

            onTriggered: fileModule.copyToClipboard(logModuleLinkMenu.url)
        }

        Menu {
            title: qsTr("Open In")
            icon.source: "qrc:/icon/open.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Explorer")
                icon.source: "qrc:/icon/folder.svg"
                icon.width: 16; icon.height: 16

                onTriggered: fileModule.fileOpenInExplorer(logModuleLinkMenu.url)
            }

            MenuItem {
                text: qsTr("Application")
                icon.source: "qrc:/icon/apps.svg"
                icon.width: 16; icon.height: 16

                onTriggered: fileModule.fileOpenInApplication(logModuleLinkMenu.url)
            }
        }
    }

    // menu module
    Menu {
        id: menuModuleFileMenu
        implicitWidth: 300

        onOpened: {
            mainWindow.overlayFlagSet(false, false)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            id: menuModuleFileMenuOpenWorkspaceItem

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                IconImage {
                    source: "qrc:/icon/open.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    color: global.fore
                }

                Label {
                    text: qsTr("Open Workspace")
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: "Ctrl+O"
                }
            }

            Shortcut {
                sequence: "Ctrl+O"
                onActivated: menuModuleFileMenuOpenWorkspaceItem.triggered()
            }

            onTriggered: Qt.callLater(() => mainWindow.workspaceOpen())
        }

        MenuItem {
            id: menuModuleFileMenuSaveWorkspaceItem

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                IconImage {
                    source: "qrc:/icon/save.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    color: global.fore
                }

                Label {
                    text: qsTr("Save Workspace")
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: "Ctrl+S"
                }
            }

            Shortcut {
                sequence: "Ctrl+S"
                onActivated: menuModuleFileMenuSaveWorkspaceItem.triggered()
            }

            onTriggered: mainWindow.workspaceSave("")
        }

        MenuItem {
            id: menuModuleFileMenuSaveWorkspaceAsItem

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                IconImage {
                    source: "qrc:/icon/saveAs.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    color: global.fore
                }

                Label {
                    text: qsTr("Save Workspace")
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: "Ctrl+Shift+S"
                }
            }

            Shortcut {
                sequence: "Ctrl+Shift+S"
                onActivated: menuModuleFileMenuSaveWorkspaceAsItem.triggered()
            }

            onTriggered: menuModuleFileMenuSaveDialog.open()

            FileDialog {
                id: menuModuleFileMenuSaveDialog
                currentFolder: StandardPaths.standardLocations(StandardPaths.DesktopLocation)[0]
                fileMode: FileDialog.SaveFile
                nameFilters: ["Json (*.json)"]
                currentFile: currentFolder + "/config.json"
                onAccepted: mainWindow.workspaceSave(selectedFile)
            }
        }
    }

    Menu {
        id: menuModuleEditMenu
        implicitWidth: 300
        property var menuSession

        onOpened: {
            mainWindow.overlayFlagSet(false, false)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: menuModuleEditMenu.menuSession = documentModule.menuGet("edit")

        MenuItem {
            id: menuModuleEditUndoItem
            enabled: menuModuleEditMenu.menuSession ? menuModuleEditMenu.menuSession.undoable : false

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                IconImage {
                    source: "qrc:/icon/undo.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    color: menuModuleEditUndoItem.enabled ? global.fore : global.foreDisabled
                }

                Label {
                    text: qsTr("Undo")
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: "Ctrl+Z"
                }
            }

            onTriggered: documentModule.menuRequest("undo")
        }

        MenuItem {
            id: menuModuleEditRedoItem
            enabled: menuModuleEditMenu.menuSession ? menuModuleEditMenu.menuSession.redoable : false

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                IconImage {
                    source: "qrc:/icon/redo.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    color: menuModuleEditRedoItem.enabled ? global.fore : global.foreDisabled
                }

                Label {
                    text: qsTr("Redo")
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: "Ctrl+Y"
                }
            }

            onTriggered: documentModule.menuRequest("redo")
        }

        MenuSeparator {
        }

        MenuItem {
            id: menuModuleEditCutItem
            enabled: menuModuleEditMenu.menuSession ? menuModuleEditMenu.menuSession.copiable : false

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                IconImage {
                    source: "qrc:/icon/cut.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    color: menuModuleEditCutItem.enabled ? global.fore : global.foreDisabled
                }

                Label {
                    text: qsTr("Cut")
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: "Ctrl+X"
                }
            }

            onTriggered: documentModule.menuRequest("cut")
        }

        MenuItem {
            id: menuModuleEditCopyItem
            enabled: menuModuleEditMenu.menuSession ? menuModuleEditMenu.menuSession.copiable : false

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                IconImage {
                    source: "qrc:/icon/copy.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    color: menuModuleEditCopyItem.enabled ? global.fore : global.foreDisabled
                }

                Label {
                    text: qsTr("Copy")
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: "Ctrl+C"
                }
            }

            onTriggered: documentModule.menuRequest("copy")
        }

        MenuItem {
            id: menuModuleEditPasteItem
            enabled: menuModuleEditMenu.menuSession ? menuModuleEditMenu.menuSession.pastable : false

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                IconImage {
                    source: "qrc:/icon/paste.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    color: menuModuleEditPasteItem.enabled ? global.fore : global.foreDisabled
                }

                Label {
                    text: qsTr("Paste")
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: "Ctrl+V"
                }
            }

            onTriggered: documentModule.menuRequest("paste")
        }

        MenuSeparator {
        }

        MenuItem {
            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                IconImage {
                    source: "qrc:/icon/search.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    color: global.fore
                }

                Label {
                    text: qsTr("Search")
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: "Ctrl+F"
                }
            }

            onTriggered: documentModule.menuRequest("search")
        }

        MenuItem {
            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                IconImage {
                    source: "qrc:/icon/replace.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    color: global.fore
                }

                Label {
                    text: qsTr("Replace")
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: "Ctrl+R"
                }
            }

            onTriggered: documentModule.menuRequest("replace")
        }
    }

    Menu {
        id: menuModuleViewMenu

        onOpened: {
            mainWindow.overlayFlagSet(false, false)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Explorer")
            checkable: true
            checked: explorerModuleAction ? explorerModuleAction.checked : false

            onTriggered: explorerModuleAction.toggle()
        }

        MenuItem {
            text: qsTr("Port")
            checkable: true
            checked: portModuleAction ? portModuleAction.checked : false

            onTriggered: portModuleAction.toggle()
        }

        MenuItem {
            text: qsTr("Send")
            checkable: true
            checked: sendModuleAction ? sendModuleAction.checked : false

            onTriggered: sendModuleAction.toggle()
        }

        MenuItem {
            text: qsTr("Threadpool")
            checkable: true
            checked: threadpoolModuleAction ? threadpoolModuleAction.checked : false

            onTriggered: threadpoolModuleAction.toggle()
        }

        MenuSeparator {
        }

        MenuItem {
            text: qsTr("Diagnostics")
            checkable: true
            checked: diagnosticsModuleAction ? diagnosticsModuleAction.checked : false

            onTriggered: diagnosticsModuleAction.toggle()
        }

        MenuItem {
            text: qsTr("Structure")
            checkable: true
            checked: structureModuleAction ? structureModuleAction.checked : false

            onTriggered: structureModuleAction.toggle()
        }

        MenuSeparator {
        }

        MenuItem {
            text: qsTr("Database")
            checkable: true
            checked: databaseModuleAction ? databaseModuleAction.checked : false

            onTriggered: databaseModuleAction.toggle()
        }

        MenuItem {
            text: qsTr("Datatable")
            checkable: true
            checked: datatableModuleAction ? datatableModuleAction.checked : false

            onTriggered: datatableModuleAction.toggle()
        }

        MenuItem {
            text: qsTr("Dataplot")
            checkable: true
            checked: dataplotModuleAction ? dataplotModuleAction.checked : false

            onTriggered: dataplotModuleAction.toggle()
        }

        MenuSeparator {
        }

        MenuItem {
            text: qsTr("Breakpoint")
            checkable: true
            checked: breakpointModuleAction ? breakpointModuleAction.checked : false

            onTriggered: breakpointModuleAction.toggle()
        }

        MenuItem {
            text: qsTr("Debug")
            checkable: true
            checked: debugModuleAction ? debugModuleAction.checked : false

            onTriggered: debugModuleAction.toggle()
        }

        MenuItem {
            text: qsTr("Watch")
            checkable: true
            checked: watchModuleAction ? watchModuleAction.checked : false

            onTriggered: watchModuleAction.toggle()
        }

        MenuSeparator {
        }

        MenuItem {
            text: qsTr("Log")
            checkable: true
            checked: logModuleAction ? logModuleAction.checked : false

            onTriggered: logModuleAction.toggle()
        }

        MenuItem {
            text: qsTr("Git")
            checkable: true
            checked: gitModuleAction ? gitModuleAction.checked : false

            onTriggered: gitModuleAction.toggle()
        }

        MenuItem {
            text: qsTr("Cmd")

            onClicked: terminalModule.cmdOpen()
        }

        MenuItem {
            text: qsTr("Powershell")

            onClicked: terminalModule.powershellOpen()
        }
    }

    Menu {
        id: menuModuleNavMenu
        implicitWidth: 300
        property var menuSession

        onOpened: {
            mainWindow.overlayFlagSet(false, false)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: menuModuleNavMenu.menuSession = documentModule.menuGet("nav")

        MenuItem {
            id: menuModuleNavPrevItem
            enabled: menuModuleNavMenu.menuSession ? menuModuleNavMenu.menuSession.prev : false

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                IconImage {
                    source: "qrc:/icon/arrowLeft.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    color: menuModuleNavPrevItem.enabled ? global.fore : global.foreDisabled
                }

                Label {
                    text: qsTr("Prev")
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: "Alt+Left"
                }
            }

            Shortcut {
                sequence: "Alt+Left"
                onActivated: menuModuleNavPrevItem.triggered()
            }

            onTriggered: documentModule.navigationPrev()
        }

        MenuItem {
            id: menuModuleNavNextItem
            enabled: menuModuleNavMenu.menuSession ? menuModuleNavMenu.menuSession.next : false

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                IconImage {
                    source: "qrc:/icon/arrowRight.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    color: menuModuleNavNextItem.enabled ? global.fore : global.foreDisabled
                }

                Label {
                    text: qsTr("Next")
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: "Alt+Right"
                }
            }

            Shortcut {
                sequence: "Alt+Right"
                onActivated: menuModuleNavNextItem.triggered()
            }

            onTriggered: documentModule.navigationNext()
        }

        MenuSeparator {
        }

        MenuItem {
            enabled: menuModuleNavMenu.menuSession ? menuModuleNavMenu.menuSession.navigation : false

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                Item {
                    Layout.preferredWidth: 16; Layout.preferredHeight: 16;
                }

                Label {
                    text: qsTr("Definition(s)")
                }

                Item {
                    Layout.fillWidth: true
                }
            }

            onTriggered: documentModule.definitionRequest(menuModuleNavMenu.menuSession.documentUrl, menuModuleNavMenu.menuSession.line, menuModuleNavMenu.menuSession.character)
        }

        MenuItem {
            enabled: menuModuleNavMenu.menuSession ? menuModuleNavMenu.menuSession.navigation : false

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                Item {
                    Layout.preferredWidth: 16; Layout.preferredHeight: 16;
                }

                Label {
                    text: qsTr("Reference(s)")
                }

                Item {
                    Layout.fillWidth: true
                }
            }

            onTriggered: documentModule.referencesRequest(menuModuleNavMenu.menuSession.documentUrl, menuModuleNavMenu.menuSession.line, menuModuleNavMenu.menuSession.character)
        }

        MenuItem {
            enabled: menuModuleNavMenu.menuSession ? menuModuleNavMenu.menuSession.navigation : false

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                Item {
                    Layout.preferredWidth: 16; Layout.preferredHeight: 16;
                }

                Label {
                    text: qsTr("Implementation(s)")
                }

                Item {
                    Layout.fillWidth: true
                }
            }

            onTriggered: documentModule.implementationRequest(menuModuleNavMenu.menuSession.documentUrl, menuModuleNavMenu.menuSession.line, menuModuleNavMenu.menuSession.character)
        }

        MenuItem {
            enabled: menuModuleNavMenu.menuSession ? menuModuleNavMenu.menuSession.navigation : false

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                Item {
                    Layout.preferredWidth: 16; Layout.preferredHeight: 16;
                }

                Label {
                    text: qsTr("Type Definition(s)")
                }

                Item {
                    Layout.fillWidth: true
                }
            }

            onTriggered: documentModule.typeDefinitionRequest(menuModuleNavMenu.menuSession.documentUrl, menuModuleNavMenu.menuSession.line, menuModuleNavMenu.menuSession.character)
        }
    }

    Menu {
        id: menuModuleCodeMenu
        implicitWidth: 300
        property var menuSession

        onOpened: {
            mainWindow.overlayFlagSet(false, false)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: menuModuleCodeMenu.menuSession = documentModule.menuGet("code")

        MenuItem {
            id: menuModuleCodeCompletionItem

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                IconImage {
                    source: "qrc:/icon/completion.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    color: global.fore
                }

                Label {
                    text: qsTr("Completion")
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: "Ctrl+Space"
                }
            }

            Shortcut {
                sequence: "Ctrl+Space"
                onActivated: {
                    menuModuleCodeMenu.menuSession = documentModule.menuGet("code")
                    menuModuleCodeCompletionItem.triggered()
                }
            }

            onTriggered: documentModule.completionRequest(menuModuleCodeMenu.menuSession.documentUrl, menuModuleCodeMenu.menuSession.line, menuModuleCodeMenu.menuSession.character)
        }

        MenuItem {
            id: menuModuleCodeReformatItem

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                IconImage {
                    source: "qrc:/icon/brush.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    color: global.fore
                }

                Label {
                    text: menuModuleCodeMenu.menuSession ? menuModuleCodeMenu.menuSession.text ? qsTr("Reformat Selected") : qsTr("Reformat") : false
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: "Ctrl+Alt+L"
                }
            }

            Shortcut {
                sequence: "Ctrl+Alt+L"
                onActivated: {
                    menuModuleCodeMenu.menuSession = documentModule.menuGet("code")
                    menuModuleCodeReformatItem.triggered()
                }
            }

            onTriggered: {
                if (menuModuleCodeMenu.menuSession.text) {
                    documentModule.rangeFormattingRequest(menuModuleCodeMenu.menuSession.documentUrl, menuModuleCodeMenu.menuSession.startLine, menuModuleCodeMenu.menuSession.startCharacter, menuModuleCodeMenu.menuSession.endLine, menuModuleCodeMenu.menuSession.endCharacter)
                } else {
                    documentModule.formattingRequest(menuModuleCodeMenu.menuSession.documentUrl)
                }
            }
        }

        Menu {
            title: qsTr("Folding")
            icon.source: "qrc:/icon/fold.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Contract Top")
                icon.source: "qrc:/icon/collapse.svg"
                icon.width: 16; icon.height: 16

                onTriggered: documentModule.foldContractTop(menuModuleCodeMenu.menuSession.documentUrl)
            }

            MenuItem {
                text: qsTr("Contract Recursively")
                icon.source: "qrc:/icon/collapse.svg"
                icon.width: 16; icon.height: 16

                onTriggered: documentModule.foldContractRecursively(menuModuleCodeMenu.menuSession.documentUrl)
            }

            MenuItem {
                text: qsTr("Expand Recursively")
                icon.source: "qrc:/icon/expand.svg"
                icon.width: 16; icon.height: 16

                onTriggered: documentModule.foldExpandRecursively(menuModuleCodeMenu.menuSession.documentUrl)
            }
        }
    }

    Menu {
        id: menuModuleExecMenu
        implicitWidth: 300
        property var menuSession

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: menuModuleExecMenu.menuSession = documentModule.menuGet("exec")

        MenuItem {
            id: menuModuleExecRunItem

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                IconImage {
                    source: "qrc:/icon/play.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    color: global.successFore3
                }

                Label {
                    text: qsTr("Run ") + (menuModuleExecMenu.menuSession ? menuModuleExecMenu.menuSession.text ? "Selected" : menuModuleExecMenu.menuSession.documentName : "")
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: "Shift+F10"
                }
            }

            Shortcut {
                sequence: "Shift+F10"
                onActivated: {
                    menuModuleExecMenu.menuSession = documentModule.menuGet("exec")
                    menuModuleExecRunItem.triggered()
                }
            }

            onTriggered: {
                if (menuModuleExecMenu.menuSession.text) {
                    threadpoolModule.threadStart(menuModuleExecMenu.menuSession.documentUrl, 0, menuModuleExecMenu.menuSession.startLine, menuModuleExecMenu.menuSession.startCharacter, menuModuleExecMenu.menuSession.endLine, menuModuleExecMenu.menuSession.endCharacter)
                } else {
                    threadpoolModule.threadStart(menuModuleExecMenu.menuSession.documentUrl, 0)
                }
            }
        }

        MenuItem {
            id: menuModuleExecDebugItem

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                IconImage {
                    source: "qrc:/icon/bug.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    color: global.successFore3
                }

                Label {
                    text: qsTr("Debug ") + (menuModuleExecMenu.menuSession ? menuModuleExecMenu.menuSession.documentName : "")
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: "Shift+F9"
                }
            }

            Shortcut {
                sequence: "Shift+F9"
                onActivated: {
                    menuModuleExecMenu.menuSession = documentModule.menuGet("exec")
                    menuModuleExecDebugItem.triggered()
                }
            }

            onTriggered: threadpoolModule.threadStart(menuModuleExecMenu.menuSession.documentUrl, 1)
        }
    }

    Menu {
        id: menuModuleGitMenu

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                Item {
                    Layout.preferredWidth: 16; Layout.preferredHeight: 16;
                }

                Label {
                    text: qsTr("Status")
                }

                Item {
                    Layout.fillWidth: true
                }
            }

            onTriggered: gitModule.gitStatus()
        }

        MenuItem {
            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                IconImage {
                    source: "qrc:/icon/commit.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    color: global.fore
                }

                Label {
                    text: qsTr("Commit")
                }

                Item {
                    Layout.fillWidth: true
                }
            }

            onTriggered: gitModule.gitCommit()
        }
    }

    // port module
    Menu {
        id: portModuleTableMenu
        property int portIndex

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("New")
            icon.source: "qrc:/icon/add.svg"
            icon.width: 16; icon.height: 16

            onTriggered: portModule.portSetting()
        }

        MenuItem {
            text: qsTr("Edit")
            icon.source: "qrc:/icon/edit.svg"
            icon.width: 16; icon.height: 16

            onTriggered: portModule.portSetting(portModuleTableMenu.portIndex)
        }

        Menu {
            title: qsTr("Delete")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16

            DelayButton {
                delay: 1000
                text: qsTr("Confirm")

                onActivated: {
                    portModule.portRemove(portModuleTableMenu.portIndex)
                    progress = 0
                    portModuleTableMenu.close()
                }
            }
        }
    }

    Menu {
        id: portModuleRootMenu

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("New")
            icon.source: "qrc:/icon/add.svg"
            icon.width: 16; icon.height: 16

            onTriggered: portModule.portSetting()
        }
    }

    // status module
    Menu {
        id: statusModuleEolModeMenu
        property var eolModeButton
        property url documentUrl
        property int eolMode

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            statusModuleEolModeMenu.eolMode = documentModule.eolModeGet(documentUrl)
            statusModuleEolModeViewItem.checked = documentModule.eolViewGet(documentUrl)
        }

        MenuItem {
            text: "CRLF"
            enabled: statusModuleEolModeMenu.eolMode !== 0
            onTriggered: {
                documentModule.eolModeSet(statusModuleEolModeMenu.documentUrl, 0)
                statusModuleEolModeMenu.eolModeButton.text = "CRLF"
            }
        }

        MenuItem {
            text: "CR"
            enabled: statusModuleEolModeMenu.eolMode !== 1
            onTriggered: {
                documentModule.eolModeSet(statusModuleEolModeMenu.documentUrl, 1)
                statusModuleEolModeMenu.eolModeButton.text = "CR"
            }
        }

        MenuItem {
            text: "LF"
            enabled: statusModuleEolModeMenu.eolMode !== 2
            onTriggered: {
                documentModule.eolModeSet(statusModuleEolModeMenu.documentUrl, 2)
                statusModuleEolModeMenu.eolModeButton.text = "LF"
            }
        }

        MenuSeparator {
        }

        MenuItem {
            id: statusModuleEolModeViewItem
            checkable: true
            text: qsTr("View EOL")

            onTriggered: documentModule.eolViewSet(statusModuleEolModeMenu.documentUrl, statusModuleEolModeViewItem.checked)
        }
    }

    // structure module
    Menu {
        id: structureModuleRootMenu
        property var treeView

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        Menu {
            title: qsTr("Folding")
            icon.source: "qrc:/icon/fold.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Collapse All")
                icon.source: "qrc:/icon/collapse.svg"
                icon.width: 16; icon.height: 16

                onTriggered: structureModuleRootMenu.treeView.collapseRecursively()
            }

            MenuItem {
                text: qsTr("Expand All")
                icon.source: "qrc:/icon/expand.svg"
                icon.width: 16; icon.height: 16

                onTriggered: structureModuleRootMenu.treeView.expandRecursively()
            }
        }
    }

    // threadpool module
    Dialog {
        id: threadpoolModuleErrorDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        title: qsTr("Terminate Request Has Been Sent")
        standardButtons: Dialog.Ok

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
    }

    Menu {
        id: threadpoolModuleThreadMenu
        property string threadId

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Terminate")
            icon.source: "qrc:/icon/stop.svg"
            icon.width: 16; icon.height: 16

            onTriggered: threadpoolModule.threadStop(threadpoolModuleThreadMenu.threadId)
        }
    }

    // watch module
    Dialog {
        id: watchModuleExpressionDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        title: qsTr("Enter Watch")
        standardButtons: Dialog.Ok
        property int watchIndex
        property url watchUrl
        property string watchExpression

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            watchModuleUrlTextField.text = watchModuleExpressionDialog.watchUrl
            watchModuleExpressionTextField.text = watchModuleExpressionDialog.watchExpression
            watchModuleExpressionTextField.forceActiveFocus()
            watchModuleExpressionTextField.selectAll()
        }
        onAccepted: {
            if (watchModuleExpressionDialog.watchIndex === -1 || !watchModuleExpressionDialog.watchExpression) {
                watchModule.watchInsert(watchModuleExpressionDialog.watchIndex, watchModuleUrlTextField.text, watchModuleExpressionTextField.text)
            } else {
                watchModule.watchRename(watchModuleExpressionDialog.watchIndex, watchModuleUrlTextField.text, watchModuleExpressionTextField.text)
            }
        }

        ColumnLayout {
            width: parent.width

            Label {
                text: qsTr("Url")
                horizontalAlignment: Text.AlignLeft
                Layout.fillWidth: true
            }

            TextField {
                id: watchModuleUrlTextField
                width: parent.width
                placeholderText: qsTr("Enter url:")
                Layout.fillWidth: true

                onAccepted: watchModuleExpressionDialog.accept()
                Keys.onEscapePressed: watchModuleExpressionDialog.reject()
            }

            Label {
                text: qsTr("Expression")
                horizontalAlignment: Text.AlignLeft
                Layout.fillWidth: true
            }

            TextField {
                id: watchModuleExpressionTextField
                width: parent.width
                placeholderText: qsTr("Enter expression:")
                Layout.fillWidth: true

                onAccepted: watchModuleExpressionDialog.accept()
                Keys.onEscapePressed: watchModuleExpressionDialog.reject()
            }
        }
    }

    Dialog {
        id: watchModuleValueDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        title: qsTr("Edit Value")
        standardButtons: Dialog.Ok
        property string currentThread
        property url watchUrl
        property string watchExpression
        property string currentValue
        property string currentType

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            watchModuleValueTextField.text = watchModuleValueDialog.currentValue
            watchModuleValueTextField.forceActiveFocus()
            watchModuleValueTextField.selectAll()
            watchModuleValueComboBox.currentValue = watchModuleValueDialog.currentType
        }
        onAccepted: {
            threadpoolModule.valueSet(watchModuleValueDialog.currentThread, watchModuleValueDialog.watchUrl, watchModuleValueDialog.watchExpression, watchModuleValueTextField.text, watchModuleValueComboBox.currentValue)
        }

        ColumnLayout {
            width: parent.width

            Label {
                text: watchModuleValueDialog.currentThread
                horizontalAlignment: Text.AlignLeft
                Layout.fillWidth: true
            }

            Label {
                text: watchModuleValueDialog.watchUrl
                horizontalAlignment: Text.AlignLeft
                Layout.fillWidth: true
            }

            Label {
                text: watchModuleValueDialog.watchExpression
                horizontalAlignment: Text.AlignLeft
                Layout.fillWidth: true
            }

            RowLayout {

                TextField {
                    id: watchModuleValueTextField
                    placeholderText: qsTr("Enter new value:")
                    Layout.fillWidth: true

                    onAccepted: watchModuleValueDialog.accept()
                    Keys.onEscapePressed: watchModuleValueDialog.reject()
                }

                ComboBox {
                    id: watchModuleValueComboBox
                    model: ListModel {
                        ListElement {
                            text: "boolean"; value: "boolean"
                        }
                        ListElement {
                            text: "number"; value: "number"
                        }
                        ListElement {
                            text: "string"; value: "string"
                        }
                    }
                    textRole: "text"
                    valueRole: "value"
                }
            }
        }
    }

    Menu {
        id: watchModuleExpressionMenu
        property int watchIndex
        property url watchUrl
        property string watchExpression

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Rename")
            icon.source: "qrc:/icon/rename.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                watchModuleExpressionDialog.watchIndex = watchModuleExpressionMenu.watchIndex
                watchModuleExpressionDialog.watchUrl = watchModuleExpressionMenu.watchUrl
                watchModuleExpressionDialog.watchExpression = watchModuleExpressionMenu.watchExpression
                watchModuleExpressionDialog.open()
            }
        }

        MenuItem {
            text: qsTr("Clear")
            icon.source: "qrc:/icon/eraser.svg"
            icon.width: 16; icon.height: 16

            onTriggered: watchModule.watchClear(watchModuleExpressionMenu.watchIndex)
        }

        MenuItem {
            text: qsTr("Delete")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16

            onTriggered: watchModule.watchRemove(watchModuleExpressionMenu.watchIndex)
        }
    }

    Menu {
        id: watchModuleValueMenu
        property url watchUrl
        property string watchExpression
        property string currentValue
        property string currentType

        onOpened: {
            mainWindow.overlayFlagSet(false, true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Hot Update")
            icon.source: "qrc:/icon/edit.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                if (!debugModule.threadGet()) {
                    debugModuleErrorDialog.open()
                } else {
                    watchModuleValueDialog.currentThread = debugModule.threadGet()
                    watchModuleValueDialog.watchUrl = watchModuleValueMenu.watchUrl
                    watchModuleValueDialog.watchExpression = watchModuleValueMenu.watchExpression
                    watchModuleValueDialog.currentValue = watchModuleValueMenu.currentValue
                    watchModuleValueDialog.currentType = watchModuleValueMenu.currentType
                    watchModuleValueDialog.open()
                }
            }
        }

        MenuItem {
            text: qsTr("Clear")
            icon.source: "qrc:/icon/eraser.svg"
            icon.width: 16; icon.height: 16

            onTriggered: watchModule.watchClear(watchModuleExpressionMenu.watchIndex)
        }

        MenuItem {
            text: qsTr("Delete")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16

            onTriggered: watchModule.watchRemove(watchModuleExpressionMenu.watchIndex)
        }
    }

    Menu {
        id: watchModuleRootMenu

        onOpened: {
            widgetCount += 1
            mainWindow.overlayFlagSet(undefined, true)
        }
        onClosed: widgetCount -= 1

        Menu {
            title: qsTr("Clear")
            icon.source: "qrc:/icon/eraser.svg"
            icon.width: 16; icon.height: 16

            DelayButton {
                delay: 1000
                text: qsTr("Confirm")

                onActivated: {
                    watchModule.watchClear(-1)
                    progress = 0
                    watchModuleRootMenu.close()
                }
            }
        }
    }

    Component.onCompleted: {
        const objects = {
            "mainItem": mainItem,
            "mainWindowBusyDialog": mainWindowBusyDialog,
            "mainWindowCloseDialog": mainWindowCloseDialog,
            "mainWindowMessageDialog": mainWindowMessageDialog,
            "mainWindowQuitDialog": mainWindowQuitDialog,
            "mainWindowTextView": mainWindowTextView,
            "mainWindowToolTip": mainWindowToolTip,

            "lualsProgressDialog": lualsProgressDialog,

            "breakpointModuleEditDialog": breakpointModuleEditDialog,
            "breakpointModuleLineMenu": breakpointModuleLineMenu,
            "breakpointModuleFileMenu": breakpointModuleFileMenu,
            "breakpointModuleRootMenu": breakpointModuleRootMenu,

            "databaseModuleEditDialog": databaseModuleEditDialog,
            "databaseModuleTableMenu": databaseModuleTableMenu,
            "databaseModuleRootMenu": databaseModuleRootMenu,

            "datatableModuleEditDialog": datatableModuleEditDialog,
            "datatableModuleTableMenu": datatableModuleTableMenu,
            "datatableModuleRootMenu": datatableModuleRootMenu,

            "dataplotModuleRootMenu": dataplotModuleRootMenu,

            "debugModuleErrorDialog": debugModuleErrorDialog,

            "diagnosticsModuleDiagnosticMenu": diagnosticsModuleDiagnosticMenu,

            "documentModuleSaveDialog": documentModuleSaveDialog,
            "documentModuleEditorMenu": documentModuleEditorMenu,
            "documentModuleCompletionToolTip": documentModuleCompletionToolTip,
            "documentModuleCompletionTableView": documentModuleCompletionTableView,
            "documentModuleCompletionDetailTableView": documentModuleCompletionDetailTableView,
            "documentModuleDwellToolTip": documentModuleDwellToolTip,
            "documentModuleDwellDiagnosticTextArea": documentModuleDwellDiagnosticTextArea,
            "documentModuleDwellHoverTextArea": documentModuleDwellHoverTextArea,
            "documentModuleDwellCodeActionMenu": documentModuleDwellCodeActionMenu,
            "documentModuleDwellSuggestionMenu": documentModuleDwellSuggestionMenu,
            "documentModuleNavigationToolTip": documentModuleNavigationToolTip,
            "documentModuleNavigationTableView": documentModuleNavigationTableView,
            "documentModuleNavigationDetailLabel": documentModuleNavigationDetailLabel,
            "documentModulePositionTooltip": documentModulePositionTooltip,
            "documentModuleSignatureToolTip": documentModuleSignatureToolTip,
            "documentModuleSignatureLabel": documentModuleSignatureLabel,

            "explorerModuleFileMenu": explorerModuleFileMenu,
            "explorerModuleFolderMenu": explorerModuleFolderMenu,
            "explorerModuleRootMenu": explorerModuleRootMenu,

            "fileModulePropertyDialog": fileModulePropertyDialog,

            "logModuleHeightDialog": logModuleHeightDialog,
            "logModuleLinkMenu": logModuleLinkMenu,

            "menuModuleFileMenu": menuModuleFileMenu,
            "menuModuleEditMenu": menuModuleEditMenu,
            "menuModuleViewMenu": menuModuleViewMenu,
            "menuModuleNavMenu": menuModuleNavMenu,
            "menuModuleCodeMenu": menuModuleCodeMenu,
            "menuModuleExecMenu": menuModuleExecMenu,
            "menuModuleGitMenu": menuModuleGitMenu,

            "portModuleTableMenu": portModuleTableMenu,
            "portModuleRootMenu": portModuleRootMenu,

            "statusModuleEolModeMenu": statusModuleEolModeMenu,

            "structureModuleRootMenu": structureModuleRootMenu,

            "threadpoolModuleErrorDialog": threadpoolModuleErrorDialog,
            "threadpoolModuleThreadMenu": threadpoolModuleThreadMenu,

            "watchModuleExpressionMenu": watchModuleExpressionMenu,
            "watchModuleValueMenu": watchModuleValueMenu,
            "watchModuleRootMenu": watchModuleRootMenu,
        };
        mainWindow.propertyGet(objects)
    }
}