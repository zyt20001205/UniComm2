import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent
    property bool pushable: commitModel.rowCount() > 0

    Rectangle {
        anchors.fill: parent
        color: global.back
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true

            TreeView {
                id: commitTreeView
                clip: true
                model: commitModel
                Layout.fillWidth: true; Layout.fillHeight: true
                Layout.preferredWidth: 1
                property int selectedRow: -1

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                    palette {
                        mid: global.stroke
                        dark: global.strokePressed
                    }
                }

                delegate: Item {
                    implicitWidth: commitTreeView.width; implicitHeight: 24
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
                        color: commitTreeView.selectedRow === row ? global.backSelected : "transparent"
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

                        Label {
                            horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
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
                            if (!(isTreeNode && hasChildren)) {
                                mainToolTip.position = parent.mapToGlobal(point.position)
                                mainToolTip.text = model.hash
                            }
                        }
                    }

                    TapHandler {
                        acceptedButtons: Qt.LeftButton
                        gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                        onTapped: {
                            commitTreeView.selectedRow = row
                            if (isTreeNode && hasChildren) {
                                commitTreeView.toggleExpanded(row)
                                gitModule.gitDiff_()
                            } else {
                                gitModule.gitShow_(model.hash)
                            }
                        }
                    }
                }

                TapHandler {
                    acceptedButtons: Qt.LeftButton

                    onTapped: {
                        commitTreeView.selectedRow = -1
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 1

                TreeView {
                    id: showTreeView
                    width: parent.width
                    clip: true
                    model: showModel
                    Layout.fillWidth: true; Layout.fillHeight: true
                    property int selectedRow: -1

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                        palette {
                            mid: global.stroke
                            dark: global.strokePressed
                        }
                    }

                    delegate: Item {
                        implicitWidth: showTreeView.width; implicitHeight: 24
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
                            color: showTreeView.selectedRow === row ? global.backSelected : "transparent"
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
                                    source: model.decoration
                                }
                            }

                            Label {
                                horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                                text: model.display || ""
                                elide: Text.ElideRight
                                color: {
                                    switch (model.status) {
                                        case 3:
                                            return global.brandBack
                                        case 5:
                                            return global.successFore3
                                        case 6:
                                            return global.stroke
                                        case 7:
                                            return global.warningFore3
                                        default:
                                            return global.fore
                                    }
                                }
                                Layout.preferredHeight: 24; Layout.fillWidth: true
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
                                if (!(isTreeNode && hasChildren)) {
                                    mainToolTip.position = parent.mapToGlobal(point.position)
                                    switch (model.status) {
                                        case 3:
                                            mainToolTip.text = "Modified"
                                            break
                                        case 5:
                                            mainToolTip.text = "Added"
                                            break
                                        case 6:
                                            mainToolTip.text = "Deleted"
                                            break
                                        case 7:
                                            mainToolTip.text = "Renamed"
                                            break
                                        default:
                                            mainToolTip.text = "contact author: unsupported status (" + model.status + ")"
                                    }
                                }
                            }
                        }

                        TapHandler {
                            acceptedButtons: Qt.LeftButton
                            gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                            onTapped: {
                                showTreeView.selectedRow = row
                                if (isTreeNode && hasChildren) {
                                    showTreeView.toggleExpanded(row)
                                }
                            }
                        }

                        TapHandler {
                            acceptedButtons: Qt.RightButton
                            gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                            onTapped: {
                                if (!(isTreeNode && hasChildren)) {
                                    console.log(model.status, model.documentUrl)
                                }
                            }
                        }
                    }

                    TapHandler {
                        acceptedButtons: Qt.LeftButton

                        onTapped: showTreeView.selectedRow = -1
                    }
                }

                ColumnLayout {
                    implicitHeight: subjectLabel.implicitHeight + dateLabel.implicitHeight + authorLabel.implicitHeight + spacing * 2

                    Label {
                        id: subjectLabel
                        horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        font.bold: true
                        Layout.fillWidth: true
                    }

                    Label {
                        id: dateLabel
                        horizontalAlignment: Text.AlignRight; verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    Label {
                        id: authorLabel
                        horizontalAlignment: Text.AlignRight; verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true; Layout.preferredHeight: 24
            Layout.alignment: Qt.AlignRight

            Button {
                enabled: pushable
                text: qsTr("Push")

                onClicked: gitModule.gitPush()
            }
        }
    }

    function reset() {
        commitTreeView.selectedRow = -1
    }

    Connections {
        target: commitModel

        function onRowsInserted() {
            pushable = true
        }

        function onRowsRemoved() {
            pushable = commitModel.rowCount() > 0
        }

        function onModelReset() {
            pushable = false
        }
    }

    Component.onCompleted: {
        const objects = {
            "subjectLabel": subjectLabel,
            "dateLabel": dateLabel,
            "authorLabel": authorLabel
        };
        gitModule.propertyGet_(objects)
    }
}
