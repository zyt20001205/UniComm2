import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent

    RowLayout {
        anchors.fill: parent
        Layout.alignment: Qt.AlignLeft

        Button {
            Layout.preferredWidth: 60; Layout.preferredHeight: 24
            flat: true
            text: "File"

            onClicked: fileMenu.popup()
        }

        Button {
            Layout.preferredWidth: 60; Layout.preferredHeight: 24
            flat: true
            text: "View"

            onClicked: viewMenu.popup()
        }
    }
}