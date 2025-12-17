import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

TreeView {
    id: treeView
    anchors.fill: parent
    clip: true
    model: fileSystemModel
    rootIndex: fileRootIndex
    columnWidthProvider: function (col) {
        return col === 0 ? treeView.width : 0
    }

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

        Item {
            id: icon
            width: 24; height: 24
            anchors.left: indicator.right
            anchors.verticalCenter: parent.verticalCenter

            Image {
                width: 16; height: 16
                anchors.centerIn: parent
                source: isTreeNode && hasChildren ? "qrc:/icon/folder.svg" : "qrc:/icon/document.svg"
            }
        }

        Label {
            id: text
            anchors.left: icon.right; anchors.right: parent.right
            anchors.leftMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            text: model.fileName
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

        TapHandler {
            onDoubleTapped: explorerModule.scriptOpen(model.filePath)
        }

        TapHandler {
            acceptedButtons: Qt.RightButton
            gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds
            onSingleTapped: {
                if (isTreeNode && hasChildren) {
                    folderMenu.filePath = model.filePath
                    folderMenu.fileName = model.fileName
                    folderMenu.popup()
                } else {
                    scriptMenu.filePath = model.filePath
                    scriptMenu.fileName = model.fileName
                    scriptMenu.popup()
                }
            }
        }
    }

    TapHandler {
        acceptedButtons: Qt.RightButton
        onSingleTapped: {
            rootMenu.popup()
        }
    }
}
