import QtGraphs
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent

    StackLayout {
        currentIndex: dataSourceComboBox.currentIndex
        anchors.fill: parent
        clip: true

        GraphsView {
            axisX: BarCategoryAxis {
                categories: ["database"]
            }
            axisY: ValueAxis {
                id: valueAxis
            }
            marginLeft: 10; marginTop: 10; marginRight: 10; marginBottom: 10
            theme: GraphsTheme {
                theme: GraphsTheme.Theme.QtGreen
                backgroundVisible: false
                plotAreaBackgroundVisible: false
                gridVisible: false
            }
            Layout.fillWidth: true; Layout.fillHeight: true

            Component {
                id: barComponent

                BarSet {}
            }

            BarSeries {
                id: barSeries
                selectable: true
                property var barSetMap

                onClicked: (index, barset) => {
                    console.log(index)
                }

                Component.onCompleted: {
                    barSetMap = {}
                }
            }
        }
    }

    Drawer {
        id: drawer
        width: 0.33 * rootItem.width; height: rootItem.height
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
                                        const bar = barComponent.createObject(null, {
                                            label: key,
                                            values: [value]
                                        });
                                        barSeries.append(bar)
                                        barSeries.barSetMap[key] = bar
                                    } else {
                                        const bar = barSeries.barSetMap[key]
                                        barSeries.remove(bar)
                                        delete barSeries.barSetMap[key]
                                        if (barSeries.count === 0) {
                                            valueAxis.max = 10
                                            valueAxis.min = 0
                                        }
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
                    model: datatableStringListModel
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
                            } else {
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
            rootMenu.popup()
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
                if (value < valueAxis.min) {
                    valueAxis.min = value
                } else if (value > valueAxis.max * 0.8) {
                    valueAxis.max = value / 0.8
                }
            }
        }
    }
}