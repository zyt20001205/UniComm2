import QtQuick
import QtQuick.Controls

Item {
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
        title: ""
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

            Keys.onReturnPressed: breakpointModuleConditionDialog.accept()
            Keys.onEnterPressed: breakpointModuleConditionDialog.accept()
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
        id: explorerModuleScriptErrorDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 400
        modal: true
        title: qsTr("Script Already Exists")
        standardButtons: Dialog.Ok

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1
    }

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
        onAccepted: explorerModule.scriptNew(explorerModuleScriptNewDialog.filePath, explorerModuleScriptNameTextField.text)

        TextField {
            id: explorerModuleScriptNameTextField
            width: parent.width
            placeholderText: qsTr("Enter script name:")

            Keys.onReturnPressed: explorerModuleScriptNewDialog.accept()
            Keys.onEnterPressed: explorerModuleScriptNewDialog.accept()
            Keys.onEscapePressed: explorerModuleScriptNewDialog.reject()
        }
    }

    Dialog {
        id: explorerModuleFolderErrorDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 400
        modal: true
        title: qsTr("Folder Already Exists")
        standardButtons: Dialog.Ok
        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1
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
        onAccepted: explorerModule.folderNew(explorerModuleFolderNewDialog.filePath, explorerModuleFolderNameTextField.text)

        TextField {
            id: explorerModuleFolderNameTextField
            width: parent.width
            placeholderText: qsTr("Enter folder name:")

            Keys.onReturnPressed: explorerModuleFolderNewDialog.accept()
            Keys.onEnterPressed: explorerModuleFolderNewDialog.accept()
            Keys.onEscapePressed: explorerModuleFolderNewDialog.reject()
        }
    }

    Menu {
        id: explorerModuleScriptMenu
        property string filePath
        property string fileName

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1

        Menu {
            title: qsTr("Fold")
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

        Menu {
            title: qsTr("Open In")
            icon.source: "qrc:/icon/open.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Explorer")
                icon.source: "qrc:/icon/folder.svg"
                icon.width: 16; icon.height: 16

                onTriggered: systemModule.resourceOpenInExplorer("file:///" + explorerModuleScriptMenu.filePath)
            }

            MenuItem {
                text: qsTr("Application")
                icon.source: "qrc:/icon/apps.svg"
                icon.width: 16; icon.height: 16

                onTriggered: systemModule.resourceOpenInApplication("file:///" + explorerModuleScriptMenu.filePath)
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
                    systemModule.resourceDelete("file:///" + explorerModuleScriptMenu.filePath)
                    progress = 0
                    explorerModuleScriptMenu.close()
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

        Menu {
            title: qsTr("Fold")
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
            text: qsTr("Open In Explorer")
            icon.source: "qrc:/icon/open.svg"
            icon.width: 16; icon.height: 16

            onTriggered: systemModule.resourceOpenInExplorer("file:///" + explorerModuleFolderMenu.filePath)
        }

        Menu {
            title: qsTr("Delete")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16

            DelayButton {
                delay: 1000
                text: qsTr("Confirm")

                onActivated: {
                    systemModule.resourceDelete("file:///" + explorerModuleFolderMenu.filePath)
                    progress = 0
                    explorerModuleFolderMenu.close()
                }
            }
        }
    }

    Menu {
        id: explorerModuleRootMenu
        property url rootUrl
        property var treeView

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1

        Menu {
            title: qsTr("Fold")
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

        Menu {
            title: qsTr("New")
            icon.source: "qrc:/icon/add.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Script")
                icon.source: "qrc:/icon/document.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    explorerModuleScriptNewDialog.filePath = ""
                    explorerModuleScriptNewDialog.open()
                }
            }

            MenuItem {
                text: qsTr("Folder")
                icon.source: "qrc:/icon/folder.svg"
                icon.width: 16; icon.height: 16

                onTriggered: {
                    explorerModuleFolderNewDialog.filePath = ""
                    explorerModuleFolderNewDialog.open()
                }
            }
        }

        MenuItem {
            text: qsTr("Open In Explorer")
            icon.source: "qrc:/icon/open.svg"
            icon.width: 16; icon.height: 16

            onTriggered: systemModule.resourceOpenInExplorer(explorerModuleRootMenu.rootUrl)
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

                onTriggered: systemModule.resourceOpenInExplorer(logModuleLinkMenu.url)
            }

            MenuItem {
                text: qsTr("Application")
                icon.source: "qrc:/icon/apps.svg"
                icon.width: 16; icon.height: 16

                onTriggered: systemModule.resourceOpenInApplication(logModuleLinkMenu.url)
            }
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
            enabled: scriptModuleEditorMenu.menuSession["gotoMenu"]

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

        Menu {
            title: qsTr("Open In")
            icon.source: "qrc:/icon/open.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Explorer")
                icon.source: "qrc:/icon/folder.svg"
                icon.width: 16; icon.height: 16

                onTriggered: systemModule.resourceOpenInExplorer(scriptModuleEditorMenu.scriptUrl)
            }

            MenuItem {
                text: qsTr("Application")
                icon.source: "qrc:/icon/apps.svg"
                icon.width: 16; icon.height: 16

                onTriggered: systemModule.resourceOpenInApplication(scriptModuleEditorMenu.scriptUrl)
            }
        }
    }

    // system module
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
        onAccepted: systemModule.resourceRename(systemModuleRenameDialog.fileUrl, systemModuleRenameTextField.text)

        TextField {
            id: systemModuleRenameTextField
            width: parent.width
            placeholderText: qsTr("Enter new name:")

            Keys.onReturnPressed: explorerModuleScriptNewDialog.accept()
            Keys.onEnterPressed: explorerModuleScriptNewDialog.accept()
            Keys.onEscapePressed: explorerModuleScriptNewDialog.reject()
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
            "mainWindowCloseDialog": mainWindowCloseDialog,
            "mainWindowBusyDialog": mainWindowBusyDialog,
            "mainWindowTooltip": mainWindowTooltip,

            "breakpointModuleLineMenu": breakpointModuleLineMenu,
            "breakpointModuleFileMenu": breakpointModuleFileMenu,
            "breakpointModuleRootMenu": breakpointModuleRootMenu,

            "diagnosticsModuleDiagnosticMenu": diagnosticsModuleDiagnosticMenu,

            "explorerModuleScriptErrorDialog": explorerModuleScriptErrorDialog,
            "explorerModuleFolderErrorDialog": explorerModuleFolderErrorDialog,
            "explorerModuleScriptMenu": explorerModuleScriptMenu,
            "explorerModuleFolderMenu": explorerModuleFolderMenu,
            "explorerModuleRootMenu": explorerModuleRootMenu,

            "logModuleEmptyDialog": logModuleEmptyDialog,
            "logModuleHeightDialog": logModuleHeightDialog,
            "logModuleLinkMenu": logModuleLinkMenu,

            "scriptModuleEditorMenu": scriptModuleEditorMenu,

            "threadpoolModuleThreadMenu": threadpoolModuleThreadMenu
        };
        mainWindow.propertyGet(objects)
    }
}