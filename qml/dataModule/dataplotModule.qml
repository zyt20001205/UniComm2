import QtGraphs
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent

    RowLayout {
        anchors.fill: parent
        anchors.margins: 4

        StackLayout {
            currentIndex: dataSourceComboBox.currentIndex
            Layout.fillWidth: true; Layout.fillHeight: true
            Layout.preferredWidth: 2
            clip: true

            GraphsView {
                Layout.fillWidth: true; Layout.fillHeight: true
                theme: GraphsTheme {
                    theme: GraphsTheme.Theme.QtGreen
                    backgroundVisible: false
                    gridVisible: false
                }

                axisX: BarCategoryAxis {
                    categories: dataSourceComboBox.currentIndex === 0 ? ["database"] : ["datatable"]
                }

                axisY: ValueAxis {
                    id: valueAxis
                }

                BarSeries {
                    id: barSeries
                    property var barSetMap

                    Component.onCompleted: {
                        barSetMap = {}
                    }
                }

                Component {
                    id: barComponent

                    BarSet {
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true; Layout.fillHeight: true
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: 1

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
                    model: standardItemModel
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
                                    let bar;
                                    if (checked) {
                                        const index = standardItemModel.index(row, 1);
                                        const value = standardItemModel.data(index, Qt.DisplayRole)
                                        bar = barComponent.createObject(null, {
                                            label: key,
                                            values: [value]
                                        });
                                        barSeries.append(bar)
                                        barSeries.barSetMap[key] = bar
                                    } else {
                                        bar = barSeries.barSetMap[key]
                                        barSeries.remove(bar)
                                        delete barSeries.barSetMap[key]
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
            }
        }
    }

    Connections {
        target: standardItemModel

        function onDataChanged(topLeft, bottomRight, roles) {
            const row = topLeft.row
            const keyIndex = standardItemModel.index(row, 0);
            const watched = standardItemModel.data(keyIndex, Qt.WhatsThisRole)
            if (watched) {
                const key = standardItemModel.data(keyIndex, Qt.Display)
                const valueIndex = standardItemModel.index(row, 1);
                const value = standardItemModel.data(valueIndex, Qt.Display)
                barSeries.barSetMap[key].values = [value]
                if (value < valueAxis.min) {
                    valueAxis.min = value
                }
                else if (value > valueAxis.max * 0.8) {
                    valueAxis.max = value / 0.8
                }
            }
        }
    }
}