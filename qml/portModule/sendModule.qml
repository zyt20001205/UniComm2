import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    anchors.fill: parent

    RowLayout {
        anchors.fill: parent
        anchors.margins: 4

        ColumnLayout {
            Layout.alignment: Qt.AlignTop

            ComboBox {
                id: portNameCombobox
                model: standardItemModel
                textRole: "display"
                valueRole: "whatsThis"
            }

            TextField {
                id: portSendTextField
                Layout.fillWidth: true

                Keys.onReturnPressed: console.log("send")
                Keys.onEnterPressed: console.log("send")
            }
        }
    }
}