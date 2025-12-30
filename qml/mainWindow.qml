import QtQuick
import QtQuick.Controls

Item {
    id: mainItem
    anchors.centerIn: parent

    // overlay control
    property int widgetCount: 0

    onWidgetCountChanged: {
        // console.log("current count:", widgetCount);

        if (widgetCount === 0) {
            mainWindow.overlayTransparent(true)
        } else {
            mainWindow.overlayTransparent(false)
        }
    }

    // main window
    Dialog {
        id: mainWindowCloseDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 400
        modal: true
        title: qsTr("Save and Exit?")
        standardButtons: Dialog.Yes | Dialog.No

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1
        onAccepted: mainWindow.quit()
    }

    Dialog {
        id: mainWindowBusyDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 400
        modal: true
        standardButtons: Dialog.Abort
        topPadding: 30; bottomPadding: 20

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1
        onRejected: systemModule.processTerminate()

        ProgressBar {
            width: parent.width
            indeterminate: true
        }
    }

    Component {
        id: mainWindowMessageComponent

        Dialog {
            id: mainWindowMessageDialog
            parent: Overlay.overlay
            anchors.centerIn: parent
            width: 400
            modal: true
            standardButtons: Dialog.Ok
            property string text
            topPadding: 30; bottomPadding: 20

            onAboutToShow: widgetCount += 1
            onClosed: {
                widgetCount -= 1
                destroy()
            }

            Label {
                text: mainWindowMessageDialog.text
            }
        }
    }

    function messageDialogNew(title, text, eventloop) {
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
        id: mainWindowTooltip
        onOpened: {
            mainWindow.overlayPenetrate(true)
            mainWindow.overlayShow()
        }
        onClosed: {
            mainWindow.overlayPenetrate(false)
            mainWindow.overlayHide()
        }
    }

    // breakpoint module
    Dialog {
        id: breakpointModuleConditionDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 400
        modal: true
        title: qsTr("Enter Condition")
        standardButtons: Dialog.Ok
        property url url
        property int line

        onAboutToShow: {
            breakpointModuleConditionTextField.text = breakpointModule.conditionGet(breakpointModuleConditionDialog.url, breakpointModuleConditionDialog.line)
            breakpointModuleConditionTextField.forceActiveFocus()
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAccepted: breakpointModule.conditionSet(breakpointModuleConditionDialog.url, breakpointModuleConditionDialog.line, breakpointModuleConditionTextField.text)

        TextField {
            id: breakpointModuleConditionTextField
            width: parent.width
            placeholderText: qsTr("true")

            onAccepted: breakpointModuleConditionDialog.accept()
            Keys.onEscapePressed: breakpointModuleConditionDialog.reject()
        }
    }

    Menu {
        id: breakpointModuleLineMenu
        property url url
        property int line

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("View")
            icon.source: "qrc:/icon/eye.svg"
            icon.width: 16; icon.height: 16

            onTriggered: breakpointModule.markerInsert(breakpointModuleLineMenu.url, breakpointModuleLineMenu.line)
        }

        MenuItem {
            text: qsTr("Condition")
            icon.source: "qrc:/icon/equalCircle.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                breakpointModuleConditionDialog.url = breakpointModuleLineMenu.url
                breakpointModuleConditionDialog.line = breakpointModuleLineMenu.line
                breakpointModuleConditionDialog.open()
            }
        }

        MenuItem {
            text: qsTr("Delete")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16

            onTriggered: breakpointModule.breakpointDelete(breakpointModuleLineMenu.url, breakpointModuleLineMenu.line)
        }
    }

    Menu {
        id: breakpointModuleFileMenu
        property url url

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1

        Menu {
            title: qsTr("Delete Breakpoints")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16

            DelayButton {
                delay: 1000
                text: qsTr("Confirm")

                onActivated: {
                    breakpointModule.breakpointsDelete(breakpointModuleFileMenu.url)
                    progress = 0
                    breakpointModuleFileMenu.close()
                }
            }
        }
    }

    Menu {
        id: breakpointModuleRootMenu

        onAboutToShow: widgetCount += 1
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
    }

    // database module
    Dialog {
        id: databaseModuleNameDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 400
        modal: true
        title: qsTr("Enter Key Name")
        standardButtons: Dialog.Ok
        property int databaseIndex
        property string databaseKey

        onAboutToShow: {
            databaseModuleNameTextField.text = databaseModuleNameDialog.databaseKey
            databaseModuleNameTextField.forceActiveFocus()
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAccepted: {
            if (databaseModuleNameDialog.databaseKey) {
                databaseModule.databaseRename(databaseModuleNameDialog.databaseIndex, databaseModuleNameTextField.text)
            } else {
                databaseModule.databaseInsert(databaseModuleNameDialog.databaseIndex, databaseModuleNameTextField.text)
            }
        }

        TextField {
            id: databaseModuleNameTextField
            width: parent.width
            placeholderText: qsTr("Enter key:")

            onAccepted: databaseModuleNameDialog.accept()
            Keys.onEscapePressed: databaseModuleNameDialog.reject()
        }
    }

    Menu {
        id: databaseModuleTableMenu
        property int databaseIndex
        property string databaseKey

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Insert")
            icon.source: "qrc:/icon/add.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                databaseModuleNameDialog.databaseIndex = databaseModuleTableMenu.databaseIndex
                databaseModuleNameDialog.databaseKey = ""
                databaseModuleNameDialog.open()
            }
        }

        MenuItem {
            text: qsTr("Rename")
            icon.source: "qrc:/icon/rename.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                databaseModuleNameDialog.databaseIndex = databaseModuleTableMenu.databaseIndex
                databaseModuleNameDialog.databaseKey = databaseModuleTableMenu.databaseKey
                databaseModuleNameDialog.open()
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

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("New")
            icon.source: "qrc:/icon/add.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                databaseModuleNameDialog.databaseIndex = -1
                databaseModuleNameDialog.databaseKey = ""
                databaseModuleNameDialog.open()
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
        id: datatableModuleNameDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 400
        modal: true
        title: qsTr("Enter Key Name")
        standardButtons: Dialog.Ok
        property int datatableIndex
        property string datatableKey

        onAboutToShow: {
            datatableModuleNameTextField.text = datatableModuleNameDialog.datatableKey
            datatableModuleNameTextField.forceActiveFocus()
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAccepted: {
            if (datatableModuleNameDialog.datatableKey) {
                datatableModule.datatableRename(datatableModuleNameDialog.datatableIndex, datatableModuleNameTextField.text)
            } else {
                datatableModule.datatableInsert(datatableModuleNameDialog.datatableIndex, datatableModuleNameTextField.text)
            }
        }

        TextField {
            id: datatableModuleNameTextField
            width: parent.width
            placeholderText: qsTr("Enter key:")

            onAccepted: datatableModuleNameDialog.accept()
            Keys.onEscapePressed: datatableModuleNameDialog.reject()
        }
    }

    Menu {
        id: datatableModuleTableMenu
        property int datatableIndex
        property string datatableKey

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Insert")
            icon.source: "qrc:/icon/add.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                datatableModuleNameDialog.datatableIndex = datatableModuleTableMenu.datatableIndex
                datatableModuleNameDialog.datatableKey = ""
                datatableModuleNameDialog.open()
            }
        }

        MenuItem {
            text: qsTr("Rename")
            icon.source: "qrc:/icon/rename.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                datatableModuleNameDialog.datatableIndex = datatableModuleTableMenu.datatableIndex
                datatableModuleNameDialog.datatableKey = datatableModuleTableMenu.datatableKey
                datatableModuleNameDialog.open()
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

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("New")
            icon.source: "qrc:/icon/add.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                datatableModuleNameDialog.datatableIndex = -1
                datatableModuleNameDialog.datatableKey = ""
                datatableModuleNameDialog.open()
            }
        }

        MenuItem {
            text: qsTr("Export")
            icon.source: "qrc:/icon/share.svg"
            icon.width: 16; icon.height: 16

            onTriggered: datatableModule.datatableExport()
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

    // debug module

    // diagnostics module
    Menu {
        id: diagnosticsModuleDiagnosticMenu
        property string diagnostic
        property var position

        onAboutToShow: widgetCount += 1
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

            onTriggered: diagnosticsModule.indicatorInsert(diagnosticsModuleDiagnosticMenu.position)
        }
    }

    // explorer module
    Dialog {
        id: explorerModuleScriptNewDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 400
        modal: true
        title: qsTr("New Script")
        standardButtons: Dialog.Ok | Dialog.Cancel
        property string filePath

        onAboutToShow: {
            explorerModuleScriptNameTextField.clear()
            explorerModuleScriptNameTextField.forceActiveFocus()
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
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
        width: 400
        modal: true
        title: qsTr("New Folder")
        standardButtons: Dialog.Ok | Dialog.Cancel
        property string filePath

        onAboutToShow: {
            explorerModuleFolderNameTextField.clear()
            explorerModuleFolderNameTextField.forceActiveFocus()
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
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

        onAboutToShow: widgetCount += 1
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
                    const treeView = explorerModuleRootMenu.treeView
                    for (let i = 0; i < treeView.rows; i++) {
                        treeView.collapseRecursively(i)
                    }
                }
            }

            MenuItem {
                text: qsTr("Expand All")
                icon.source: "qrc:/icon/expand.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    const treeView = explorerModuleRootMenu.treeView
                    for (let i = 0; i < treeView.rows; i++) {
                        treeView.expandRecursively(i)
                    }
                }
            }
        }
    }

    Menu {
        id: explorerModuleFolderMenu
        property string filePath
        property string fileName

        onAboutToShow: widgetCount += 1
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
                    const treeView = explorerModuleRootMenu.treeView
                    for (let i = 0; i < treeView.rows; i++) {
                        treeView.collapseRecursively(i)
                    }
                }
            }

            MenuItem {
                text: qsTr("Expand All")
                icon.source: "qrc:/icon/expand.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    const treeView = explorerModuleRootMenu.treeView
                    for (let i = 0; i < treeView.rows; i++) {
                        treeView.expandRecursively(i)
                    }
                }
            }
        }
    }

    Menu {
        id: explorerModuleRootMenu
        property string rootPath
        property var treeView

        onAboutToShow: widgetCount += 1
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
                    const treeView = explorerModuleRootMenu.treeView
                    for (let i = 0; i < treeView.rows; i++) {
                        treeView.collapseRecursively(i)
                    }
                }
            }

            MenuItem {
                text: qsTr("Expand All")
                icon.source: "qrc:/icon/expand.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    const treeView = explorerModuleRootMenu.treeView
                    for (let i = 0; i < treeView.rows; i++) {
                        treeView.expandRecursively(i)
                    }
                }
            }
        }
    }

    // log module
    Dialog {
        id: logModuleEmptyDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 400
        modal: true
        title: qsTr("Log Is Empty")
        standardButtons: Dialog.Ok

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1
    }

    Dialog {
        id: logModuleHeightDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 400
        modal: true
        title: qsTr("Set Max Line Count")
        standardButtons: Dialog.Ok | Dialog.Cancel

        onAboutToShow: {
            logModuleHeightSpinBox.value = logModule.heightGet()
            logModuleHeightSpinBox.forceActiveFocus()
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
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

        onAboutToShow: widgetCount += 1
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

    // port module
    Menu {
        id: portModuleTableMenu
        property int portIndex

        onAboutToShow: widgetCount += 1
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

        onAboutToShow: widgetCount += 1
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
        property url scriptUrl
        property var menuSession
        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1

        Menu {
            title: qsTr("Goto")
            icon.source: "qrc:/icon/arrowRight.svg"
            icon.width: 16; icon.height: 16
            enabled: scriptModuleEditorMenu.menuSession ? scriptModuleEditorMenu.menuSession["gotoMenu"] : false

            MenuItem {
                text: qsTr("Definition(s)")
                icon.source: "qrc:/icon/definition.svg"
                icon.width: 8; icon.height: 8

                onTriggered: scriptModule.definitionRequest(scriptModuleEditorMenu.scriptUrl, scriptModuleEditorMenu.menuSession["line"], scriptModuleEditorMenu.menuSession["index"])
            }

            MenuItem {
                text: qsTr("References(s)")
                icon.source: "qrc:/icon/reference.svg"
                icon.width: 8; icon.height: 8

                onTriggered: scriptModule.referencesRequest(scriptModuleEditorMenu.scriptUrl, scriptModuleEditorMenu.menuSession["line"], scriptModuleEditorMenu.menuSession["index"])
            }

            MenuItem {
                text: qsTr("Implementation(s)")
                icon.source: "qrc:/icon/implementation.svg"
                icon.width: 8; icon.height: 8

                onTriggered: scriptModule.implementationRequest(scriptModuleEditorMenu.scriptUrl, scriptModuleEditorMenu.menuSession["line"], scriptModuleEditorMenu.menuSession["index"])
            }

            MenuItem {
                text: qsTr("Type Definition(s)")
                icon.source: "qrc:/icon/typeDefinition.svg"
                icon.width: 8; icon.height: 8

                onTriggered: scriptModule.typeDefinitionRequest(scriptModuleEditorMenu.scriptUrl, scriptModuleEditorMenu.menuSession["line"], scriptModuleEditorMenu.menuSession["index"])
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

                onTriggered: scriptModule.collapseAll(scriptModuleEditorMenu.scriptUrl)
            }

            MenuItem {
                text: qsTr("Expand All")
                icon.source: "qrc:/icon/expand.svg"
                icon.width: 16; icon.height: 16

                onTriggered: scriptModule.expandAll(scriptModuleEditorMenu.scriptUrl)
            }
        }

        MenuItem {
            text: qsTr("Formatting")
            icon.source: "qrc:/icon/brush.svg"
            icon.width: 16; icon.height: 16

            onTriggered: scriptModule.formattingRequest(scriptModuleEditorMenu.scriptUrl)
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

                onTriggered: systemModule.fileOpenInExplorer(scriptModuleEditorMenu.scriptUrl)
            }

            MenuItem {
                text: qsTr("Application")
                icon.source: "qrc:/icon/apps.svg"
                icon.width: 16; icon.height: 16

                onTriggered: systemModule.fileOpenInApplication(scriptModuleEditorMenu.scriptUrl)
            }
        }
    }

    // system module
    Dialog {
        id: systemModuleErrorDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 400
        modal: true
        standardButtons: Dialog.Ok

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1
    }

    Dialog {
        id: systemModuleRenameDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 400
        modal: true
        title: qsTr("Rename")
        standardButtons: Dialog.Ok | Dialog.Cancel
        property string fileUrl

        onAboutToShow: {
            systemModuleRenameTextField.clear()
            systemModuleRenameTextField.forceActiveFocus()
            widgetCount += 1
        }
        onClosed: widgetCount -= 1
        onAccepted: systemModule.fileRename(systemModuleRenameDialog.fileUrl, systemModuleRenameTextField.text)

        TextField {
            id: systemModuleRenameTextField
            width: parent.width
            placeholderText: qsTr("Enter new name:")

            onAccepted: systemModuleRenameDialog.accept()
            Keys.onEscapePressed: systemModuleRenameDialog.reject()
        }
    }

    // threadpool module
    Menu {
        id: threadpoolModuleThreadMenu
        property string threadId

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Terminate")
            icon.source: "qrc:/icon/stop.svg"
            icon.width: 16; icon.height: 16

            onTriggered: threadpoolModule.threadStop(threadpoolModuleThreadMenu.threadId)
        }
    }

    Component.onCompleted: {
        const objects = {
            "mainItem": mainItem,
            "mainWindowCloseDialog": mainWindowCloseDialog,
            "mainWindowBusyDialog": mainWindowBusyDialog,
            "mainWindowTooltip": mainWindowTooltip,

            "breakpointModuleLineMenu": breakpointModuleLineMenu,
            "breakpointModuleFileMenu": breakpointModuleFileMenu,
            "breakpointModuleRootMenu": breakpointModuleRootMenu,

            "databaseModuleNameDialog": databaseModuleNameDialog,
            "databaseModuleTableMenu": databaseModuleTableMenu,
            "databaseModuleRootMenu": databaseModuleRootMenu,

            "datatableModuleNameDialog": datatableModuleNameDialog,
            "datatableModuleTableMenu": datatableModuleTableMenu,
            "datatableModuleRootMenu": datatableModuleRootMenu,

            "diagnosticsModuleDiagnosticMenu": diagnosticsModuleDiagnosticMenu,

            "explorerModuleScriptMenu": explorerModuleScriptMenu,
            "explorerModuleFolderMenu": explorerModuleFolderMenu,
            "explorerModuleRootMenu": explorerModuleRootMenu,

            "logModuleEmptyDialog": logModuleEmptyDialog,
            "logModuleHeightDialog": logModuleHeightDialog,
            "logModuleLinkMenu": logModuleLinkMenu,

            "portModuleTableMenu": portModuleTableMenu,
            "portModuleRootMenu": portModuleRootMenu,

            "scriptModuleEditorMenu": scriptModuleEditorMenu,

            "systemModuleErrorDialog": systemModuleErrorDialog,

            "threadpoolModuleThreadMenu": threadpoolModuleThreadMenu
        };
        mainWindow.propertyGet(objects)
    }
}