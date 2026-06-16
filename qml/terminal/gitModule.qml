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
            SplitView.preferredWidth: 400

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
                id: branchTreeView
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
                    implicitWidth: branchTreeView.width; implicitHeight: 24
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
                        color: branchTreeView.selectedRow === row ? global.backSelected : "transparent"
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
                            branchTreeView.selectedRow = row
                            if (isTreeNode && hasChildren) {
                                branchTreeView.toggleExpanded(row)
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

                    onTapped: branchTreeView.selectedRow = -1
                }
            }
        }

        TableView {
            id: tableView
            alternatingRows: false
            clip: true
            editTriggers: TableView.NoEditTriggers
            model: logModel
            visible: modelVisible
            contentWidth: width
            SplitView.fillWidth: true
            property int hoveredRow: -1
            property int selectedRow: -1

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                palette {
                    mid: global.stroke
                    dark: global.strokePressed
                }
            }

            columnWidthProvider: function (column) {
                if (column !== columns - 1) return implicitColumnWidth(column)
                let usedWidth = 0
                for (let i = 0; i < columns - 1; ++i) usedWidth += implicitColumnWidth(i)
                return width - usedWidth
            }

            delegate: Item {
                implicitWidth: Math.max(textMetrics.width + 16, 60)
                implicitHeight: 24
                required property int column
                required property int row

                Rectangle {
                    anchors.fill: parent
                    color: global.back
                }

                Rectangle {
                    anchors.fill: parent
                    radius: 6
                    topLeftRadius: column === 0 ? radius : 0
                    bottomLeftRadius: column === 0 ? radius : 0
                    topRightRadius: column === tableView.columns - 1 ? radius : 0
                    bottomRightRadius: column === tableView.columns - 1 ? radius : 0
                    color: global.backHover
                    opacity: tableView.hoveredRow === row ? 1 : 0
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 150
                        }
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    topLeftRadius: column === 0 ? 6 : 0
                    bottomLeftRadius: column === 0 ? 6 : 0
                    topRightRadius: column === tableView.columns - 1 ? 6 : 0
                    bottomRightRadius: column === tableView.columns - 1 ? 6 : 0
                    color: tableView.selectedRow === row ? global.backSelected : "transparent"
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
                    visible: column !== tableView.columns - 1
                }

                HoverHandler {
                    onHoveredChanged: {
                        if (hovered) {
                            tableView.hoveredRow = row
                        } else if (tableView.hoveredRow === row) {
                            tableView.hoveredRow = -1
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
                        tableView.selectedRow = row
                        gitModule.gitShow(model.hash)
                    }
                }

                TapHandler {
                    acceptedButtons: Qt.RightButton
                    gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                    onTapped: {
                        logMenu.hash = model.hash
                        logMenu.popup()
                    }
                }
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton

                onTapped: tableView.selectedRow = -1
            }

            Canvas {
                id: canvas
                parent: tableView.contentItem

                x: tableView.width - width
                y: 0
                width: 24 * laneCount
                height: tableView.contentHeight
                z: 100
                property int laneCount: 0

                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()

                    const unit = 24
                    const colors = ["#264653", "#2a9d8f", "#e9c46a", "#f4a261", "#e76f51"]

                    for (let i = 0; i < logModel.rowCount(); ++i) {
                        const index = logModel.index(i, 3);
                        const pos = logModel.data(index, Qt.UserRole + 2)
                        const parents = logModel.data(index, Qt.UserRole + 3) || []
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
                        const pos = logModel.data(index, Qt.UserRole + 2)
                        const _x = pos.x * unit + unit / 2
                        const _y = pos.y * unit + unit / 2
                        ctx.fillStyle = colors[pos.x % colors.length]
                        ctx.beginPath()
                        ctx.arc(_x, _y, 4, 0, Math.PI * 2)
                        ctx.fill()
                    }
                }
            }
        }

        ColumnLayout {
            SplitView.preferredWidth: 400

            TreeView {
                id: showTreeView
                width: parent.width
                clip: true
                model: showModel
                visible: modelVisible
                Layout.fillHeight: true
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

        function onModelReset() {
            canvas.requestPaint()
            tableView.selectedRow = 0
        }
    }

    function branchExpand() {
        for (let i = 0; i < branchTreeView.rows; ++i) {
            branchTreeView.expandRecursively(i)
        }
    }

    Component.onCompleted: {
        const objects = {
            "canvas": canvas,
            "subjectLabel": subjectLabel,
            "dateLabel": dateLabel,
            "authorLabel": authorLabel
        };
        gitModule.propertyGet(objects)
    }
}
