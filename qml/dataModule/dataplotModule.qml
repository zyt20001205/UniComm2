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
                    categories: ["test"]
                }

                axisY: ValueAxis {
                    min: 0
                    max: 100
                }

                BarSeries {
                    id: barSeries
                    property var barSetMap
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
                    delegate: CheckDelegate {
                        implicitWidth: databaseTableView.width
                        text: model.display
                        background: Rectangle {
                            color: "white"
                        }

                        onClicked: {
                            const label = model.display
                            let bar;
                            if (checked) {
                                bar = barComponent.createObject(null, {
                                    label: label,
                                    values: [50]
                                });
                                barSeries.append(bar)
                                barSeries.barSetMap[label] = bar
                            } else {
                                bar = barSeries.barSetMap[label]
                                barSeries.remove(bar)
                                delete barSeries.barSetMap[label]
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

    Component.onCompleted: {
        barSeries.barSetMap = {}
    }
}