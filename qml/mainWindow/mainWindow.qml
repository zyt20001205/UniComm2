import QtCore
import QtQuick
import QtQuick.Controls
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
            border.color: "#c50f1f"
            border.width: 3
        }

        RowLayout {
            anchors.centerIn: parent

            Label {
                id: debugWidgetCount
                color: "#c50f1f"
                font.pointSize: 16
            }
        }
    }

    onWidgetCountChanged: {
        if (widgetCount < 0) {
            console.log("severe error occured!!! widget count: " + widgetCount)
            widgetCount = 0
        }
        if (widgetCount === 0) {
            mainWindow.overlayTransparent(true)
            mainWindow.overlayFocus(false)
        } else {
            mainWindow.overlayTransparent(false)
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
            mainWindow.overlayFocus(true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onRejected: systemModule.processTerminate()

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
            mainWindow.overlayFocus(true)
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
            mainWindow.overlayFocus(true)
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
            mainWindow.overlayFocus(true)
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
                mainWindow.overlayFocus(true)
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
        id: mainWindowToolTip
        parent: Overlay.overlay
        x: position.x + 10; y: position.y + 10
        closePolicy: Popup.NoAutoClose
        visible: text
        property point position

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
            mainWindow.overlayFocus(true)
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
        property url scriptUrl
        property int line

        onOpened: {
            mainWindow.overlayFocus(true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            breakpointModuleEnabledCheckBox.checkState = breakpointModule.enabledGet(breakpointModuleEditDialog.scriptUrl, breakpointModuleEditDialog.line) ? Qt.Checked : Qt.Unchecked
            breakpointModuleConditionTextField.text = breakpointModule.conditionGet(breakpointModuleEditDialog.scriptUrl, breakpointModuleEditDialog.line)
            breakpointModuleConditionTextField.forceActiveFocus()
            breakpointModuleConditionTextField.selectAll()
        }
        onAccepted: {
            breakpointModule.enabledSet(breakpointModuleEditDialog.scriptUrl, breakpointModuleEditDialog.line, breakpointModuleEnabledCheckBox.checked)
            breakpointModule.conditionSet(breakpointModuleEditDialog.scriptUrl, breakpointModuleEditDialog.line, breakpointModuleConditionTextField.text)
        }

        ColumnLayout {
            width: parent.width

            Label {
                text: breakpointModuleEditDialog.scriptUrl + ":" + breakpointModuleEditDialog.line
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
        property url scriptUrl
        property int line
        property var treeView

        onOpened: {
            mainWindow.overlayFocus(true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("View")
            icon.source: "qrc:/icon/eye.svg"
            icon.width: 16; icon.height: 16

            onTriggered: breakpointModule.markerAdd(breakpointModuleLineMenu.scriptUrl, breakpointModuleLineMenu.line)
        }

        MenuItem {
            text: qsTr("Setting")
            icon.source: "qrc:/icon/settings.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                breakpointModuleEditDialog.scriptUrl = breakpointModuleLineMenu.scriptUrl
                breakpointModuleEditDialog.line = breakpointModuleLineMenu.line
                breakpointModuleEditDialog.open()
            }
        }

        MenuItem {
            text: qsTr("Delete")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16

            onTriggered: breakpointModule.breakpointDelete(breakpointModuleLineMenu.scriptUrl, breakpointModuleLineMenu.line)
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
        property url scriptUrl
        property var treeView

        onOpened: {
            mainWindow.overlayFocus(true)
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
                    breakpointModule.breakpointsDelete(breakpointModuleFileMenu.scriptUrl)
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
            mainWindow.overlayFocus(true)
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
            mainWindow.overlayFocus(true)
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
            mainWindow.overlayFocus(true)
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
            mainWindow.overlayFocus(true)
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
            mainWindow.overlayFocus(true)
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
            mainWindow.overlayFocus(true)
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
            mainWindow.overlayFocus(true)
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
            mainWindow.overlayFocus(true)
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
            mainWindow.overlayFocus(true)
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
            mainWindow.overlayFocus(true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Copy")
            icon.source: "qrc:/icon/copy.svg"
            icon.width: 16; icon.height: 16

            onTriggered: diagnosticsModule.diagnosticCopy(diagnosticsModuleDiagnosticMenu.diagnostic)
        }

        MenuItem {
            text: qsTr("View")
            icon.source: "qrc:/icon/eye.svg"
            icon.width: 16; icon.height: 16

            onTriggered: diagnosticsModule.indicatorFill(diagnosticsModuleDiagnosticMenu.position)
        }
    }

    // explorer module
    Dialog {
        id: explorerModuleScriptNewDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        title: qsTr("New Script")
        standardButtons: Dialog.Ok | Dialog.Cancel
        property string filePath


        onOpened: {
            mainWindow.overlayFocus(true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            explorerModuleScriptNameTextField.clear()
            explorerModuleScriptNameTextField.forceActiveFocus()
        }
        onAccepted: systemModule.fileNew("file:///" + explorerModuleScriptNewDialog.filePath + "/" + explorerModuleScriptNameTextField.text + ".lua")

        TextField {
            id: explorerModuleScriptNameTextField
            width: parent.width
            placeholderText: qsTr("Enter script name:")

            onAccepted: explorerModuleScriptNewDialog.accept()
            Keys.onEscapePressed: explorerModuleScriptNewDialog.reject()
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
        property string filePath

        onOpened: {
            mainWindow.overlayFocus(true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            explorerModuleFolderNameTextField.clear()
            explorerModuleFolderNameTextField.forceActiveFocus()
        }
        onAccepted: systemModule.fileNew("file:///" + explorerModuleFolderNewDialog.filePath + "/" + explorerModuleFolderNameTextField.text)

        TextField {
            id: explorerModuleFolderNameTextField
            width: parent.width
            placeholderText: qsTr("Enter folder name:")

            onAccepted: explorerModuleFolderNewDialog.accept()
            Keys.onEscapePressed: explorerModuleFolderNewDialog.reject()
        }
    }

    Menu {
        id: explorerModuleScriptMenu
        property string filePath
        property string fileName
        property var treeView

        onOpened: {
            mainWindow.overlayFocus(true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Run")
            icon.source: "qrc:/icon/play.svg"
            icon.width: 16; icon.height: 16

            onTriggered: explorerModule.scriptRun(explorerModuleScriptMenu.filePath)
        }

        MenuItem {
            text: qsTr("Debug")
            icon.source: "qrc:/icon/bug.svg"
            icon.width: 16; icon.height: 16

            onTriggered: explorerModule.scriptDebug(explorerModuleScriptMenu.filePath)
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

                onTriggered: systemModule.fileOpenInExplorer("file:///" + explorerModuleScriptMenu.filePath)
            }

            MenuItem {
                text: qsTr("Application")
                icon.source: "qrc:/icon/apps.svg"
                icon.width: 16; icon.height: 16

                onTriggered: systemModule.fileOpenInApplication("file:///" + explorerModuleScriptMenu.filePath)
            }
        }

        MenuItem {
            text: qsTr("Rename")
            icon.source: "qrc:/icon/rename.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                systemModuleRenameDialog.fileUrl = "file:///" + explorerModuleScriptMenu.filePath
                systemModuleRenameDialog.open()
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
                    systemModule.fileDelete("file:///" + explorerModuleScriptMenu.filePath)
                    progress = 0
                    explorerModuleScriptMenu.close()
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

                onTriggered: {
                    for (let i = 0; i < explorerModuleScriptMenu.treeView.rows; ++i) {
                        explorerModuleScriptMenu.treeView.collapseRecursively(i)
                    }
                }
            }

            MenuItem {
                text: qsTr("Expand All")
                icon.source: "qrc:/icon/expand.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    for (let i = 0; i < explorerModuleScriptMenu.treeView.rows; ++i) {
                        explorerModuleScriptMenu.treeView.expandRecursively(i)
                    }
                }
            }
        }
    }

    Menu {
        id: explorerModuleFolderMenu
        property string filePath
        property string fileName
        property var treeView

        onOpened: {
            mainWindow.overlayFocus(true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Open In Explorer")
            icon.source: "qrc:/icon/open.svg"
            icon.width: 16; icon.height: 16

            onTriggered: systemModule.fileOpenInExplorer("file:///" + explorerModuleFolderMenu.filePath)
        }

        Menu {
            title: qsTr("New")
            icon.source: "qrc:/icon/add.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Script")
                icon.source: "qrc:/icon/document.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    explorerModuleScriptNewDialog.filePath = explorerModuleFolderMenu.filePath
                    explorerModuleScriptNewDialog.open()
                }
            }

            MenuItem {
                text: qsTr("Folder")
                icon.source: "qrc:/icon/folder.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    explorerModuleFolderNewDialog.filePath = explorerModuleFolderMenu.filePath
                    explorerModuleFolderNewDialog.open()
                }
            }
        }

        MenuItem {
            text: qsTr("Rename")
            icon.source: "qrc:/icon/rename.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                systemModuleRenameDialog.fileUrl = "file:///" + explorerModuleFolderMenu.filePath
                systemModuleRenameDialog.open()
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
                    systemModule.fileDelete("file:///" + explorerModuleFolderMenu.filePath)
                    progress = 0
                    explorerModuleFolderMenu.close()
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
        property string rootPath
        property var treeView

        onOpened: {
            mainWindow.overlayFocus(true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Open In Explorer")
            icon.source: "qrc:/icon/open.svg"
            icon.width: 16; icon.height: 16

            onTriggered: systemModule.fileOpenInExplorer("file:///" + explorerModuleRootMenu.rootPath)
        }

        Menu {
            title: qsTr("New")
            icon.source: "qrc:/icon/add.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Script")
                icon.source: "qrc:/icon/document.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    explorerModuleScriptNewDialog.filePath = explorerModuleRootMenu.rootPath
                    explorerModuleScriptNewDialog.open()
                }
            }

            MenuItem {
                text: qsTr("Folder")
                icon.source: "qrc:/icon/folder.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    explorerModuleFolderNewDialog.filePath = explorerModuleRootMenu.rootPath
                    explorerModuleFolderNewDialog.open()
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
            mainWindow.overlayFocus(true)
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
            mainWindow.overlayFocus(true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Copy URL")
            icon.source: "qrc:/icon/copy.svg"
            icon.width: 16; icon.height: 16

            onTriggered: systemModule.copyToClipboard(logModuleLinkMenu.url)
        }

        Menu {
            title: qsTr("Open In")
            icon.source: "qrc:/icon/open.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Explorer")
                icon.source: "qrc:/icon/folder.svg"
                icon.width: 16; icon.height: 16

                onTriggered: systemModule.fileOpenInExplorer(logModuleLinkMenu.url)
            }

            MenuItem {
                text: qsTr("Application")
                icon.source: "qrc:/icon/apps.svg"
                icon.width: 16; icon.height: 16

                onTriggered: systemModule.fileOpenInApplication(logModuleLinkMenu.url)
            }
        }
    }

    // menu module
    Menu {
        id: menuModuleFileMenu

        onOpened: {
            mainWindow.overlayFocus(true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Open Workspace")
            icon.source: "qrc:/icon/open.svg"
            icon.width: 16; icon.height: 16

            onTriggered: Qt.callLater(() => mainWindow.workspaceOpen())
        }

        MenuItem {
            text: qsTr("Save Workspace")
            icon.source: "qrc:/icon/save.svg"
            icon.width: 16; icon.height: 16

            onTriggered: mainWindow.workspaceSave("")
        }

        MenuItem {
            text: qsTr("Save Workspace As")
            icon.source: "qrc:/icon/saveAs.svg"
            icon.width: 16; icon.height: 16

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
            mainWindow.overlayFocus(true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            scriptModule.menuSet("edit")
        }

        MenuItem {
            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12
                enabled: menuModuleEditMenu.menuSession ? menuModuleEditMenu.menuSession.undoable : false

                Image {
                    source: "qrc:/icon/undo.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
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

            onTriggered: scriptModule.menuRequest("undo")
        }

        MenuItem {
            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12
                enabled: menuModuleEditMenu.menuSession ? menuModuleEditMenu.menuSession.redoable : false

                Image {
                    source: "qrc:/icon/redo.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
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

            onTriggered: scriptModule.menuRequest("redo")
        }

        MenuSeparator {
        }

        MenuItem {
            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12
                enabled: menuModuleEditMenu.menuSession ? menuModuleEditMenu.menuSession.copiable : false

                Image {
                    source: "qrc:/icon/cut.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
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

            onTriggered: scriptModule.menuRequest("cut")
        }

        MenuItem {
            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12
                enabled: menuModuleEditMenu.menuSession ? menuModuleEditMenu.menuSession.copiable : false

                Image {
                    source: "qrc:/icon/copy.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
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

            onTriggered: scriptModule.menuRequest("copy")
        }

        MenuItem {
            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12
                enabled: menuModuleEditMenu.menuSession ? menuModuleEditMenu.menuSession.pastable : false

                Image {
                    source: "qrc:/icon/paste.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
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

            onTriggered: scriptModule.menuRequest("paste")
        }

        MenuSeparator {
        }

        MenuItem {
            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                Image {
                    source: "qrc:/icon/search.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
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

            onTriggered: scriptModule.menuRequest("search")
        }

        MenuItem {
            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                Image {
                    source: "qrc:/icon/replace.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
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

            onTriggered: scriptModule.menuRequest("replace")
        }
    }

    Menu {
        id: menuModuleViewMenu

        onOpened: {
            mainWindow.overlayFocus(true)
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
            text: qsTr("Log")
            checkable: true
            checked: logModuleAction ? logModuleAction.checked : false

            onTriggered: logModuleAction.toggle()
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
    }

    Menu {
        id: menuModuleCodeMenu
        implicitWidth: 300
        property var menuSession

        onOpened: {
            mainWindow.overlayFocus(true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            scriptModule.menuSet("code")
        }

        MenuItem {
            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                Item {
                    Layout.preferredWidth: 16; Layout.preferredHeight: 16;
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

            onTriggered: scriptModule.completionRequest(menuModuleCodeMenu.menuSession.scriptUrl, menuModuleCodeMenu.menuSession.line, menuModuleCodeMenu.menuSession.character)
        }

        MenuItem {
            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12; anchors.rightMargin: 12

                Image {
                    source: "qrc:/icon/brush.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                }

                Label {
                    text: menuModuleCodeMenu.menuSession ? menuModuleCodeMenu.menuSession.text ? qsTr("Range Formatting") : qsTr("Document Formatting") : false
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: "Ctrl+Alt+L"
                }
            }

            onTriggered: {
                if (menuModuleCodeMenu.menuSession.text) {
                    scriptModule.rangeFormattingRequest(menuModuleCodeMenu.menuSession.scriptUrl, menuModuleCodeMenu.menuSession.startLine, menuModuleCodeMenu.menuSession.startCharacter, menuModuleCodeMenu.menuSession.endLine, menuModuleCodeMenu.menuSession.endCharacter)
                } else {
                    scriptModule.formattingRequest(menuModuleCodeMenu.menuSession.scriptUrl)
                }
            }
        }
    }

    // port module
    Menu {
        id: portModuleTableMenu
        property int portIndex

        onOpened: {
            mainWindow.overlayFocus(true)
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
            mainWindow.overlayFocus(true)
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

    // script module
    Menu {
        id: scriptModuleEditorMenu
        focus: false
        property var menuSession

        onOpened: {
            mainWindow.overlayFocus(true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            scriptModule.menuSet("editor")
            scriptModuleEditorMenuRunHereItem.enabled = threadpoolModule.debugging()
        }

        MenuItem {
            text: qsTr("Run")
            icon.source: "qrc:/icon/play.svg"
            icon.width: 16; icon.height: 16

            onTriggered: threadpoolModule.threadStart(scriptModuleEditorMenu.menuSession.scriptUrl, 0)
        }

        MenuItem {
            text: qsTr("Run Selected")
            icon.source: "qrc:/icon/play.svg"
            icon.width: 16; icon.height: 16
            enabled: scriptModuleEditorMenu.menuSession ? scriptModuleEditorMenu.menuSession.text : false
            onTriggered: threadpoolModule.threadStart(scriptModuleEditorMenu.menuSession.scriptUrl, 0, scriptModuleEditorMenu.menuSession.startLine, scriptModuleEditorMenu.menuSession.startCharacter, scriptModuleEditorMenu.menuSession.endLine, scriptModuleEditorMenu.menuSession.endCharacter)
        }

        MenuItem {
            text: qsTr("Debug")
            icon.source: "qrc:/icon/bug.svg"
            icon.width: 16; icon.height: 16

            onTriggered: threadpoolModule.threadStart(scriptModuleEditorMenu.menuSession.scriptUrl, 1)
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

                onTriggered: scriptModule.foldContractTop(scriptModuleEditorMenu.menuSession.scriptUrl)
            }

            MenuItem {
                text: qsTr("Contract Recursively")
                icon.source: "qrc:/icon/collapse.svg"
                icon.width: 16; icon.height: 16

                onTriggered: scriptModule.foldContractRecursively(scriptModuleEditorMenu.menuSession.scriptUrl)
            }

            MenuItem {
                text: qsTr("Expand Recursively")
                icon.source: "qrc:/icon/expand.svg"
                icon.width: 16; icon.height: 16

                onTriggered: scriptModule.foldExpandRecursively(scriptModuleEditorMenu.menuSession.scriptUrl)
            }
        }

        MenuItem {
            text: scriptModuleEditorMenu.menuSession ? scriptModuleEditorMenu.menuSession.text ? qsTr("Range Formatting") : qsTr("Document Formatting") : false
            icon.source: "qrc:/icon/brush.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                if (scriptModuleEditorMenu.menuSession.text) {
                    scriptModule.rangeFormattingRequest(scriptModuleEditorMenu.menuSession.scriptUrl, scriptModuleEditorMenu.menuSession.startLine, scriptModuleEditorMenu.menuSession.startCharacter, scriptModuleEditorMenu.menuSession.endLine, scriptModuleEditorMenu.menuSession.endCharacter)
                } else {
                    scriptModule.formattingRequest(scriptModuleEditorMenu.menuSession.scriptUrl)
                }
            }
        }

        Menu {
            title: qsTr("Navigation")
            icon.source: "qrc:/icon/arrowRight.svg"
            icon.width: 16; icon.height: 16
            enabled: scriptModuleEditorMenu.menuSession ? scriptModuleEditorMenu.menuSession.navigation : false

            MenuItem {
                text: qsTr("Definition(s)")
                icon.source: "qrc:/icon/definition.svg"
                icon.width: 8; icon.height: 8

                onTriggered: scriptModule.definitionRequest(scriptModuleEditorMenu.menuSession.scriptUrl, scriptModuleEditorMenu.menuSession.line, scriptModuleEditorMenu.menuSession.character)
            }

            MenuItem {
                text: qsTr("References(s)")
                icon.source: "qrc:/icon/reference.svg"
                icon.width: 8; icon.height: 8

                onTriggered: scriptModule.referencesRequest(scriptModuleEditorMenu.menuSession.scriptUrl, scriptModuleEditorMenu.menuSession.line, scriptModuleEditorMenu.menuSession.character)
            }

            MenuItem {
                text: qsTr("Implementation(s)")
                icon.source: "qrc:/icon/implementation.svg"
                icon.width: 8; icon.height: 8

                onTriggered: scriptModule.implementationRequest(scriptModuleEditorMenu.menuSession.scriptUrl, scriptModuleEditorMenu.menuSession.line, scriptModuleEditorMenu.menuSession.character)
            }

            MenuItem {
                text: qsTr("Type Definition(s)")
                icon.source: "qrc:/icon/typeDefinition.svg"
                icon.width: 8; icon.height: 8

                onTriggered: scriptModule.typeDefinitionRequest(scriptModuleEditorMenu.menuSession.scriptUrl, scriptModuleEditorMenu.menuSession.line, scriptModuleEditorMenu.menuSession.character)
            }
        }

        MenuSeparator {
        }

        MenuItem {
            text: qsTr("Add Watch")
            icon.source: "qrc:/icon/eye.svg"
            icon.width: 16; icon.height: 16
            enabled: scriptModuleEditorMenu.menuSession ? scriptModuleEditorMenu.menuSession.text : false
            ToolTip.visible: hovered && !enabled
            ToolTip.text: qsTr("Nothing selected")

            onTriggered: {
                watchModuleExpressionDialog.watchIndex = -1
                watchModuleExpressionDialog.watchUrl = scriptModuleEditorMenu.menuSession.scriptUrl
                watchModuleExpressionDialog.watchExpression = scriptModuleEditorMenu.menuSession.text
                watchModuleExpressionDialog.open()
            }
        }

        MenuItem {
            id: scriptModuleEditorMenuRunHereItem
            text: qsTr("Run Here")
            icon.source: "qrc:/icon/debugContinue.svg"
            icon.width: 16; icon.height: 16
            enabled: false
            ToolTip.visible: hovered && !enabled
            ToolTip.text: qsTr("No debug sessions")

            onTriggered: debugModule.stateSet("", 6)
        }

        MenuItem {
            id: scriptModuleEditorMenuAssemblyItem
            text: qsTr("Assembly")
            icon.source: "qrc:/icon/assembly.svg"
            icon.width: 16; icon.height: 16

            onTriggered: scriptModule.assemblyToggle(scriptModuleEditorMenu.menuSession.scriptUrl, !scriptModuleEditorMenu.menuSession.assembly)
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

                onTriggered: systemModule.fileOpenInExplorer(scriptModuleEditorMenu.menuSession.scriptUrl)
            }

            MenuItem {
                text: qsTr("Application")
                icon.source: "qrc:/icon/apps.svg"
                icon.width: 16; icon.height: 16

                onTriggered: systemModule.fileOpenInApplication(scriptModuleEditorMenu.menuSession.scriptUrl)
            }
        }
    }

    ToolTip {
        id: scriptModuleCompletionToolTip
        parent: Overlay.overlay
        x: position.x - 30; y: position.y
        closePolicy: Popup.CloseOnPressOutside | Popup.CloseOnReleaseOutside
        property point position
        property var completionWidget
        property int typed

        onOpened: {
            mainWindow.overlayFocus(false)
            widgetCount += 1
        }
        onClosed: {
            widgetCount -= 1
            scriptModuleCompletionDetailToolTip.close()
        }
        onAboutToShow: {
            scriptModuleCompletionDetailToolTip.open()
            scriptModuleCompletionDetailTimer.restart()
        }

        contentItem: TableView {
            id: scriptModuleCompletionTableView
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
                implicitWidth: scriptModuleCompletionTableView.width; implicitHeight: textMetrics.height + 4
                required property int row

                Rectangle {
                    anchors.fill: parent
                    radius: 6
                    color: "#ebebeb"
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
                    color: scriptModuleCompletionTableView.selectedRow === row ? "#e0e0e0" : "transparent"
                }

                TextMetrics {
                    id: textMetrics
                    font: scriptModuleCompletionToolTip.font
                    text: model.display
                }

                RowLayout {
                    anchors.fill: parent
                    spacing: 0

                    Item {
                        Layout.preferredWidth: 24; Layout.preferredHeight: 24

                        Image {
                            anchors.centerIn: parent
                            width: 16; height: 16
                            source: model.decoration
                        }
                    }

                    Label {
                        font: scriptModuleCompletionToolTip.font
                        horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                        text: "<span style='color: #115ea3;'>" + model.display.substring(0, scriptModuleCompletionToolTip.typed) + "</span>" + model.display.substring(scriptModuleCompletionToolTip.typed)
                        textFormat: Text.RichText
                        elide: Text.ElideRight
                        Layout.fillWidth: true; Layout.preferredHeight: 24
                    }
                }

                HoverHandler {
                    id: hoverHandler

                    onPointChanged: scriptModuleCompletionTableView.hoveredRow = row
                    onHoveredChanged: {
                        if (!hovered) {
                            scriptModuleCompletionTableView.hoveredRow = -1
                            scriptModuleCompletionDetailTimer.restart()
                        }
                    }
                }

                TapHandler {
                    acceptedButtons: Qt.LeftButton

                    onTapped: scriptModuleCompletionTableView.selectedRow = row
                    onDoubleTapped: scriptModuleCompletionToolTip.completionWidget.textReplace()
                }

                Component.onCompleted: {
                    scriptModuleCompletionTableView.idealWidth = Math.max(24 + textMetrics.width + 4 + 10, scriptModuleCompletionTableView.idealWidth)
                    scriptModuleCompletionTableView.idealHeight = textMetrics.height + 4 + scriptModuleCompletionTableView.idealHeight
                }
            }

            onHoveredRowChanged: scriptModuleCompletionDetailTimer.restart()

            onSelectedRowChanged: {
                positionViewAtRow(selectedRow, TableView.Contain, 0, Qt.rect(0, 0, 0, 0))
                scriptModuleCompletionDetailTimer.restart()
            }

            Timer {
                id: scriptModuleCompletionDetailTimer
                interval: 150

                onTriggered: {
                    var interestRow
                    if (scriptModuleCompletionTableView.hoveredRow !== -1) {
                        interestRow = scriptModuleCompletionTableView.hoveredRow
                    } else {
                        interestRow = scriptModuleCompletionTableView.selectedRow
                    }
                    scriptModuleCompletionToolTip.completionWidget.detailReload(interestRow)
                    const index = scriptModuleCompletionTableView.index(interestRow, 0);
                    const item = scriptModuleCompletionTableView.itemAtIndex(index);
                    if (item) {
                        let idealY = item.mapToItem(scriptModuleCompletionTableView, 0, 0).y
                        idealY = Math.max(0, idealY)
                        idealY = Math.min(scriptModuleCompletionTableView.height - item.height, idealY)
                        scriptModuleCompletionDetailToolTip.y = idealY - 6
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
                target: scriptModuleCompletionTableView.model

                function onModelReset() {
                    scriptModuleCompletionTableView.idealWidth = 0
                    scriptModuleCompletionTableView.idealHeight = 0
                    scriptModuleCompletionDetailTimer.restart()
                }
            }
        }

        ToolTip {
            id: scriptModuleCompletionDetailToolTip
            x: scriptModuleCompletionToolTip.width - 5

            contentItem: TableView {
                id: scriptModuleCompletionDetailTableView
                anchors.fill: parent
                anchors.margins: 6
                implicitWidth: idealWidth; implicitHeight: idealHeight
                alternatingRows: false
                clip: true
                editTriggers: TableView.NoEditTriggers
                flickableDirection: Flickable.VerticalFlick
                property int idealWidth; property int idealHeight

                delegate: Item {
                    implicitWidth: scriptModuleCompletionDetailTableView.width; implicitHeight: textMetrics.height + 4

                    TextMetrics {
                        id: textMetrics
                        font: scriptModuleCompletionToolTip.font
                        text: model.display
                    }

                    Label {
                        id: scriptModuleCompletionDetailLabel
                        anchors.fill: parent
                        font: scriptModuleCompletionToolTip.font
                        horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                        text: model.display
                        elide: Text.ElideRight
                    }

                    Component.onCompleted: {
                        scriptModuleCompletionDetailTableView.idealWidth = Math.max(textMetrics.width + 4, scriptModuleCompletionDetailTableView.idealWidth)
                        scriptModuleCompletionDetailTableView.idealHeight = textMetrics.height + 4 + scriptModuleCompletionDetailTableView.idealHeight
                    }
                }

                Connections {
                    target: scriptModuleCompletionDetailTableView.model

                    function onModelReset() {
                        scriptModuleCompletionDetailTableView.idealWidth = 0
                        scriptModuleCompletionDetailTableView.idealHeight = 0
                    }
                }
            }
        }
    }

    ToolTip {
        id: scriptModuleDwellToolTip
        parent: Overlay.overlay
        x: position.x; y: position.y
        closePolicy: Popup.CloseOnPressOutside | Popup.CloseOnReleaseOutside
        property point position
        property var dwellWidget
        property var codeActions
        property var suggestions

        onOpened: {
            mainWindow.overlayFocus(false)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        contentItem: ColumnLayout {

            ScrollView {
                visible: scriptModuleDwellDiagnosticTextArea.length > 0
                Layout.minimumWidth: 400; Layout.maximumWidth: 800

                TextArea {
                    id: scriptModuleDwellDiagnosticTextArea
                    background: null
                    readOnly: true
                    textFormat: TextEdit.RichText
                    verticalAlignment: TextEdit.AlignTop
                    wrapMode: Text.Wrap

                    HoverHandler {
                        id: diagnosticHoverHandler
                        cursorShape: scriptModuleDwellDiagnosticTextArea.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
                    }

                    TapHandler {
                        acceptedButtons: Qt.LeftButton
                        onTapped: {
                            if (scriptModuleDwellDiagnosticTextArea.hoveredLink) {
                                scriptModuleDwellToolTip.dwellWidget.linkClick(scriptModuleDwellDiagnosticTextArea.hoveredLink)
                            }
                        }
                    }
                }
            }

            ScrollView {
                visible: scriptModuleDwellHoverTextArea.length > 0
                Layout.minimumWidth: 400; Layout.maximumWidth: 800

                TextArea {
                    id: scriptModuleDwellHoverTextArea
                    background: null
                    readOnly: true
                    textFormat: TextEdit.MarkdownText
                    verticalAlignment: TextEdit.AlignTop
                    wrapMode: Text.Wrap

                    HoverHandler {
                        id: dwellHoverHandler
                        cursorShape: scriptModuleDwellHoverTextArea.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
                    }

                    ToolTip {
                        visible: scriptModuleDwellHoverTextArea.hoveredLink
                        text: "Click to open link"
                        x: dwellHoverHandler.point.position.x + 10
                        y: dwellHoverHandler.point.position.y + 10
                    }

                    TapHandler {
                        acceptedButtons: Qt.LeftButton
                        onTapped: {
                            if (scriptModuleDwellHoverTextArea.hoveredLink) {
                                Qt.openUrlExternally(scriptModuleDwellHoverTextArea.hoveredLink)
                            }
                        }
                    }
                }
            }
        }

        Menu {
            id: scriptModuleDwellCodeActionMenu
            x: scriptModuleDwellToolTip.width - 5; y: -8

            onAboutToShow: {
                for (var i = scriptModuleDwellCodeActionMenu.count - 1; i >= 0; --i) {
                    var item = scriptModuleDwellCodeActionMenu.itemAt(i)
                    scriptModuleDwellCodeActionMenu.removeItem(item)
                    item.destroy()
                }
                if (scriptModuleDwellToolTip.codeActions.length) {
                    for (let i = 0; i < scriptModuleDwellToolTip.codeActions.length; ++i) {
                        const menuItem = scriptModuleDwellCodeActionMenuComponent.createObject(
                            null,
                            {
                                text: scriptModuleDwellToolTip.codeActions[i]["title"],
                                codeAction: scriptModuleDwellToolTip.codeActions[i]["edit"]
                            }
                        )
                        scriptModuleDwellCodeActionMenu.addItem(menuItem)
                    }
                } else {
                    const menuItem = scriptModuleDwellCodeActionMenuComponent.createObject(
                        null,
                        {
                            text: qsTr("No fixes available")
                        }
                    )
                    scriptModuleDwellCodeActionMenu.addItem(menuItem)
                }
            }

            Component {
                id: scriptModuleDwellCodeActionMenuComponent

                MenuItem {
                    property var codeAction

                    onTriggered: {
                        scriptModuleDwellToolTip.dwellWidget.codeActionAccept(codeAction)
                        scriptModuleDwellToolTip.close()
                    }

                    ToolTip.visible: hovered
                    ToolTip.text: text
                }
            }
        }

        Menu {
            id: scriptModuleDwellSuggestionMenu
            x: scriptModuleDwellToolTip.width - 5; y: -8

            MenuItem {
                text: scriptModuleDwellToolTip.suggestions ? scriptModuleDwellToolTip.suggestions[0] : ""
                visible: text

                onTriggered: {
                    scriptModuleDwellToolTip.dwellWidget.suggestionAccept(text)
                    scriptModuleDwellToolTip.close()
                }
            }

            MenuItem {
                text: scriptModuleDwellToolTip.suggestions ? scriptModuleDwellToolTip.suggestions[1] : ""
                visible: text

                onTriggered: {
                    scriptModuleDwellToolTip.dwellWidget.suggestionAccept(text)
                    scriptModuleDwellToolTip.close()
                }
            }

            MenuItem {
                text: scriptModuleDwellToolTip.suggestions ? scriptModuleDwellToolTip.suggestions[2] : ""
                visible: text

                onTriggered: {
                    scriptModuleDwellToolTip.dwellWidget.suggestionAccept(text)
                    scriptModuleDwellToolTip.close()
                }
            }

            MenuItem {
                text: scriptModuleDwellToolTip.suggestions ? scriptModuleDwellToolTip.suggestions[3] : ""
                visible: text

                onTriggered: {
                    scriptModuleDwellToolTip.dwellWidget.suggestionAccept(text)
                    scriptModuleDwellToolTip.close()
                }
            }

            MenuItem {
                text: scriptModuleDwellToolTip.suggestions ? scriptModuleDwellToolTip.suggestions[4] : ""
                visible: text

                onTriggered: {
                    scriptModuleDwellToolTip.dwellWidget.suggestionAccept(text)
                    scriptModuleDwellToolTip.close()
                }
            }
        }
    }

    ToolTip {
        id: scriptModuleNavigationToolTip
        parent: Overlay.overlay
        x: position.x - 30; y: position.y
        closePolicy: Popup.CloseOnPressOutside | Popup.CloseOnReleaseOutside
        property point position
        property var navigationWidget

        onOpened: {
            mainWindow.overlayFocus(false)
            widgetCount += 1
        }
        onClosed: {
            widgetCount -= 1
            scriptModuleNavigationDetailToolTip.close()
        }
        onAboutToShow: {
            scriptModuleNavigationDetailToolTip.open()
            scriptModuleNavigationDetailTimer.restart()
        }

        contentItem: TableView {
            id: scriptModuleNavigationTableView
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
                implicitWidth: scriptModuleNavigationTableView.width; implicitHeight: textMetrics.height + 4
                required property int row

                Rectangle {
                    anchors.fill: parent
                    radius: 6
                    color: "#ebebeb"
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
                    color: scriptModuleNavigationTableView.selectedRow === row ? "#e0e0e0" : "transparent"
                }

                TextMetrics {
                    id: textMetrics
                    font: scriptModuleNavigationToolTip.font
                    text: model.display
                }

                RowLayout {
                    anchors.fill: parent
                    spacing: 0

                    Item {
                        Layout.preferredWidth: 24; Layout.preferredHeight: 24

                        Image {
                            anchors.centerIn: parent
                            width: 8; height: 8
                            source: model.decoration
                        }
                    }

                    Label {
                        font: scriptModuleNavigationToolTip.font
                        horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                        text: model.display
                        elide: Text.ElideRight
                        Layout.fillWidth: true; Layout.preferredHeight: 24
                    }
                }

                HoverHandler {
                    id: hoverHandler

                    onPointChanged: scriptModuleNavigationTableView.hoveredRow = row
                    onHoveredChanged: {
                        if (!hovered) {
                            scriptModuleNavigationTableView.hoveredRow = -1
                            scriptModuleNavigationDetailTimer.restart()
                        }
                    }
                }

                TapHandler {
                    acceptedButtons: Qt.LeftButton

                    onTapped: scriptModuleNavigationTableView.selectedRow = row
                    onDoubleTapped: scriptModuleNavigationToolTip.navigationWidget.indicatorInsert()
                }

                Component.onCompleted: {
                    scriptModuleNavigationTableView.idealWidth = Math.max(24 + textMetrics.width + 4 + 10, scriptModuleNavigationTableView.idealWidth)
                    scriptModuleNavigationTableView.idealHeight = textMetrics.height + 4 + scriptModuleNavigationTableView.idealHeight
                }
            }

            onHoveredRowChanged: scriptModuleNavigationDetailTimer.restart()

            onSelectedRowChanged: {
                positionViewAtRow(selectedRow, TableView.Contain, 0, Qt.rect(0, 0, 0, 0))
                scriptModuleNavigationDetailTimer.restart()
            }

            Timer {
                id: scriptModuleNavigationDetailTimer
                interval: 150

                onTriggered: {
                    var interestRow
                    if (scriptModuleNavigationTableView.hoveredRow !== -1) {
                        interestRow = scriptModuleNavigationTableView.hoveredRow
                    } else {
                        interestRow = scriptModuleNavigationTableView.selectedRow
                    }
                    scriptModuleNavigationToolTip.navigationWidget.detailReload(interestRow)
                    const index = scriptModuleNavigationTableView.index(interestRow, 0);
                    const item = scriptModuleNavigationTableView.itemAtIndex(index);
                    if (item) {
                        let idealY = item.mapToItem(scriptModuleNavigationTableView, 0, 0).y
                        idealY = Math.max(0, idealY)
                        idealY = Math.min(scriptModuleNavigationTableView.height - item.height, idealY)
                        scriptModuleNavigationDetailToolTip.y = idealY - 6
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
                target: scriptModuleNavigationTableView.model

                function onModelReset() {
                    scriptModuleNavigationTableView.idealWidth = 0
                    scriptModuleNavigationTableView.idealHeight = 0
                    scriptModuleNavigationDetailTimer.restart()
                }
            }
        }

        ToolTip {
            id: scriptModuleNavigationDetailToolTip
            x: scriptModuleNavigationToolTip.width - 5

            contentItem: Label {
                id: scriptModuleNavigationDetailLabel
                font: scriptModuleNavigationToolTip.font
                textFormat: Text.RichText
            }
        }
    }

    ToolTip {
        id: scriptModulePositionTooltip
        parent: Overlay.overlay
        x: position.x + 10; y: position.y + 10
        closePolicy: Popup.CloseOnPressOutside | Popup.CloseOnReleaseOutside
        property point position

        Behavior on x {
            enabled: scriptModulePositionTooltip.visible
            NumberAnimation {
                duration: 50
                easing.type: Easing.Linear
            }
        }
        Behavior on y {
            enabled: scriptModulePositionTooltip.visible
            NumberAnimation {
                duration: 50
                easing.type: Easing.Linear
            }
        }
    }

    ToolTip {
        id: scriptModuleSignatureToolTip
        parent: Overlay.overlay
        x: position.x - 6; y: position.y - implicitHeight
        closePolicy: Popup.CloseOnPressOutside | Popup.CloseOnReleaseOutside
        property point position

        onOpened: {
            mainWindow.overlayFocus(false)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1

        contentItem: Label {
            id: scriptModuleSignatureLabel
            textFormat: Text.RichText
        }
    }

    // status module
    Menu {
        id: statusModuleEolModeMenu
        property var eolModeButton
        property url scriptUrl
        property int eolMode

        onOpened: {
            mainWindow.overlayFocus(true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            statusModuleEolModeMenu.eolMode = scriptModule.eolModeGet(scriptUrl)
            statusModuleEolModeViewItem.checked = scriptModule.eolViewGet(scriptUrl)
        }

        MenuItem {
            text: "CRLF"
            enabled: statusModuleEolModeMenu.eolMode !== 0
            onTriggered: {
                scriptModule.eolModeSet(statusModuleEolModeMenu.scriptUrl, 0)
                statusModuleEolModeMenu.eolModeButton.text = "CRLF"
            }
        }

        MenuItem {
            text: "CR"
            enabled: statusModuleEolModeMenu.eolMode !== 1
            onTriggered: {
                scriptModule.eolModeSet(statusModuleEolModeMenu.scriptUrl, 1)
                statusModuleEolModeMenu.eolModeButton.text = "CR"
            }
        }

        MenuItem {
            text: "LF"
            enabled: statusModuleEolModeMenu.eolMode !== 2
            onTriggered: {
                scriptModule.eolModeSet(statusModuleEolModeMenu.scriptUrl, 2)
                statusModuleEolModeMenu.eolModeButton.text = "LF"
            }
        }

        MenuSeparator {
        }

        MenuItem {
            id: statusModuleEolModeViewItem
            checkable: true
            text: qsTr("View EOL")

            onTriggered: scriptModule.eolViewSet(statusModuleEolModeMenu.scriptUrl, statusModuleEolModeViewItem.checked)
        }
    }

    // structure module
    Menu {
        id: structureModuleRootMenu
        property var treeView

        onOpened: {
            mainWindow.overlayFocus(true)
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

    // system module
    Dialog {
        id: systemModuleRenameDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        title: qsTr("Rename")
        standardButtons: Dialog.Ok | Dialog.Cancel
        property string fileUrl

        onOpened: {
            mainWindow.overlayFocus(true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: {
            systemModuleRenameTextField.clear()
            systemModuleRenameTextField.forceActiveFocus()
            systemModuleRenameTextField.selectAll()
        }
        onAccepted: systemModule.fileRename(systemModuleRenameDialog.fileUrl, systemModuleRenameTextField.text)

        TextField {
            id: systemModuleRenameTextField
            width: parent.width
            placeholderText: qsTr("Enter new name:")

            onAccepted: systemModuleRenameDialog.accept()
            Keys.onEscapePressed: systemModuleRenameDialog.reject()
        }
    }

    Dialog {
        id: systemModulePermissionDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 600
        modal: true
        title: qsTr("Permission Management")
        standardButtons: Dialog.Ok | Dialog.Cancel
        property string fileUrl
        property bool readonly

        onOpened: {
            mainWindow.overlayFocus(true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAboutToShow: systemModulePermissionLabel.text = readonly ? qsTr("This file is writable. Would you like to make it read-only?") : qsTr("This file is read-only. Would you like to make it writable?")
        onAccepted: systemModule.filePermission(systemModulePermissionDialog.fileUrl, systemModulePermissionDialog.readonly)

        Label {
            id: systemModulePermissionLabel
            width: parent.width
            horizontalAlignment: Text.AlignLeft
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
            mainWindow.overlayFocus(true)
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
    }

    Menu {
        id: threadpoolModuleThreadMenu
        property string threadId

        onOpened: {
            mainWindow.overlayFocus(true)
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
            mainWindow.overlayFocus(true)
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
            mainWindow.overlayFocus(true)
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
            mainWindow.overlayFocus(true)
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
            mainWindow.overlayFocus(true)
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
            mainWindow.overlayFocus(true)
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

            "explorerModuleScriptMenu": explorerModuleScriptMenu,
            "explorerModuleFolderMenu": explorerModuleFolderMenu,
            "explorerModuleRootMenu": explorerModuleRootMenu,

            "logModuleHeightDialog": logModuleHeightDialog,
            "logModuleLinkMenu": logModuleLinkMenu,

            "menuModuleFileMenu": menuModuleFileMenu,
            "menuModuleEditMenu": menuModuleEditMenu,
            "menuModuleViewMenu": menuModuleViewMenu,
            "menuModuleCodeMenu": menuModuleCodeMenu,

            "portModuleTableMenu": portModuleTableMenu,
            "portModuleRootMenu": portModuleRootMenu,

            "scriptModuleEditorMenu": scriptModuleEditorMenu,
            "scriptModuleCompletionToolTip": scriptModuleCompletionToolTip,
            "scriptModuleCompletionTableView": scriptModuleCompletionTableView,
            "scriptModuleCompletionDetailTableView": scriptModuleCompletionDetailTableView,
            "scriptModuleDwellToolTip": scriptModuleDwellToolTip,
            "scriptModuleDwellDiagnosticTextArea": scriptModuleDwellDiagnosticTextArea,
            "scriptModuleDwellHoverTextArea": scriptModuleDwellHoverTextArea,
            "scriptModuleDwellCodeActionMenu": scriptModuleDwellCodeActionMenu,
            "scriptModuleDwellSuggestionMenu": scriptModuleDwellSuggestionMenu,
            "scriptModuleNavigationToolTip": scriptModuleNavigationToolTip,
            "scriptModuleNavigationTableView": scriptModuleNavigationTableView,
            "scriptModuleNavigationDetailLabel": scriptModuleNavigationDetailLabel,
            "scriptModulePositionTooltip": scriptModulePositionTooltip,
            "scriptModuleSignatureToolTip": scriptModuleSignatureToolTip,
            "scriptModuleSignatureLabel": scriptModuleSignatureLabel,

            "statusModuleEolModeMenu": statusModuleEolModeMenu,

            "structureModuleRootMenu": structureModuleRootMenu,

            "systemModulePermissionDialog": systemModulePermissionDialog,

            "threadpoolModuleErrorDialog": threadpoolModuleErrorDialog,
            "threadpoolModuleThreadMenu": threadpoolModuleThreadMenu,

            "watchModuleExpressionMenu": watchModuleExpressionMenu,
            "watchModuleValueMenu": watchModuleValueMenu,
            "watchModuleRootMenu": watchModuleRootMenu,
        };
        mainWindow.propertyGet(objects)
    }
}