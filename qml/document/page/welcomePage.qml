import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: global.back
    }

    ColumnLayout {
        anchors.centerIn: parent

        Button {
            flat: true
            icon.source: "qrc:/icon/home.svg"
            text: qsTr("Open Workspace")
            font.pixelSize: 16

            onClicked: welcomePage.workspaceOpen()
        }

        Button {
            flat: true
            icon.source: "qrc:/icon/github.svg"
            text: qsTr("GitHub")
            font.pixelSize: 16

            onClicked: Qt.openUrlExternally("https://github.com/zyt20001205/UniComm")
        }
    }
}
