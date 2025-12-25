import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    anchors.fill: parent

    RowLayout {
        anchors.fill: parent
        anchors.margins: 4

        ColumnLayout {
            Layout.alignment: Qt.AlignTop

            RowLayout {

                ComboBox {
                    id: nameComboBox
                    model: standardItemModel
                    textRole: "display"
                    valueRole: "whatsThis"
                    Layout.fillWidth: true
                }

                Switch {
                    id: overrideSwitch
                    ToolTip.text: checked ? qsTr("Use Port Config") : qsTr("Manual Override")
                    ToolTip.visible: hovered

                    onClicked: {
                        if (checked) {
                            sendModule.configLoad()
                        }
                    }
                }
            }

            GridLayout {
                columns: 2
                visible: overrideSwitch.checked

                Label {
                    text: qsTr("Format")
                }

                ComboBox {
                    id: formatComboBox
                    model: ListModel {
                        ListElement {
                            text: qsTr("raw"); value: "raw"
                        }
                        ListElement {
                            text: qsTr("hex"); value: "hex"
                        }
                        ListElement {
                            text: qsTr("ascii"); value: "ascii"
                        }
                        ListElement {
                            text: qsTr("utf-8"); value: "utf-8"
                        }
                    }
                    textRole: "text"
                    valueRole: "value"
                    Layout.fillWidth: true
                }

                Label {
                    text: qsTr("Suffix")
                }

                ComboBox {
                    id: suffixComboBox
                    model: ListModel {
                        ListElement {
                            text: qsTr("null"); value: "null"
                        }
                        ListElement {
                            text: qsTr("crlf"); value: "crlf"
                        }
                        ListElement {
                            text: qsTr("crc16 modbus"); value: "crc16 modbus"
                        }
                    }
                    textRole: "text"
                    valueRole: "value"
                    Layout.fillWidth: true
                }
            }

            TextField {
                id: sendTextField
                Layout.fillWidth: true

                Keys.onReturnPressed: sendModule.commandSend()
                Keys.onEnterPressed: sendModule.commandSend()
            }
        }
    }

    Component.onCompleted: {
        const objects = {
            "nameComboBox": nameComboBox,
            "overrideSwitch": overrideSwitch,
            "formatComboBox": formatComboBox,
            "suffixComboBox": suffixComboBox,
            "sendTextField": sendTextField
        };
        sendModule.propertyGet(objects)
    }
}