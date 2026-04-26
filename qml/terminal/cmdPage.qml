import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent

    ColumnLayout{
        anchors.fill: parent

        ScrollView {
            Layout.fillWidth: true; Layout.fillHeight: true

            TextArea {
                id: textArea
                textFormat: TextEdit.PlainText
                verticalAlignment: TextEdit.AlignTop
            }
        }

        TextField {
            id: textField
            Layout.fillWidth: true

            onAccepted: {
                cmdPage.terminalInput(textField.text)
                textField.clear()
            }
        }
    }

    Component.onCompleted: {
        const objects = {
            "textArea": textArea,
            "textField": textField
        };
        cmdPage.propertyGet(objects)
    }
}