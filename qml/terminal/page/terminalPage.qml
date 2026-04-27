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
                readOnly: true
                textFormat: TextEdit.PlainText
                verticalAlignment: TextEdit.AlignTop
            }
        }

        TextField {
            id: textField
            Layout.fillWidth: true

            onAccepted: {
                terminalPage.terminalInput(textField.text)
                textField.clear()
            }
        }
    }

    Component.onCompleted: {
        const objects = {
            "textArea": textArea,
            "textField": textField
        };
        terminalPage.propertyGet(objects)
    }
}