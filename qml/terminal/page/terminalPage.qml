import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent

    ScrollView {
        anchors.fill: parent

        TextArea {
            id: textArea
            activeFocusOnTab: false
            textFormat: TextEdit.PlainText
            verticalAlignment: TextEdit.AlignTop
            property int basePosition: 0

            Keys.onPressed: (event) => {
                switch (event.key) {
                    case Qt.Key_Tab:{
                        event.accepted = true
                    }
                        break;
                    case Qt.Key_Enter:
                    case Qt.Key_Return: {
                        if (!(event.modifiers & Qt.ShiftModifier)) {
                            if (textArea.cursorPosition <= textArea.basePosition) {
                                event.accepted = true
                            }
                            const currentPos = textArea.cursorPosition
                            const input = textArea.getText(textArea.basePosition, currentPos)
                            terminalPage.terminalInput(input + '\n')
                            textArea.basePosition = currentPos
                            event.accepted = true
                        }
                    }
                        break;
                    case Qt.Key_Backspace: {
                        if (textArea.cursorPosition <= textArea.basePosition) {
                            event.accepted = true
                        }
                    }
                        break;
                    default:
                        break;
                }
            }
        }
    }

    function terminalOutput(output) {
        textArea.append(output)
        textArea.basePosition = textArea.cursorPosition
    }

    Component.onCompleted: {
        const objects = {
            "textArea": textArea
        };
        terminalPage.propertyGet(objects)
    }
}