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

        onAccepted: logModule.heightWrite(spinBox.value)
        onAboutToShow: spinBox.forceActiveFocus()

        SpinBox {
            id: spinBox
            from: 1000
            to: 10000
            stepSize: 1000

            Keys.onReturnPressed: dialog.accept()
            Keys.onEnterPressed: dialog.accept()

            Component.onCompleted: value = logModule.heightRead()
        }
    }

    Component.onCompleted: {
        const objects = {
            "mainWindowCloseDialog": mainWindowCloseDialog,
            "logModuleHeightDialog": logModuleHeightDialog
        };
        mainWindow.propertyGet(objects)
    }
}