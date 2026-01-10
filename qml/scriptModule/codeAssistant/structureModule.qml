import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent

    TreeView {
        id: treeView
        anchors.fill: parent
        clip: true
        model: standardItemModel
        property int selectedRow: -1

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        delegate: Item {
            implicitWidth: treeView.width; implicitHeight: 24
            required property TreeView treeView
            required property bool isTreeNode
            required property bool expanded
            required property bool hasChildren
            required property int depth
            required property int row

            Rectangle {
                anchors.fill: parent
                radius: 6
                color: "#ebebeb"
                opacity: hoverHandler.hovered ? 1 : 0
                Behavior on opacity {
                    NumberAnimation {
                        duration: 150
                    }
                }
            }

            Rectangle {
                anchors.fill: parent
                radius: 6
                color: treeView.selectedRow === row ? "#e0e0e0" : "transparent"
            }

            RowLayout {
                anchors.fill: parent
                spacing: 0

                Item {
                    Layout.preferredWidth: depth * 24; Layout.preferredHeight: 24
                }

                Item {
                    Layout.preferredWidth: 24; Layout.preferredHeight: 24

                    Image {
                        anchors.centerIn: parent
                        width: 16; height: 16
                        source: expanded ? "qrc:/icon/arrowExpand.svg" : "qrc:/icon/arrowCollapse.svg"
                        visible: isTreeNode && hasChildren
                    }
                }

                Item {
                    Layout.preferredWidth: 24; Layout.preferredHeight: 24

                    Image {
                        anchors.centerIn: parent
                        width: 16; height: 16
                        source: model.decoration
                    }
                }

                Label {
                    horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                    text: model.display
                    elide: Text.ElideRight
                    Layout.fillWidth: true; Layout.preferredHeight: 24
                }
            }

            HoverHandler {
                id: hoverHandler
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                onTapped: {
                    treeView.selectedRow = row
                    structureModule.markerInsert(treeView.model.data(treeView.index(row, 0), Qt.WhatsThisRole))
                    if (isTreeNode && hasChildren) {
                        treeView.toggleExpanded(row)
                    }
                }
            }

            TapHandler {
                acceptedButtons: Qt.RightButton
                gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                onTapped: {
                    rootMenu.treeView = treeView
                    rootMenu.popup()
                }
            }
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton

            onTapped: treeView.selectedRow = -1
        }

        TapHandler {
            acceptedButtons: Qt.RightButton

            onTapped: {
                rootMenu.treeView = treeView
                rootMenu.popup()
            }
        }
    }
}
