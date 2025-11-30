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
            model: filterModel

            delegate: Item {
                implicitWidth: indicator.implicitWidth + icon.implicitWidth + text.implicitWidth
                implicitHeight: 24

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
                    anchors.verticalCenter: parent.verticalCenter
                    source: model.decoration
                }

                Image {
                    id: indicator
                    width: 24; height: 24
                    x: 16 + depth * 24
                    anchors.verticalCenter: parent.verticalCenter
                    visible: isTreeNode && hasChildren
                    source: expanded ? "qrc:/icon/arrowExpand.svg" : "qrc:/icon/arrowCollapse.svg"

                    TapHandler {
                        onSingleTapped: {
                            let index = treeView.index(row, column)
                            treeView.toggleExpanded(row)
                        }
                    }
                }

                Label {
                    id: text
                    x: indicator.visible ? indicator.x + 24 : 16 + depth * 24
                    anchors.verticalCenter: parent.verticalCenter
                    text: model.display
                }
            }
        }
    }
}
