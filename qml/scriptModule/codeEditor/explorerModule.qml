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
        model: fileSystemModel
        rootIndex: modelRootIndex
        columnWidthProvider: function (col) {
            return col === 0 ? treeView.width : 0
        }
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
            required property int column

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
                        width: 20; height: 20
                        source: getIcon()

                        function getIcon() {
                            if (isTreeNode && hasChildren) {
                                return "qrc:/icon/folder.svg"
                            } else {
                                const suffix = model.fileName.split('.').pop()
                                switch (suffix) {
                                    case "csv":
                                        return "qrc:/icon/fileTypeCsv.svg"
                                    case "bmp":
                                    case "gif":
                                    case "ico":
                                    case "jpeg":
                                    case "jpg":
                                    case "png":
                                    case "svg":
                                    case "tif":
                                    case "tiff":
                                    case "webp":
                                        return "qrc:/icon/fileTypeImage.svg"
                                    case "json":
                                        return "qrc:/icon/fileTypeJson.svg"
                                    case "lua":
                                        return "qrc:/icon/fileTypeLua.svg"
                                    default:
                                        return "qrc:/icon/document.svg"
                                }
                            }
                        }
                    }
                }

                Label {
                    horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                    text: model.fileName
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
                    if (isTreeNode && hasChildren) {
                        treeView.toggleExpanded(row)
                    }
                }
                onDoubleTapped: {
                    if (!(isTreeNode && hasChildren)) {
                        explorerModule.scriptOpen(model.filePath)
                    }
                }
            }

            TapHandler {
                acceptedButtons: Qt.RightButton
                gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                onTapped: {
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
            acceptedButtons: Qt.LeftButton

            onTapped: treeView.selectedRow = -1
        }

        TapHandler {
            acceptedButtons: Qt.RightButton

            onTapped: {
                rootMenu.rootPath = modelRootPath
                rootMenu.treeView = treeView
                rootMenu.popup()
            }
        }
    }

    Component.onCompleted: {
        const objects = {
            "treeView": treeView
        };
        explorerModule.propertyGet(objects)
    }
}
