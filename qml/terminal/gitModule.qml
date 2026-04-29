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
                source: "qrc:/icon/fileTypeGit.svg"
                Layout.alignment: Qt.AlignVCenter
            }
        }
    }

    ScrollView {
        anchors.fill: parent
        visible: gitEnabled

        TextArea {
            id: textArea
            readOnly: true
            text: ">>> "
            textFormat: TextEdit.PlainText
            verticalAlignment: TextEdit.AlignTop
        }
    }

    function terminalStdin(input) {
        textArea.insert(textArea.length, input)
    }

    function terminalStdout(output) {
        textArea.append(output)
    }

    function terminalStderr(error) {
        textArea.append(error)
    }

    function processFinished() {
        textArea.append(">>> ")
    }

    Component.onCompleted: {
        const objects = {
            "textArea": textArea
        };
        gitModule.propertyGet(objects)
    }
}