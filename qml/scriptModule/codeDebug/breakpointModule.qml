import QtQuick
import QtQuick.Controls

TreeView {
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

        Item {
            id: indicator
            width: (depth + 1) * 24; height: 24
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            Image {
                width: 16; height: 16
                anchors.right: parent.right
                anchors.rightMargin: 4
                anchors.verticalCenter: parent.verticalCenter
                visible: isTreeNode && hasChildren
                source: expanded ? "qrc:/icon/arrowExpand.svg" : "qrc:/icon/arrowCollapse.svg"

                TapHandler {
                    enabled: indicator.visible
                    onSingleTapped: treeView.toggleExpanded(row)
                }
            }
        }

        Label {
            id: text
            anchors.left: indicator.right; anchors.right: parent.right
            anchors.leftMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            text: model.display

            ToolTip.visible: hoverHandler.hovered
            ToolTip.delay: 500
            ToolTip.text: {
                if (isTreeNode && hasChildren) {
                    qsTr("Click to view file")
                } else{
                    qsTr("Click to view line")
                }
            }

            HoverHandler {
                id: hoverHandler
                cursorShape: Qt.PointingHandCursor
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onSingleTapped: {
                    if (isTreeNode && hasChildren) {
                        breakpointModule.scriptOpen(model.whatsThis)
                    } else {
                        breakpointModule.markerInsert(model.whatsThis, model.display)
                    }
                }
            }
        }

        Rectangle {
            id: highlightRect
            anchors.fill: parent
            z: -1
            radius: 2
            color: "#f5f5f5"
            opacity: 0
            Behavior on opacity {
                NumberAnimation {
                    duration: 150
                }
            }
        }

        HoverHandler {
            onHoveredChanged: {
                if (hovered) {
                    highlightRect.opacity = 1
                } else {
                    highlightRect.opacity = 0
                }
            }
        }
    }
}
