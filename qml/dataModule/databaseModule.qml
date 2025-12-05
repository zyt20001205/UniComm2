import QtQuick
import QtQuick.Controls
import Qt.labs.qmlmodels

Item {
    anchors.fill: parent

    VerticalHeaderView {
        id: verticalHeaderView
        anchors.left: parent.left
        width: parent.width / 3; height: parent.height
        syncView: tableView
        clip: true
        interactive: false
        movableRows: true

        delegate: VerticalHeaderViewDelegate {
            id: verticalDelegate
            required property int index

            background: Rectangle {
                color: "transparent"
                border.width: 0
            }

            contentItem: Text {
                anchors.fill: parent
                clip: true
                font.family: "Segoe UI"
                font.pointSize: 10
                horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                text: verticalHeader[verticalDelegate.index]
            }
        }
    }

    Rectangle {
        id: separator
        x: verticalHeaderView.width
        width: 2; height: parent.height
        color : separatorHoverHandler.hovered ? "#0f6cbd" : "#e0e0e0"

        HoverHandler {
            id: separatorHoverHandler
            cursorShape: Qt.SplitHCursor
        }

        DragHandler {
            id: separatorDrag
            target: null

            property real startWidth: verticalHeaderView.width

            onActiveChanged: {
                if (active) {
                    startWidth = verticalHeaderView.width
                }
            }

            onTranslationChanged: {
                if (active) {
                    verticalHeaderView.width = startWidth + translation.x
                }
            }
        }
    }

    TableView {
        id: tableView
        anchors.left: separator.right; anchors.right: parent.right
        height: parent.height
        alternatingRows: false
        clip: true
        editTriggers: TableView.NoEditTriggers
        model: databaseModel
        contentWidth: width
        property int selectedRow: 0

        delegate: Rectangle {
            id: textCell
            required property int column
            required property int row

            implicitWidth: parent.width
            implicitHeight: 24

            Text {
                anchors.fill: parent
                z: 2
                font.family: "Segoe UI"
                font.pointSize: 10
                horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                text: model.display
                elide: Text.ElideRight
            }

            Rectangle {
                id: highlightRect
                anchors.fill: parent
                z: 1
                radius: 2
                color: "#f5f5f5"
                opacity: hoverHandler.hovered ? 1 : 0
                Behavior on opacity {
                    NumberAnimation {
                        duration: 150
                    }
                }
            }

            HoverHandler {
                id: hoverHandler
            }
        }
    }
}