import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.qmlmodels

Item {
    objectName: "diagnosticsRoot"
    anchors.fill: parent

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
            width: contentItem.implicitWidth + 20; height: 24
        }
    }

    Component {
        id: pageComponent

        Item {
            Layout.fillWidth: true; Layout.fillHeight: true
            property var diagnostics: []

            HorizontalHeaderView {
                id: horizontalHeaderView
                anchors.left: parent.left; anchors.right: parent.right
                anchors.top: parent.top
                syncView: tableView
                clip: true
            }

            TableView {
                id: tableView
                anchors.left: parent.left; anchors.right: parent.right
                anchors.top: horizontalHeaderView.bottom; anchors.bottom: parent.bottom
                clip: true
                model: tableModel

                TableModel {
                    id: tableModel
                    TableModelColumn {
                        display: "severity"
                    }
                    TableModelColumn {
                        display: "source"
                    }
                    TableModelColumn {
                        display: "code"
                    }
                    TableModelColumn {
                        display: "data"
                    }
                    TableModelColumn {
                        display: "message"
                    }

                    Component.onCompleted: {
                        if (diagnostics.length > 0) {
                            for (var i = 0; i < diagnostics.length; i++) {
                                var diagnostic = diagnostics[i]
                                tableModel.appendRow({
                                    "severity": diagnostic.severity,
                                    "source": diagnostic.source,
                                    "code": diagnostic.code,
                                    "data": diagnostic.data,
                                    "message": diagnostic.message
                                })
                            }
                        }
                    }
                }

                delegate: Item {
                    implicitWidth: stackLayout.width / 5; implicitHeight: 24

                    RowLayout {
                        anchors.fill: parent

                        Text {
                            Layout.preferredWidth: 80; Layout.fillHeight: true
                            text: display
                        }
                    }
                }
            }
        }
    }

    function append(name, diagnostics) {
        var tabButton = tabButtonComponent.createObject(tabBar, {
            "text": name
        });
        var page = pageComponent.createObject(stackLayout, {
            "diagnostics": diagnostics
        });
    }
}