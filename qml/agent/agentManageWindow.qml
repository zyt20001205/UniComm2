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

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal
        handle: Item {
            implicitWidth: 5

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 1
                color: global.stroke
            }
        }

        Rectangle {
            color: global.back
            SplitView.preferredWidth: 180; SplitView.fillHeight: true

            ListView {
                id: navigationView
                anchors.fill: parent
                anchors.margins: 10
                clip: true
                currentIndex: 0
                model: [
                    qsTr("Models"),
                    qsTr("MCP"),
                    qsTr("Skills"),
                    qsTr("Hooks"),
                    qsTr("Context")
                ]
                spacing: 2

                delegate: ItemDelegate {
                    required property int index
                    required property string modelData
                    width: navigationView.width
                    height: 36
                    text: modelData
                    highlighted: navigationView.currentIndex === index

                    onClicked: navigationView.currentIndex = index
                }
            }
        }

        StackLayout {
            currentIndex: navigationView.currentIndex
            SplitView.fillWidth: true; SplitView.fillHeight: true

            SettingsPage {
                title: qsTr("Models")
                description: qsTr("Configure model providers, credentials, and available models.")
            }

            SettingsPage {
                title: qsTr("MCP")
                description: qsTr("Add and manage MCP servers exposed to the agent.")
            }

            SettingsPage {
                title: qsTr("Skills")
                description: qsTr("Choose the skills loaded while preparing a turn.")
            }

            SettingsPage {
                title: qsTr("Hooks")
                description: qsTr("Configure scripts for agent lifecycle events.")
            }

            SettingsPage {
                title: qsTr("Context")
                description: qsTr("Manage context limits, token budgets, and compaction.")
            }
        }
    }

    component SettingsPage: Item {
        id: settingsPage
        property string title
        property string description

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 8

            Label {
                text: settingsPage.title
                font.pixelSize: 22
                font.bold: true
                Layout.fillWidth: true
            }

            Label {
                text: settingsPage.description
                color: global.stroke
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
