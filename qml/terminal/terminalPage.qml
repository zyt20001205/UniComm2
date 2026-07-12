import QtQuick

Item {
    id: rootItem
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: global.back
    }

    Item {
        id: terminalItem
        anchors.fill: parent

        Rectangle {
            id: damageOverlay
            visible: false
            color: "transparent"
            border.color: "#ff00ff"
            border.width: 1
            z: 100
        }
    }

    Component.onCompleted: {
        const objects = {
            "terminalItem": terminalItem,
            "damageOverlay": damageOverlay
        };
        terminalPage.propertyGet(objects)
    }
}
