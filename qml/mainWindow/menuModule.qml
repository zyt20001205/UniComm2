import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent

    RowLayout {
        anchors.fill: parent

        Button {
            id: fileMenuButton
            Layout.preferredWidth: 48; Layout.preferredHeight: 24
            flat: true
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            text: "File"

            onClicked: {
                const globalPos = fileMenuButton.mapToGlobal(0, fileMenuButton.height);
                const localPos = fileMenu.parent.mapFromGlobal(globalPos.x, globalPos.y);
                fileMenu.popup(localPos.x, localPos.y)
            }
        }

        Button {
            id: viewMenuButton
            Layout.preferredWidth: 48; Layout.preferredHeight: 24
            flat: true
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            text: "View"

            onClicked: {
                const globalPos = viewMenuButton.mapToGlobal(0, viewMenuButton.height);
                const localPos = viewMenu.parent.mapFromGlobal(globalPos.x, globalPos.y);
                viewMenu.popup(localPos.x, localPos.y)
            }
        }

        Item {
            Layout.fillWidth: true; Layout.preferredHeight: 24
        }
    }
}