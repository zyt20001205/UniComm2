import QtQuick
import QtQuick.Controls

Item {
    anchors.centerIn: parent

    // main window
    Dialog {
        id: mainWindowCloseDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        modal: true
        title: qsTr("Save and Exit?")
        standardButtons: Dialog.Yes | Dialog.No
        onOpened: mainWindow.overlayShow()
        onClosed: mainWindow.overlayHide()

        onAccepted: {
            mainWindow.quit()
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
        onOpened: mainWindow.overlayShow()
        onClosed: mainWindow.overlayHide()
        property string url
        property int line

        onAccepted: breakpointModule.conditionSet(breakpointModuleConditionDialog.url, breakpointModuleConditionDialog.line, breakpointModuleConditionTextField.text)
        onAboutToShow: {
            breakpointModuleConditionTextField.text = breakpointModule.conditionGet(breakpointModuleConditionDialog.url, breakpointModuleConditionDialog.line)
            breakpointModuleConditionTextField.forceActiveFocus()
        }

        TextField {
            id: breakpointModuleConditionTextField
            width: parent.width

            Keys.onReturnPressed: breakpointModuleConditionDialog.accept()
            Keys.onEnterPressed: breakpointModuleConditionDialog.accept()
        }
    }

    Menu {
        id: breakpointModuleLineMenu
        onOpened: mainWindow.overlayShow()
        onClosed: mainWindow.overlayHide()
        property string url
        property int line

        MenuItem {
            text: qsTr("View Breakpoint")
            icon.source: "qrc:/icon/eye.svg"
            icon.width: 16; icon.height: 16
            onTriggered: breakpointModule.markerInsert(breakpointModuleLineMenu.url, breakpointModuleLineMenu.line)
        }
        MenuItem {
            text: qsTr("Delete Breakpoint")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16
            onTriggered: breakpointModule.breakpointDelete(breakpointModuleLineMenu.url, breakpointModuleLineMenu.line)
        }
        MenuItem {
            text: qsTr("Conditional Breakpoint")
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
        onOpened: mainWindow.overlayShow()
        onClosed: mainWindow.overlayHide()
        property string url

        MenuItem {
            text: qsTr("Delete Breakpoints")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16
            onTriggered: breakpointModule.breakpointsDelete(breakpointModuleFileMenu.url)
        }
    }

    Menu {
        id: breakpointModuleRootMenu
        onOpened: mainWindow.overlayShow()
        onClosed: mainWindow.overlayHide()

        MenuItem {
            text: qsTr("Delete All")
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16
            onTriggered: breakpointModule.allDelete()
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
        onOpened: mainWindow.overlayShow()
        onClosed: mainWindow.overlayHide()

        onAccepted: logModule.heightSet(logModuleHeightSpinBox.value)
        onAboutToShow: {
            logModuleHeightSpinBox.value = logModule.heightGet()
            logModuleHeightSpinBox.forceActiveFocus()
        }

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
        onOpened: mainWindow.overlayShow()
        onClosed: mainWindow.overlayHide()
        property string url

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

    Component.onCompleted: {
        const objects = {
            "mainWindowCloseDialog": mainWindowCloseDialog,
            "breakpointModuleConditionDialog": breakpointModuleConditionDialog,
            "breakpointModuleLineMenu": breakpointModuleLineMenu,
            "breakpointModuleFileMenu": breakpointModuleFileMenu,
            "breakpointModuleRootMenu": breakpointModuleRootMenu,
            "logModuleHeightDialog": logModuleHeightDialog,
            "logModuleLinkMenu": logModuleLinkMenu
        };
        mainWindow.propertyGet(objects)
    }
}