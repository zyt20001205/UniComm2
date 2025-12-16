import QtQuick
import QtQuick.Controls

Item {
    anchors.centerIn: parent

    Dialog {
        objectName: "mainWindowCloseDialog"
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
}