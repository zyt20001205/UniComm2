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
        focus: true

        Keys.onPressed: (event) => {
            event.accepted = terminalPage.terminalInput(event.key, event.modifiers, event.text)
        }
    }

    Component.onCompleted: {
        const objects = {
            "terminalItem": terminalItem
        };
        terminalPage.propertyGet(objects)
    }
}
