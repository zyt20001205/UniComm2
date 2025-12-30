import QtGraphs
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent

    RowLayout {
        anchors.fill: parent

        StackLayout {
            currentIndex: dataSourceComboBox.currentIndex
            Layout.fillWidth: true; Layout.fillHeight: true
            Layout.preferredWidth: 3
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
                    BarSet {
                        label: "test barset"
                        values: [50]
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
        }
    }
}