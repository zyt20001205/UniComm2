import QtQuick
import QtQuick.Controls

TreeView {
    id: structureTreeView
    anchors.fill: parent
    clip: true
    model: standardModel

    delegate: Item {
        required property TreeView treeView
        required property bool isTreeNode
        required property bool expanded
        required property bool hasChildren
        required property int depth
        required property int row
        required property int column

        implicitWidth: treeView.width; implicitHeight: 24

        Item {
            id: indicator
            width: (depth + 1) * 24; height: 24
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            Image {
                width: 16; height: 16
                anchors.right: parent.right
                anchors.rightMargin : 4
                anchors.verticalCenter: parent.verticalCenter
                visible: isTreeNode && hasChildren
                source: expanded ? "qrc:/icon/arrowExpand.svg" : "qrc:/icon/arrowCollapse.svg"

                TapHandler {
                    enabled: indicator.visible
                    onSingleTapped: {
                        treeView.toggleExpanded(row)
                    }
                }
            }
        }

        Item {
            id: icon
            width: 24; height: 24
            anchors.left: indicator.right
            anchors.verticalCenter: parent.verticalCenter

            Image {
                width: 16; height: 16
                anchors.centerIn: parent
                source: model.decoration
            }
        }

        Label {
            id: text
            anchors.left: icon.right; anchors.right: parent.right
            anchors.leftMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            text: model.display

            ToolTip.visible: hoverHandler.hovered
            ToolTip.delay: 500
            ToolTip.text: qsTr("Line: %1\nClick to view details").arg(model.whatsThis + 1)

            HoverHandler {
                id: hoverHandler
                cursorShape: Qt.PointingHandCursor
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onSingleTapped: {
                    structureModule.markerInsert(model.whatsThis)
                }
            }
        }
    }
}
