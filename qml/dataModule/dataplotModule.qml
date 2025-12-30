import QtGraphs
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent

    BarSeries {
        anchors.fill: parent

        axisX: CategoryAxis {
            categories: ["test"]
        }

        axisY: ValueAxis {
            min: 0
            max: 100
        }

        BarSet {
            label: "test barset"
            values: [50]
            color: "blue"
        }
    }
}