import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: global.back
    }

    Item {
        anchors.fill: parent
        visible: !global.git

        RowLayout {
            anchors.centerIn: parent

            Button {
                flat: true
                text: qsTr("Click to create git repository.")
                font.pixelSize: 16
                Layout.alignment: Qt.AlignVCenter

                onClicked: gitModule.gitInit()
            }

            IconImage {
                source: "qrc:/icon/fileTypeGit.svg"
                color: global.fore
                Layout.alignment: Qt.AlignVCenter
            }
        }
    }

    RowLayout {
        anchors.fill: parent

        TreeView {
            id: treeView
            clip: true
            model: standardItemModel
            visible: global.git
            property int selectedRow: -1
            Layout.preferredWidth: 200; Layout.fillHeight: true

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                palette {
                    mid: global.stroke
                    dark: global.strokePressed
                }
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
                    color: global.backHover
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
                    color: treeView.selectedRow === row ? global.backSelected : "transparent"
                }

                RowLayout {
                    anchors.fill: parent
                    spacing: 0

                    Item {
                        Layout.preferredWidth: depth * 24; Layout.preferredHeight: 24
                    }

                    Item {
                        Layout.preferredWidth: 24; Layout.preferredHeight: 24

                        IconImage {
                            anchors.centerIn: parent
                            width: 16; height: 16
                            source: expanded ? "qrc:/icon/arrowExpand.svg" : "qrc:/icon/arrowCollapse.svg"
                            color: global.fore
                            visible: isTreeNode && hasChildren
                        }
                    }

                    Item {
                        Layout.preferredWidth: 24; Layout.preferredHeight: 24

                        IconImage {
                            anchors.centerIn: parent
                            width: 16; height: 16
                            source: model.type === "local" ? "qrc:/icon/tcpClient.svg" :
                                    model.type === "remote" ? "qrc:/icon/tcpServer.svg" :
                                    model.type === "current" ? "qrc:/icon/tag.svg" :
                                    "qrc:/icon/gitBranch.svg"
                            color: ["favourite", "current"].includes(model.type) ? global.warningFore3 : global.fore
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
                        if (isTreeNode && hasChildren) {
                            treeView.toggleExpanded(row)
                        } else {
                        }
                    }
                }

                TapHandler {
                    acceptedButtons: Qt.RightButton
                    gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                    onTapped: {
                        if (!(isTreeNode && hasChildren)) {
                            branchMenu.name = model.display
                            branchMenu.current = model.type === "current"
                            branchMenu.popup()
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
                    // rootMenu.treeView = treeView
                    // rootMenu.popup()
                }
            }
        }

        ScrollView {
            visible: global.git
            Layout.fillWidth: true; Layout.fillHeight: true

            ScrollBar.vertical: ScrollBar {
                x: parent.mirrored ? 0 : parent.width - width
                y: parent.topPadding
                height: parent.availableHeight
                active: parent.ScrollBar.horizontal.active
                policy: ScrollBar.AsNeeded
                palette {
                    mid: global.stroke
                    dark: global.strokePressed
                }
            }

            ScrollBar.horizontal: ScrollBar {
                x: parent.leftPadding
                y: parent.height - height
                width: parent.availableWidth
                active: parent.ScrollBar.vertical.active
                policy: ScrollBar.AsNeeded
                palette {
                    mid: global.stroke
                    dark: global.strokePressed
                }
            }

            TextArea {
                id: textArea
                readOnly: true
                text: ">>> "
                textFormat: TextEdit.PlainText
                verticalAlignment: TextEdit.AlignTop
                ContextMenu.menu: null
            }
        }
    }

    function terminalStdin(input) {
        textArea.insert(textArea.length, input)
    }

    function terminalStdout(output) {
        textArea.append(output)
    }

    function terminalStderr(error) {
        textArea.append(error)
    }

    function processFinished() {
        textArea.append(">>> ")
    }

    function branchExpand() {
        for (let i = 0; i < treeView.rows; ++i) {
            treeView.expandRecursively(i)
        }
    }

    Component.onCompleted: {
        const objects = {
            "textArea": textArea
        };
        gitModule.propertyGet(objects)
    }
}