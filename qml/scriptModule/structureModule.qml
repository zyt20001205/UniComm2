import QtQuick
import QtQuick.Controls

Rectangle {
    anchors.fill: parent

    TreeView {
        id: structureTreeView
        anchors.fill: parent
        clip: true
        model: standardModel

        delegate: Item {
            implicitWidth: treeView.width; implicitHeight: 24

            required property TreeView treeView
            required property bool isTreeNode
            required property bool expanded
            required property bool hasChildren
            required property int depth
            required property int row
            required property int column
            required property bool current

            Image {
                id: icon
                width: 16; height: 16
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                source: model.decoration
            }

            Image {
                id: indicator
                width: 24; height: 24
                anchors.left: icon.right
                anchors.leftMargin: depth * 24
                anchors.verticalCenter: parent.verticalCenter
                visible: isTreeNode && hasChildren
                source: expanded ? "qrc:/icon/arrowExpand.svg" : "qrc:/icon/arrowCollapse.svg"

                TapHandler {
                    enabled: indicator.visible
                    onSingleTapped: {
                        let index = treeView.index(row, column)
                        treeView.toggleExpanded(row)
                    }
                }
            }

            Label {
                id: text
                height: 24
                anchors.left: indicator.right; anchors.right: parent.right
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
}
