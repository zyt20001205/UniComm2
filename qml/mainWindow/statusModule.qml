import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    height: 24

    RowLayout {
        anchors.fill: parent

        Button {
            flat: true
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            icon.source: "qrc:/icon/home.svg"
            icon.width: 16; icon.height: 16
            text: qsTr("Workspace")
            Layout.preferredHeight: 20

            onClicked: console.log("???")
        }

        Item {
            Layout.fillWidth: true
        }
    }

    function scriptPathLoad(pathList) {
        console.log(pathList)
    }
}