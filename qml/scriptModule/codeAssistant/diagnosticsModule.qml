import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.qmlmodels

Item {
    objectName: "diagnosticsRoot"
    anchors.fill: parent

    Item {
        anchors.fill: parent
        visible: tabBar.currentIndex === -1

        RowLayout {
            anchors.centerIn: parent

            Label {
                text: qsTr("No errors found.")
                font.pixelSize: 16
                Layout.alignment: Qt.AlignVCenter
            }

            Image {
                source: "qrc:/icon/checkmarkCircle.svg"
                Layout.alignment: Qt.AlignVCenter
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TabBar {
            id: tabBar
            contentHeight: 24
            Layout.fillWidth: true; Layout.preferredHeight: 32
        }
        StackLayout {
            id: stackLayout
            Layout.fillWidth: true; Layout.fillHeight: true
            currentIndex: tabBar.currentIndex
        }
    }

    Component {
        id: tabButtonComponent

        TabButton {
            id: tabButton
            width: contentItem.implicitWidth + 24; height: 24
            property var diagnosticsModel: null

            Connections {
                target: tabButton.diagnosticsModel
                enabled: tabButton.diagnosticsModel !== null

                function onRowsInserted() {
                    tabButton.width = contentItem.implicitWidth + 24
                    tabButton.visible = true
                    tabBar.currentIndex = tabButton.TabBar.index
                }

                function onModelReset() {
                    tabButton.width = 0
                    tabButton.visible = false
                    let showIndex = -1
                    for (let i = 0; i < tabBar.count; i++) {
                        const item = tabBar.itemAt(i)
                        if (item && item.visible) {
                            showIndex = i
                            break
                        }
                    }
                    tabBar.currentIndex = showIndex
                }
            }
        }
    }

    Component {
        id: pageComponent

        Item {
            id: pageItem
            Layout.fillWidth: true; Layout.fillHeight: true
            property var horizontalHeader: null
            property var diagnosticsModel: null

            HorizontalHeaderView {
                id: horizontalHeaderView
                anchors.left: parent.left; anchors.right: parent.right
                anchors.top: parent.top
                height: 32
                syncView: tableView
                clip: true

                delegate: HorizontalHeaderViewDelegate {
                    id: horizontalDelegate
                    required property int index

                    background: Rectangle {
                        color: "transparent"
                        border.width: 0
                    }

                    contentItem: Text {
                        anchors.fill: parent
                        clip: true
                        font.family: "Segoe UI"
                        font.pointSize: 10
                        horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                        text: pageItem.horizontalHeader[horizontalDelegate.index]
                    }
                }
            }

            TableView {
                id: tableView
                anchors.left: parent.left; anchors.right: parent.right
                anchors.top: horizontalHeaderView.bottom; anchors.bottom: parent.bottom
                alternatingRows: false
                clip: true
                editTriggers: TableView.NoEditTriggers
                rowSpacing: 1
                model: pageItem.diagnosticsModel
                property string diagnostic: ""
                property int viewrow: 0

                Rectangle {
                    anchors.fill: parent
                    color: "#e0e0e0"
                    z: -1
                }

                delegate: TableViewDelegate {
                    id: tableCell
                    implicitWidth: {
                        switch (tableCell.column) {
                            case 0:
                                return 24;
                            case 1:
                                return 160;
                            case 2:
                                return 100;
                            case 3:
                                return 60;
                            case 4:
                                return tableView.width - 344;
                            default:
                                return 0;
                        }
                    }
                    implicitHeight: 24

                    contentItem: Loader {
                        sourceComponent: {
                            switch (column) {
                                case 0:
                                    return iconDelegate;
                                default:
                                    return textDelegate;
                            }
                        }

                        Component {
                            id: iconDelegate

                            Rectangle {
                                anchors.fill: parent
                                color: "white"

                                Image {
                                    width: 16; height: 16
                                    anchors.centerIn: parent
                                    source: model.decoration
                                }
                            }
                        }

                        Component {
                            id: textDelegate

                            Rectangle {
                                anchors.fill: parent
                                color: "white"

                                Text {
                                    anchors.fill: parent
                                    z: 2
                                    clip: true
                                    font.family: "Segoe UI"
                                    font.pointSize: 10
                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter
                                    text: model.display

                                    ToolTip.visible: hoverHandler.hovered
                                    ToolTip.delay: 500
                                    ToolTip.text: qsTr("Line: %1\nClick to view details").arg(tableView.viewrowGet(tableCell.row) + 1)
                                }

                                Rectangle {
                                    id: highlightRect
                                    anchors.fill: parent
                                    z: 1
                                    radius: 2
                                    color: "#f5f5f5"
                                    opacity: hoverHandler.hovered ? 1 : 0
                                    Behavior on opacity {
                                        NumberAnimation {
                                            duration: 150
                                        }
                                    }
                                }
                            }
                        }
                    }

                    HoverHandler {
                        id: hoverHandler
                    }

                    TapHandler {
                        acceptedButtons: Qt.LeftButton
                        onTapped: {
                            tableView.indicatorInsert(tableCell.row)
                        }
                    }

                    TapHandler {
                        acceptedButtons: Qt.RightButton
                        onTapped: {
                            tableView.diagnostic = model.display
                            tableView.viewrow = tableCell.row
                            diagnosticMenu.popup()
                        }
                    }

                    Menu {
                        id: diagnosticMenu
                        MenuItem {
                            text: qsTr("Copy")
                            icon.source: "qrc:/icon/copy.svg"
                            icon.width: 16; icon.height: 16
                            onTriggered: diagnosticsModule.diagnosticCopy(tableView.diagnostic)
                        }
                        MenuItem {
                            text: qsTr("View")
                            icon.source: "qrc:/icon/eye.svg"
                            icon.width: 16; icon.height: 16
                            onTriggered: tableView.indicatorInsert(tableView.viewrow)
                        }
                    }
                }

                function viewrowGet(row) {
                    return model.data(model.index(row, 0), Qt.WhatsThisRole).startLine
                }

                function indicatorInsert(row) {
                    const index = model.index(row, 0);
                    diagnosticsModule.indicatorInsert(model.data(index, Qt.WhatsThisRole))
                }
            }
        }
    }

    function append(name, horizontalHeader, diagnosticsModel) {
        const tabButton = tabButtonComponent.createObject(tabBar, {
            "text": name,
            "diagnosticsModel": diagnosticsModel
        });
        const page = pageComponent.createObject(stackLayout, {
            "horizontalHeader": horizontalHeader,
            "diagnosticsModel": diagnosticsModel
        });
        tabBar.currentIndex = tabBar.count - 1
    }
}