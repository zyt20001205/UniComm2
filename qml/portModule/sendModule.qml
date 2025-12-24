import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    anchors.fill: parent

    RowLayout {
        anchors.fill: parent

        ColumnLayout {

            ComboBox {
                id: portNameCombobox
                model: standardItemModel
                textRole: "display"
                valueRole: "whatsThis"
            }
        }
    }
}