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
        modal: true
        title: qsTr("Enter Condition")
        standardButtons: Dialog.Ok
        onOpened: mainWindow.overlayShow()
        onClosed: mainWindow.overlayHide()
        property string url
        property int line

        onAccepted: logModule.conditionSet(breakpointModuleConditionTextInput.test)
        onAboutToShow: {
            breakpointModuleConditionTextInput.text = breakpointModule.conditionGet(url, line)
            breakpointModuleConditionTextInput.forceActiveFocus()
        }

        TextInput {
            id: breakpointModuleConditionTextInput

            Keys.onReturnPressed: dialog.accept()
            Keys.onEnterPressed: dialog.accept()
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
            from: 1000
            to: 10000
            stepSize: 1000

            Keys.onReturnPressed: dialog.accept()
            Keys.onEnterPressed: dialog.accept()
        }
    }

    Component.onCompleted: {
        const objects = {
            "mainWindowCloseDialog": mainWindowCloseDialog,
            "breakpointModuleConditionDialog": breakpointModuleConditionDialog,
            "breakpointModuleLineMenu": breakpointModuleLineMenu,
            "breakpointModuleFileMenu": breakpointModuleFileMenu,
            "breakpointModuleRootMenu": breakpointModuleRootMenu,
            "logModuleHeightDialog": logModuleHeightDialog
        };
        mainWindow.propertyGet(objects)
    }
}