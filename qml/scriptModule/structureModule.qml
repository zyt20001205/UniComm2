import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true; Layout.preferredHeight: 30

            Button {
                id: functionButton
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                checkable: true
                icon.source: "qrc:/icon/symbolMethod.svg"
                icon.width: 16; icon.height: 16
            }

            Button {
                id: numberButton
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                checkable: true
                icon.source: "qrc:/icon/symbolNumeric.svg"
                icon.width: 16; icon.height: 16
            }

        }

        TreeView {
            id: structureTreeView
            Layout.fillWidth: true; Layout.fillHeight: true
            clip: true
            model: filterModel

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
                    anchors.left: parent.left
                    anchors.leftMargin: 16 + depth * 24
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
                    anchors.left: parent.left
                    anchors.leftMargin: 40 + depth * 24
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    text: model.display

                    TapHandler {
                        acceptedButtons: Qt.LeftButton
                        onSingleTapped: {
                            structureModule.markerInsert(row)
                        }
                    }

                    background: Rectangle {
                        color: "transparent"
                        border.color: "red"
                        border.width: 1
                    }
                }
            }
        }
    }
}
