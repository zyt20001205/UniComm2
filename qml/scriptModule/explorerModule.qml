import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

TreeView {
    id: explorerTreeView
    anchors.fill: parent
    clip: true
    model: fileModel
    rootIndex: fileRootIndex
    columnWidthProvider: function (col) {
        return col === 0 ? explorerTreeView.width : 0
    }

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
                anchors.rightMargin: 4
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

        TapHandler {
            acceptedButtons: Qt.RightButton
            gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds
            onSingleTapped: {
                if (isTreeNode && hasChildren) {
                    folderMenu.popup()
                } else {
                    fileMenu.popup()
                }
            }
        }

        Menu {
            id: folderMenu
            MenuItem {
                text: qsTr("New Script")
                icon.source: "qrc:/icon/documentAdd.svg"
                onTriggered: {
                    explorerModule.scriptNew(model.filePath)
                }
            }
            MenuItem {
                text: qsTr("New Folder")
                icon.source: "qrc:/icon/folderAdd.svg"
                onTriggered: {
                    explorerModule.folderNew(model.filePath)
                }
            }
            MenuItem {
                MessageDialog {
                    id: folderDeleteDialog
                    text: qsTr("Are you sure to delete folder %1 and all its contents?").arg(model.fileName)
                    buttons: MessageDialog.Yes | MessageDialog.No
                    onAccepted: {
                        explorerModule.folderDelete(model.filePath)
                    }
                }

                text: qsTr("Delete Folder")
                icon.source: "qrc:/icon/delete.svg"
                onTriggered: {
                    folderDeleteDialog.open()
                }
            }
        }

        Menu {
            id: fileMenu
            MenuItem {
                text: qsTr("Run Script")
                icon.source: "qrc:/icon/play.svg"
                onTriggered: {
                    explorerModule.scriptRun(model.filePath)
                }
            }
            MenuItem {
                text: qsTr("Debug Script")
                icon.source: "qrc:/icon/bug.svg"
                onTriggered: {
                    explorerModule.scriptDebug(model.filePath)
                }
            }
            MenuItem {
                text: qsTr("Open Script")
                icon.source: "qrc:/icon/open.svg"
                onTriggered: {
                    explorerModule.scriptOpen(model.filePath)
                }
            }
            MenuItem {
                MessageDialog {
                    id: scriptDeleteDialog
                    text: qsTr("Are you sure to delete script %1?").arg(model.fileName)
                    buttons: MessageDialog.Yes | MessageDialog.No
                    onAccepted: {
                        explorerModule.scriptDelete(model.filePath)
                    }
                }

                text: qsTr("Delete Script")
                icon.source: "qrc:/icon/delete.svg"
                onTriggered: {
                    scriptDeleteDialog.open()
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

    Menu {
        id: rootMenu
        MenuItem {
            text: qsTr("New Script")
            icon.source: "qrc:/icon/documentAdd.svg"
            onTriggered: explorerModule.scriptNew()
        }
        MenuItem {
            text: qsTr("New Folder")
            icon.source: "qrc:/icon/folderAdd.svg"
            onTriggered: explorerModule.folderNew()
        }
        MenuItem {
            text: qsTr("Open In Explorer")
            icon.source: "qrc:/icon/open.svg"
            onTriggered: explorerModule.scriptOpenInExplorer()
        }
    }
}
