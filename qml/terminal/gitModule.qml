import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent
    property bool gitEnabled: false

    Item {
        anchors.fill: parent
        visible: !gitEnabled

        RowLayout {
            anchors.centerIn: parent

            Button {
                flat: true
                text: qsTr("Click to create git repository.")
                font.pixelSize: 16
                Layout.alignment: Qt.AlignVCenter

                onClicked: gitModule.gitInit()
            }

            Image {
                source: "qrc:/icon/github.svg"
                Layout.alignment: Qt.AlignVCenter
            }
        }
    }

    ScrollView {
        anchors.fill: parent
        visible: gitEnabled

        TextArea {
            id: textArea
            activeFocusOnTab: false
            text: ">>> "
            textFormat: TextEdit.PlainText
            verticalAlignment: TextEdit.AlignTop
            property int basePosition: 4

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
                            textArea.basePosition = currentPos
                            gitModule.terminalStdin(input + '\n')
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

    function terminalStdin(input) {
        textArea.cursorPosition = textArea.basePosition
        textArea.insert(textArea.basePosition, input)
        gitModule.terminalStdin(input + '\n')
    }

    function terminalStdout(output) {
        textArea.append(output)
        textArea.insert(textArea.cursorPosition, ">>> ")
        textArea.basePosition = textArea.cursorPosition
    }

    function terminalStderr(error) {
        textArea.append(error)
        textArea.insert(textArea.cursorPosition, ">>> ")
        textArea.basePosition = textArea.cursorPosition
    }

    Component.onCompleted: {
        const objects = {
            "textArea": textArea
        };
        gitModule.propertyGet(objects)
    }
}