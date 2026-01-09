import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

TreeView {
    id: treeView
    anchors.fill: parent
    clip: true
    model: standardItemModel
    property int hoveredRow: -1
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
            color: treeView.selectedRow === row ? "#e0e0e0" : "transparent"
        }

        Rectangle {
            anchors.fill: parent
            radius: 6
            color: "#ebebeb"
            opacity: treeView.hoveredRow === row && treeView.selectedRow !== row ? 1 : 0
            Behavior on opacity {
                NumberAnimation {
                    duration: 150
                }
            }
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
    }

    HoverHandler {
        onPointChanged: treeView.hoveredRow = treeView.cellAtPosition(point.position).y
        onHoveredChanged: {
            if (!hovered) treeView.hoveredRow = -1
        }
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton

        onTapped: {
            treeView.selectedRow = treeView.cellAtPosition(point.position).y
            const index = treeView.index(treeView.selectedRow, 0)
            const item = treeView.itemAtIndex(index);
            structureModule.markerInsert(treeView.model.data(index, Qt.WhatsThisRole))
            if (item.isTreeNode && item.hasChildren) {
                treeView.toggleExpanded(treeView.selectedRow)
            }
        }
    }

    TapHandler {
        acceptedButtons: Qt.RightButton

        onTapped: {
            rootMenu.treeView = treeView
            rootMenu.popup()
        }
    }
}
