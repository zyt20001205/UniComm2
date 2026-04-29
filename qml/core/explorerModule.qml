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
        model: sortFilterProxyModel
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

            Rectangle {
                anchors.fill: parent
                radius: 6
                color: model.git ? model.git.indexStatus === 1 ? "#f7630c" : "transparent" : "transparent"
            }

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
                        source: model.source
                    }
                }

                Label {
                    horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                    color: model.git ? model.git.indexStatus === 0 ? "#c50f1f" :
                                model.git.indexStatus === 1 ? "#000000" :
                                model.git.workingTreeStatus === 2 ? "#000000" :
                                model.git.workingTreeStatus === 3 ? "#115ea3" :
                                "#000000" :
                        "#000000"
                    text: model.display
                    elide: Text.ElideRight
                    Layout.fillWidth: true; Layout.preferredHeight: 24
                }
            }

            HoverHandler {
                id: hoverHandler

                onHoveredChanged: {
                    if (!hovered) {
                        mainToolTip.text = ""
                    }
                }
                onPointChanged: {
                    mainToolTip.position = parent.mapToGlobal(point.position)
                    if (model.git) {
                        if (model.git.indexStatus === 0) {
                            mainToolTip.text = "Untracked"
                        } else if (model.git.indexStatus === 1) {
                            mainToolTip.text = "Ignored"
                        } else if (model.git.workingTreeStatus === 2) {
                            mainToolTip.text = "Unmodified"
                        } else if (model.git.workingTreeStatus === 3) {
                            mainToolTip.text = "Modified"
                        }
                    }
                }
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
                        explorerModule.documentOpen(model.documentUrl)
                    }
                }
            }

            TapHandler {
                acceptedButtons: Qt.RightButton
                gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                onTapped: {
                    if (isTreeNode && hasChildren) {
                        folderMenu.documentUrl = model.documentUrl
                        folderMenu.treeView = treeView
                        folderMenu.popup()
                    } else {
                        fileMenu.gitUntracked = model.git ? model.git.indexStatus === 0 : false
                        fileMenu.gitIgnored = model.git ? model.git.indexStatus === 1 : false
                        fileMenu.documentUrl = model.documentUrl
                        fileMenu.treeView = treeView
                        fileMenu.popup()
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
                rootMenu.documentUrl = modelRootUrl
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
