import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent
    property bool gitEnabled: false

    Rectangle {
        anchors.fill: parent
        color: global.back
    }

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

            IconImage {
                source: "qrc:/icon/fileTypeGit.svg"
                color: global.fore
                Layout.alignment: Qt.AlignVCenter
            }
        }
    }

    ScrollView {
        anchors.fill: parent
        visible: gitEnabled

        ScrollBar.vertical: ScrollBar {
            x: parent.mirrored ? 0 : parent.width - width
            y: parent.topPadding
            height: parent.availableHeight
            active: parent.ScrollBar.horizontal.active
            policy: ScrollBar.AsNeeded
            palette {
                mid: global.stroke
                dark: global.strokePressed
            }
        }

        ScrollBar.horizontal: ScrollBar {
            x: parent.leftPadding
            y: parent.height - height
            width: parent.availableWidth
            active: parent.ScrollBar.vertical.active
            policy: ScrollBar.AsNeeded
            palette {
                mid: global.stroke
                dark: global.strokePressed
            }
        }

        TextArea {
            id: textArea
            readOnly: true
            text: ">>> "
            textFormat: TextEdit.PlainText
            verticalAlignment: TextEdit.AlignTop
            ContextMenu.menu: null
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