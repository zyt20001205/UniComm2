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
            property var diagnosticsModel: null

            HorizontalHeaderView {
                id: horizontalHeaderView
                anchors.top: parent.top
                width: parent.width
                height: 32
                syncView: tableView
                clip: true
                interactive: false

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
                        text: horizontalHeader[horizontalDelegate.index]
                    }
                }
            }

            TableView {
                id: tableView
                anchors.top: horizontalHeaderView.bottom; anchors.bottom: parent.bottom
                width: parent.width
                alternatingRows: false
                clip: true
                editTriggers: TableView.NoEditTriggers
                rowSpacing: 1
                model: pageItem.diagnosticsModel
                contentWidth: width
                property string diagnostic: ""
                property int viewrow: 0

                Rectangle {
                    anchors.fill: parent
                    color: "#e0e0e0"
                    z: -1
                }

                delegate: DelegateChooser {
                    DelegateChoice {
                        column: 0
                        delegate: iconCellDelegate
                    }
                    DelegateChoice {
                        delegate: textCellDelegate
                    }
                }

                Component {
                    id: iconCellDelegate

                    Rectangle {
                        implicitWidth: 24
                        implicitHeight: 24
                        color: "white"

                        Image {
                            width: 16; height: 16
                            anchors.centerIn: parent
                            source: model.decoration
                        }
                    }
                }

                Component {
                    id: textCellDelegate

                    Rectangle {
                        id: textCell
                        required property int column
                        required property int row
                        
                        implicitWidth: {
                            if (textCell.column === tableView.columns - 1) {
                                let usedWidth = 0
                                for (let i = 0; i < tableView.columns - 1; i++) {
                                    usedWidth += tableView.columnWidth(i)
                                }
                                return tableView.width - usedWidth
                            }
                            return Math.max(textMetrics.width + 16, 60)
                        }
                        implicitHeight: 24
                        color: "white"

                        TextMetrics {
                            id: textMetrics
                            font.family: "Segoe UI"
                            font.pointSize: 10
                            text: model.display || ""
                        }

                        Text {
                            anchors.fill: parent
                            z: 2
                            font.family: "Segoe UI"
                            font.pointSize: 10
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                            text: model.display
                            elide: Text.ElideRight

                            ToolTip.visible: hoverHandler.hovered
                            ToolTip.delay: 500
                            ToolTip.text: model.display
                        }

                        Rectangle {
                            id: highlightRect
                            anchors.fill: parent
                            z: 1
                            radius: 2
                            color: "#f5f5f5"
                            opacity: hoverHandler.hovered ? 1 : 0
                            Behavior on opacity {
                                NumberAnimation { duration: 150 }
                            }
                        }

                        HoverHandler {
                            id: hoverHandler
                        }

                        TapHandler {
                            acceptedButtons: Qt.LeftButton
                            onTapped: tableView.indicatorInsert(textCell.row)
                        }

                        TapHandler {
                            acceptedButtons: Qt.RightButton
                            onTapped: {
                                tableView.diagnostic = model.display
                                tableView.viewrow = textCell.row
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
                }

                function indicatorInsert(row) {
                    const index = model.index(row, 0);
                    diagnosticsModule.indicatorInsert(model.data(index, Qt.WhatsThisRole))
                }
            }
        }
    }

    function append(name, diagnosticsModel) {
        const tabButton = tabButtonComponent.createObject(tabBar, {
            "text": name,
            "diagnosticsModel": diagnosticsModel
        });
        const page = pageComponent.createObject(stackLayout, {
            "diagnosticsModel": diagnosticsModel
        });
    }
}