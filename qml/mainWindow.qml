import QtQuick
import QtQuick.Controls

Item {
    anchors.centerIn: parent

    // overlay control
    property int widgetCount: 0

    onWidgetCountChanged: {
        // console.log("current count:", widgetCount);

        if (widgetCount === 0) {
            mainWindow.overlayActive(true)
        } else {
            mainWindow.overlayActive(false)
        }
    }

    // main window
    Dialog {
        id: mainWindowCloseDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        modal: true
        title: qsTr("Save and Exit?")
        standardButtons: Dialog.Yes | Dialog.No

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1
        onAccepted: mainWindow.quit()
    }

    ToolTip {
        id: mainTooltip
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
        property string url
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
        property string url
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
            text: qsTr("Delete")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16
            onTriggered: breakpointModule.breakpointDelete(breakpointModuleLineMenu.url, breakpointModuleLineMenu.line)
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
    }

    Menu {
        id: breakpointModuleFileMenu
        property string url

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Delete Breakpoints")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16
            onTriggered: breakpointModule.breakpointsDelete(breakpointModuleFileMenu.url)
        }
    }

    Menu {
        id: breakpointModuleRootMenu

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Delete All")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16
            onTriggered: breakpointModule.allDelete()
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
        id: explorerModuleScriptDeleteDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 400
        modal: true
        title: qsTr("Delete Script")
        standardButtons: Dialog.Ok | Dialog.Cancel
        property string filePath
        property string fileName

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1
        onAccepted: explorerModule.scriptDelete(explorerModuleScriptDeleteDialog.filePath)

        Label {
            text: qsTr('Are you sure to delete script "%1"?').arg(explorerModuleScriptDeleteDialog.fileName)
        }
    }

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
        id: explorerModuleFolderDeleteDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 400
        modal: true
        title: qsTr("Delete Folder")
        standardButtons: Dialog.Ok | Dialog.Cancel
        property string filePath
        property string fileName

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1
        onAccepted: explorerModule.folderDelete(explorerModuleFolderDeleteDialog.filePath)

        Label {
            text: qsTr('Are you sure to delete folder "%1" and all its contents?').arg(explorerModuleFolderDeleteDialog.fileName)
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

        MenuItem {
            text: qsTr("Open")
            icon.source: "qrc:/icon/open.svg"
            icon.width: 16; icon.height: 16
            onTriggered: explorerModule.scriptOpen(explorerModuleScriptMenu.filePath)
        }

        MenuItem {
            text: qsTr("Delete")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16
            onTriggered: {
                explorerModuleScriptDeleteDialog.filePath = explorerModuleScriptMenu.filePath
                explorerModuleScriptDeleteDialog.fileName = explorerModuleScriptMenu.fileName
                explorerModuleScriptDeleteDialog.open()
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
            text: qsTr("New Script")
            icon.source: "qrc:/icon/documentAdd.svg"
            icon.width: 16; icon.height: 16
            onTriggered: {
                explorerModuleScriptNewDialog.filePath = explorerModuleFolderMenu.filePath
                explorerModuleScriptNewDialog.open()
            }
        }

        MenuItem {
            text: qsTr("New Folder")
            icon.source: "qrc:/icon/folderAdd.svg"
            icon.width: 16; icon.height: 16
            onTriggered: {
                explorerModuleFolderNewDialog.filePath = explorerModuleFolderMenu.filePath
                explorerModuleFolderNewDialog.open()
            }
        }

        MenuItem {
            text: qsTr("Delete")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16
            onTriggered: {
                explorerModuleFolderDeleteDialog.filePath = explorerModuleFolderMenu.filePath
                explorerModuleFolderDeleteDialog.fileName = explorerModuleFolderMenu.fileName
                explorerModuleFolderDeleteDialog.open()
            }
        }
    }

    Menu {
        id: explorerModuleRootMenu

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("New Script")
            icon.source: "qrc:/icon/documentAdd.svg"
            icon.width: 16; icon.height: 16
            onTriggered: {
                explorerModuleScriptNewDialog.filePath = ""
                explorerModuleScriptNewDialog.open()
            }
        }

        MenuItem {
            text: qsTr("New Folder")
            icon.source: "qrc:/icon/folderAdd.svg"
            icon.width: 16; icon.height: 16
            onTriggered: {
                explorerModuleFolderNewDialog.filePath = ""
                explorerModuleFolderNewDialog.open()
            }
        }

        MenuItem {
            text: qsTr("Open In Explorer")
            icon.source: "qrc:/icon/open.svg"
            icon.width: 16; icon.height: 16
            onTriggered: explorerModule.openInExplorer()
        }
    }

    // log module
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
        property string url

        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1

        MenuItem {
            text: qsTr("Copy URL")
            icon.source: "qrc:/icon/copy.svg"
            icon.width: 16; icon.height: 16
            onTriggered: logModule.urlCopy(logModuleLinkMenu.url)
        }

        Menu {
            title: qsTr("Open In")
            icon.source: "qrc:/icon/open.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Explorer")
                icon.source: "qrc:/icon/folder.svg"
                icon.width: 16; icon.height: 16
                onTriggered: logModule.openInExplorer(logModuleLinkMenu.url)
            }

            MenuItem {
                text: qsTr("Application")
                icon.source: "qrc:/icon/apps.svg"
                icon.width: 16; icon.height: 16
                onTriggered: logModule.openInApplication(logModuleLinkMenu.url)
            }
        }
    }

    // script module
    Menu {
        id: scriptModuleEditorMenu
        property string scriptUrl
        property bool gotoMenu
        onAboutToShow: widgetCount += 1
        onClosed: widgetCount -= 1

        Menu {
            title: qsTr("Folding")

            MenuItem {
                text: qsTr("Collapse All")
                icon.source: "qrc:/icon/textCollapse.svg"
                icon.width: 16; icon.height: 16
                onTriggered: scriptModule.collapseAll(scriptModuleEditorMenu.scriptUrl)
            }

            MenuItem {
                text: qsTr("Expand All")
                icon.source: "qrc:/icon/textExpand.svg"
                icon.width: 16; icon.height: 16
                onTriggered: scriptModule.expandAll(scriptModuleEditorMenu.scriptUrl)
            }
        }

        Menu {
            title: qsTr("Goto")
            icon.source: "qrc:/icon/arrowRight.svg"
            icon.width: 16; icon.height: 16
            enabled: scriptModuleEditorMenu.gotoMenu

            MenuItem {
                text: qsTr("Definition(s)")
                icon.source: "qrc:/icon/definition.svg"
                icon.width: 8; icon.height: 8
                // onTriggered: scriptModule.collapseAll(scriptModuleEditorMenu.scriptUrl)
            }

            MenuItem {
                text: qsTr("References(s)")
                icon.source: "qrc:/icon/reference.svg"
                icon.width: 8; icon.height: 8
                // onTriggered: scriptModule.collapseAll(scriptModuleEditorMenu.scriptUrl)
            }

            MenuItem {
                text: qsTr("Implementation(s)")
                icon.source: "qrc:/icon/implementation.svg"
                icon.width: 8; icon.height: 8
                // onTriggered: scriptModule.collapseAll(scriptModuleEditorMenu.scriptUrl)
            }

            MenuItem {
                text: qsTr("Type Definition(s)")
                icon.source: "qrc:/icon/typeDefinition.svg"
                icon.width: 8; icon.height: 8
                // onTriggered: scriptModule.collapseAll(scriptModuleEditorMenu.scriptUrl)
            }
        }

        Menu {
            title: qsTr("Dock")
            icon.source: "qrc:/icon/dock.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Right")
                icon.source: "qrc:/icon/splitRight.svg"
                icon.width: 16; icon.height: 16
                // onTriggered: scriptModule.collapseAll(scriptModuleEditorMenu.scriptUrl)
            }

            MenuItem {
                text: qsTr("Left")
                icon.source: "qrc:/icon/splitLeft.svg"
                icon.width: 16; icon.height: 16
                // onTriggered: scriptModule.collapseAll(scriptModuleEditorMenu.scriptUrl)
            }

            MenuItem {
                text: qsTr("Top")
                icon.source: "qrc:/icon/splitUp.svg"
                icon.width: 16; icon.height: 16
                // onTriggered: scriptModule.collapseAll(scriptModuleEditorMenu.scriptUrl)
            }

            MenuItem {
                text: qsTr("Bottom")
                icon.source: "qrc:/icon/splitDown.svg"
                icon.width: 16; icon.height: 16
                // onTriggered: scriptModule.collapseAll(scriptModuleEditorMenu.scriptUrl)
            }
        }

        MenuItem {
            text: qsTr("Formatting")
            icon.source: "qrc:/icon/brush.svg"
            icon.width: 16; icon.height: 16
        }

        Menu {
            title: qsTr("Open In")
            icon.source: "qrc:/icon/open.svg"
            icon.width: 16; icon.height: 16

            MenuItem {
                text: qsTr("Explorer")
                icon.source: "qrc:/icon/folder.svg"
                icon.width: 16; icon.height: 16
                // onTriggered: logModule.openInExplorer(logModuleLinkMenu.url)
            }

            MenuItem {
                text: qsTr("Application")
                icon.source: "qrc:/icon/apps.svg"
                icon.width: 16; icon.height: 16
                // onTriggered: logModule.openInApplication(logModuleLinkMenu.url)
            }
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
            "mainTooltip": mainTooltip,

            "breakpointModuleLineMenu": breakpointModuleLineMenu,
            "breakpointModuleFileMenu": breakpointModuleFileMenu,
            "breakpointModuleRootMenu": breakpointModuleRootMenu,

            "diagnosticsModuleDiagnosticMenu": diagnosticsModuleDiagnosticMenu,

            "explorerModuleScriptErrorDialog": explorerModuleScriptErrorDialog,
            "explorerModuleFolderErrorDialog": explorerModuleFolderErrorDialog,
            "explorerModuleScriptMenu": explorerModuleScriptMenu,
            "explorerModuleFolderMenu": explorerModuleFolderMenu,
            "explorerModuleRootMenu": explorerModuleRootMenu,

            "logModuleHeightDialog": logModuleHeightDialog,
            "logModuleLinkMenu": logModuleLinkMenu,

            "scriptModuleEditorMenu": scriptModuleEditorMenu,

            "threadpoolModuleThreadMenu": threadpoolModuleThreadMenu
        };
        mainWindow.propertyGet(objects)
    }
}