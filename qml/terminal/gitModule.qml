import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent
    property bool modelVisible: branchModel.rowCount() > 0

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

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal
        visible: global.git

        Item {
            id: branchItem
            implicitWidth: 400

            RowLayout {
                anchors.centerIn: parent
                visible: !modelVisible

                Button {
                    flat: true
                    text: qsTr("No commits yet.")
                    font.pixelSize: 16
                    Layout.alignment: Qt.AlignVCenter

                    onClicked: gitModule.gitCommit()
                }

                IconImage {
                    source: "qrc:/icon/gitBranch.svg"
                    color: global.fore
                    Layout.alignment: Qt.AlignVCenter
                }
            }

            TreeView {
                id: treeView
                anchors.fill: parent
                clip: true
                model: branchModel
                visible: modelVisible
                property int selectedRow: -1

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
                            Layout.preferredHeight: 24
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        Label {
                            horizontalAlignment: Text.AlignRight; verticalAlignment: Text.AlignVCenter
                            text: model.commit || ""
                            elide: Text.ElideLeft
                            Layout.preferredHeight: 24
                            Layout.rightMargin: 10
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
                            mainToolTip.text = model.hash || ""
                        }
                    }

                    TapHandler {
                        acceptedButtons: Qt.LeftButton
                        gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                        onTapped: {
                            treeView.selectedRow = row
                            if (isTreeNode && hasChildren) {
                                treeView.toggleExpanded(row)
                            } else {
                                gitModule.branchSet(model.display)
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
                    acceptedButtons: Qt.MiddleButton

                    onTapped: gitModule.gitBranch()
                }
            }
        }

        TableView {
            id: tableView
            implicitWidth: 800
            alternatingRows: false
            clip: true
            editTriggers: TableView.NoEditTriggers
            model: logModel
            visible: modelVisible
            contentWidth: width

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                palette {
                    mid: global.stroke
                    dark: global.strokePressed
                }
            }

            Canvas {
                id: canvas
                parent: tableView.contentItem

                x: tableView.width - width
                y: 0
                width: 24 * laneCount
                height: tableView.contentHeight
                property int laneCount: 0

                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()

                    const unit = 24
                    const colors = ["#264653", "#2a9d8f", "#e9c46a", "#f4a261", "#e76f51"]

                    for (let i = 0; i < logModel.rowCount(); ++i) {
                        const index = logModel.index(i, 3);
                        const pos = logModel.data(index, Qt.UserRole + 1)
                        const parents = logModel.data(index, Qt.UserRole + 2) || []
                        const _x = pos.x * unit + unit / 2
                        const _y = pos.y * unit + unit / 2

                        for (let j = 0; j < parents.length; ++j) {
                            const parentPos = parents[j]
                            const px = parentPos.x * unit + unit / 2
                            const py = parentPos.y * unit + unit / 2
                            const dy = py - _y
                            ctx.lineWidth = 2
                            ctx.strokeStyle = colors[Math.max(pos.x, parentPos.x) % colors.length]
                            ctx.beginPath()
                            ctx.moveTo(_x, _y)

                            const dx = px - _x
                            const diagonalHeight = Math.abs(dx)
                            if (dx === 0) {
                                ctx.lineTo(px, py)
                            } else if (diagonalHeight <= dy) {
                                if (dx > 0) {
                                    ctx.lineTo(px, _y + diagonalHeight)
                                    ctx.lineTo(px, py)
                                } else {
                                    ctx.lineTo(_x, py - diagonalHeight)
                                    ctx.lineTo(px, py)
                                }
                            } else {
                                ctx.lineTo(px, py)
                            }
                            ctx.stroke()
                        }
                    }

                    for (let i = 0; i < logModel.rowCount(); ++i) {
                        const index = logModel.index(i, 3)
                        const pos = logModel.data(index, Qt.UserRole + 1)
                        const _x = pos.x * unit + unit / 2
                        const _y = pos.y * unit + unit / 2
                        ctx.fillStyle = colors[pos.x % colors.length]
                        ctx.beginPath()
                        ctx.arc(_x, _y, 4, 0, Math.PI * 2)
                        ctx.fill()
                    }
                }
            }

            delegate: Item {
                visible: column !== tableView.columns - 1
                implicitWidth: {
                    if (column === tableView.columns - 1) {
                        let usedWidth = 0
                        for (let i = 0; i < tableView.columns - 1; i++) {
                            usedWidth += tableView.columnWidth(i)
                        }
                        return tableView.width - usedWidth
                    }
                    return Math.max(textMetrics.width + 16, 60)
                }
                implicitHeight: 24
                required property int column

                Rectangle {
                    anchors.fill: parent
                    color: global.back
                }

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

                TextMetrics {
                    id: textMetrics
                    font: label.font
                    text: model.display || ""
                }

                Label {
                    id: label
                    anchors.fill: parent
                    leftPadding: 6
                    horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                    text: model.display || ""
                    elide: Text.ElideRight
                }

                HoverHandler {
                    id: hoverHandler
                }
            }
        }

        ScrollView {
            id: terminalView

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

    Connections {
        target: branchModel

        function onRowsInserted() {
            modelVisible = true
        }

        function onRowsRemoved() {
            modelVisible = branchModel.rowCount() > 0
        }

        function onModelReset() {
            modelVisible = false
        }
    }

    Connections {
        target: logModel

        function onRowsInserted() {
            canvas.requestPaint()
        }

        function onRowsRemoved() {
            canvas.requestPaint()
        }

        function onDataChanged() {
            canvas.requestPaint()
        }

        function onModelReset() {
            canvas.requestPaint()
        }
    }

    Component.onCompleted: {
        const objects = {
            "canvas": canvas,
            "textArea": textArea
        };
        gitModule.propertyGet(objects)
    }
}
