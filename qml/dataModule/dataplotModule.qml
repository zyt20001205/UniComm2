import QtGraphs
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent
    property bool resize: true

    StackLayout {
        currentIndex: dataSourceComboBox.currentIndex
        anchors.fill: parent
        clip: true

        GraphsView {
            axisX: BarCategoryAxis {
                categories: ["database"]
            }
            axisY: ValueAxis {
                id: barAxisY
                property real minHint: 0
                property real maxHint: 0
            }
            marginLeft: 10; marginTop: 10; marginRight: 10; marginBottom: 10
            theme: GraphsTheme {
                theme: GraphsTheme.Theme.QtGreen
                backgroundVisible: false
                plotAreaBackgroundVisible: false
                gridVisible: false
            }
            Layout.fillWidth: true; Layout.fillHeight: true

            BarSeries {
                id: barSeries
                selectable: true
                property var barSetMap

                onClicked: (index, barset) => {
                    hintPopup.barset = barset
                    hintPopup.text = barset.label + " " + barset.values[0]
                    hintPopup.open()
                }

                Component.onCompleted: {
                    barSetMap = {}
                }

                Component {
                    id: barComponent

                    BarSet {
                    }
                }

                function barInsert(key, value) {
                    const bar = barComponent.createObject(barSeries, {
                        label: key,
                        values: [value]
                    });
                    append(bar)
                    barSetMap[key] = bar
                }

                function barRemove(key) {
                    const bar = barSetMap[key]
                    remove(bar)
                    delete barSetMap[key]
                }
            }

            Popup {
                id: hintPopup
                padding: 0
                property var barset
                property string text

                Label {
                    text: hintPopup.text
                }

                onAboutToHide: {
                    barset.deselectBar(0)
                }
            }
        }

        GraphsView {
            id: lineGraph
            axisX: ValueAxis {
                id: lineAxisX
                property real minHint: 0
                property real maxHint: 0
            }
            axisY: ValueAxis {
                id: lineAxisY
                property real minHint: 0
                property real maxHint: 0
            }
            focus: true
            marginLeft: 10; marginTop: 10; marginRight: 10; marginBottom: 10
            theme: GraphsTheme {
                theme: GraphsTheme.Theme.QtGreen
                backgroundVisible: false
                plotAreaBackgroundVisible: false
                gridVisible: false
            }
            panStyle: GraphsView.PanStyle.None
            zoomAreaEnabled: true
            zoomStyle: GraphsView.ZoomStyle.Center
            Layout.fillWidth: true; Layout.fillHeight: true
            property var lineSeriesMap

            onVisibleChanged: {
                if (visible) forceActiveFocus()
            }

            HoverHandler {
                id: hoverHandler
                cursorShape: Qt.ArrowCursor
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                gesturePolicy: TapHandler.ReleaseWithinBounds
                onPressedChanged: parent.forceActiveFocus()
            }

            Keys.onPressed: (event) => {
                if (event.key === Qt.Key_Alt) {
                    hoverHandler.cursorShape = Qt.OpenHandCursor
                    panStyle = GraphsView.PanStyle.Drag
                    zoomAreaEnabled = false
                }
            }

            Keys.onReleased: (event) => {
                if (event.key === Qt.Key_Alt) {
                    hoverHandler.cursorShape = Qt.ArrowCursor
                    panStyle = GraphsView.PanStyle.None
                    zoomAreaEnabled = true
                }
            }

            Component.onCompleted: {
                lineSeriesMap = {}
            }

            Component {
                id: lineComponent

                LineSeries {
                    id: lineSeries
                    width: 3
                }
            }

            function lineInsert(key) {
                const line = lineComponent.createObject(lineGraph, {});
                addSeries(line)
                lineSeriesMap[key] = line
            }

            function lineRemove(key) {
                const line = lineSeriesMap[key]
                removeSeries(line)
                delete lineSeriesMap[key]
            }
        }
    }

    Drawer {
        id: drawer
        width: 200; height: rootItem.height
        edge: Qt.RightEdge
        background: Rectangle {
            color: "white"
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 4
            Layout.alignment: Qt.AlignTop

            Label {
                text: qsTr("Data Source")
            }

            ComboBox {
                id: dataSourceComboBox
                model: ListModel {
                    ListElement {
                        text: qsTr("Database"); value: "database"
                    }
                    ListElement {
                        text: qsTr("Datatable"); value: "datatable"
                    }
                }
                textRole: "text"
                valueRole: "value"
                Layout.fillWidth: true
            }

            StackLayout {
                currentIndex: dataSourceComboBox.currentIndex

                TableView {
                    id: databaseTableView
                    Layout.fillWidth: true; Layout.fillHeight: true
                    alternatingRows: false
                    clip: true
                    editTriggers: TableView.NoEditTriggers
                    rowSpacing: 1
                    model: databaseStandardItemModel
                    contentWidth: width
                    delegate: DelegateChooser {
                        DelegateChoice {
                            column: 0
                            delegate: CheckDelegate {
                                implicitWidth: databaseTableView.width
                                checked: model.whatsThis
                                text: model.display
                                background: Rectangle {
                                    color: "white"
                                }

                                onClicked: {
                                    model.whatsThis = checked
                                    const key = model.display
                                    if (checked) {
                                        const index = databaseStandardItemModel.index(row, 1);
                                        const value = databaseStandardItemModel.data(index, Qt.DisplayRole)
                                        barSeries.barInsert(key, value)
                                    } else {
                                        barSeries.barRemove(key)
                                    }
                                }
                            }
                        }
                        DelegateChoice {
                            delegate: Item {
                                implicitWidth: 1
                            }
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: "#e0e0e0"
                        z: -1
                    }
                }

                TableView {
                    id: datatableTableView
                    Layout.fillWidth: true; Layout.fillHeight: true
                    alternatingRows: false
                    clip: true
                    editTriggers: TableView.NoEditTriggers
                    rowSpacing: 1
                    model: datatableHeaderItemModel
                    contentWidth: width
                    delegate: CheckDelegate {
                        implicitWidth: datatableTableView.width
                        checked: model.whatsThis
                        text: model.display
                        background: Rectangle {
                            color: "white"
                        }

                        onClicked: {
                            model.whatsThis = checked
                            const key = model.display
                            if (checked) {
                                lineGraph.lineInsert(key)
                            } else {
                                lineGraph.lineRemove(key)
                            }
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: "#e0e0e0"
                        z: -1
                    }
                }
            }
        }
    }

    TapHandler {
        acceptedButtons: Qt.RightButton

        onSingleTapped: {
            rootMenu.drawer = drawer
            rootMenu.rootItem = rootItem
            rootMenu.popup()
        }
    }

    TapHandler {
        acceptedButtons: Qt.MiddleButton

        onSingleTapped: graphResize()
    }

    function graphResize(index = -1) {
        if (index === -1) index = dataSourceComboBox.currentIndex
        switch (index) {
            case 0: {
                if (barSeries.count === 0) {
                    barAxisY.min = 0
                    barAxisY.max = 10
                    barAxisY.minHint = 0
                    barAxisY.maxHint = 0
                } else {
                    barAxisY.min = barAxisY.minHint
                    barAxisY.max = barAxisY.maxHint
                }
                barAxisY.zoom = 1
                barAxisY.pan = 0
            }
                break
            case 1: {
                if (lineGraph.seriesList.length === 0) {
                    lineAxisX.min = 0
                    lineAxisX.max = 10
                    lineAxisX.minHint = 0
                    lineAxisX.maxHint = 0
                    lineAxisY.min = 0
                    lineAxisY.max = 10
                    lineAxisY.minHint = 0
                    lineAxisY.maxHint = 0
                } else {
                    lineAxisX.min = lineAxisX.minHint
                    lineAxisX.max = lineAxisX.maxHint
                    lineAxisY.min = lineAxisY.minHint
                    lineAxisY.max = lineAxisY.maxHint
                }
                lineAxisX.zoom = 1
                lineAxisX.pan = 0
                lineAxisY.zoom = 1
                lineAxisY.pan = 0
            }
                break
        }
    }

    function graphClear() {
        if (index === -1) index = dataSourceComboBox.currentIndex
        switch (index) {
            case 0: {
            }
                break
            case 1: {
            }
                break
        }
    }

    Connections {
        target: databaseStandardItemModel

        function onDataChanged(topLeft, bottomRight, roles) {
            const col = topLeft.column
            if (col !== 1) return
            const row = topLeft.row
            const keyIndex = databaseStandardItemModel.index(row, 0);
            const watched = databaseStandardItemModel.data(keyIndex, Qt.WhatsThisRole)
            if (watched) {
                const key = databaseStandardItemModel.data(keyIndex, Qt.Display)
                const valueIndex = databaseStandardItemModel.index(row, 1);
                const value = databaseStandardItemModel.data(valueIndex, Qt.Display)
                barSeries.barSetMap[key].values = [value]
                // calc range
                if (value < barAxisY.minHint * 0.8) {
                    barAxisY.minHint = value / 0.8
                } else if (value > barAxisY.maxHint * 0.8) {
                    barAxisY.maxHint = value / 0.8
                }
                if (rootItem.resize) {
                    graphResize(0)
                }
            }
        }
    }

    Connections {
        target: datatableStandardItemModel

        function onDataChanged(topLeft, bottomRight, roles) {
            const col = topLeft.column
            const row = topLeft.row
            const keyIndex = datatableHeaderItemModel.index(col, 0);
            const watched = datatableHeaderItemModel.data(keyIndex, Qt.WhatsThisRole)
            if (watched) {
                const key = datatableHeaderItemModel.data(keyIndex, Qt.Display)
                const valueIndex = datatableStandardItemModel.index(row, col)
                const value = datatableStandardItemModel.data(valueIndex, Qt.Display)
                lineGraph.lineSeriesMap[key].append(row, value)
                // calc range
                if (row > lineAxisX.maxHint) {
                    lineAxisX.maxHint = row
                }
                if (value < lineAxisY.minHint * 0.8) {
                    lineAxisY.minHint = value / 0.8
                } else if (value > lineAxisY.maxHint * 0.8) {
                    lineAxisY.maxHint = value / 0.8
                }
                if (rootItem.resize) {
                    graphResize(1)
                }
            }
        }
    }
}