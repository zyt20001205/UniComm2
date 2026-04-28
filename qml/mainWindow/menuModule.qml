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
            id: editMenuButton
            Layout.preferredWidth: 48; Layout.preferredHeight: 24
            flat: true
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            text: "Edit"

            onClicked: {
                const globalPos = editMenuButton.mapToGlobal(0, editMenuButton.height);
                const localPos = editMenu.parent.mapFromGlobal(globalPos.x, globalPos.y);
                editMenu.popup(localPos.x, localPos.y)
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

        Button {
            id: navMenuButton
            Layout.preferredWidth: 48; Layout.preferredHeight: 24
            flat: true
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            text: "Nav"

            onClicked: {
                const globalPos = navMenuButton.mapToGlobal(0, navMenuButton.height);
                const localPos = navMenu.parent.mapFromGlobal(globalPos.x, globalPos.y);
                navMenu.popup(localPos.x, localPos.y)
            }
        }

        Button {
            id: codeMenuButton
            Layout.preferredWidth: 48; Layout.preferredHeight: 24
            flat: true
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            text: "Code"

            onClicked: {
                const globalPos = codeMenuButton.mapToGlobal(0, codeMenuButton.height);
                const localPos = codeMenu.parent.mapFromGlobal(globalPos.x, globalPos.y);
                codeMenu.popup(localPos.x, localPos.y)
            }
        }

        Button {
            id: execMenuButton
            Layout.preferredWidth: 48; Layout.preferredHeight: 24
            flat: true
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            text: "Exec"

            onClicked: {
                const globalPos = execMenuButton.mapToGlobal(0, execMenuButton.height);
                const localPos = execMenu.parent.mapFromGlobal(globalPos.x, globalPos.y);
                execMenu.popup(localPos.x, localPos.y)
            }
        }

        Button {
            id: gitMenuButton
            Layout.preferredWidth: 48; Layout.preferredHeight: 24
            flat: true
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            text: "Git"

            onClicked: {
                const globalPos = gitMenuButton.mapToGlobal(0, gitMenuButton.height);
                const localPos = gitMenu.parent.mapFromGlobal(globalPos.x, globalPos.y);
                gitMenu.popup(localPos.x, localPos.y)
            }
        }

        Item {
            Layout.fillWidth: true; Layout.preferredHeight: 24
        }
    }
}