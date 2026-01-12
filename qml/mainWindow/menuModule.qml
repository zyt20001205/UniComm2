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
                const pos = fileMenuButton.mapToItem(fileMenu.parent, 0, fileMenuButton.height);
                fileMenu.popup(pos.x, pos.y)
            }
        }

        Button {
            id: viewMenuButton
            Layout.preferredWidth: 48; Layout.preferredHeight: 24
            flat: true
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            text: "View"

            onClicked: {
                const pos = viewMenuButton.mapToItem(viewMenu.parent, 0, viewMenuButton.height);
                viewMenu.popup(pos.x, pos.y)
            }
        }

        Item {
            Layout.fillWidth: true; Layout.preferredHeight: 24
        }
    }
}