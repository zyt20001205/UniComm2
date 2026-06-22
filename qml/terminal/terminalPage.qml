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
    }

    Component.onCompleted: {
        const objects = {
            "terminalItem": terminalItem
        };
        terminalPage.propertyGet(objects)
    }
}
