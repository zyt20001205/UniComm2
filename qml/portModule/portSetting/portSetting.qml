import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    property int portType

    Component {
        id: delegateComponent

        Label {
            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            opacity: 1.0 - Math.abs(Tumbler.displacement) / (Tumbler.tumbler.visibleItemCount / 2)
            text: modelData
            font.pointSize: 16
        }
    }

    ColumnLayout {
        anchors.fill: parent

        SwipeView {
            id: swipeView
            clip: true
            currentIndex: 0
            interactive: false
            Layout.fillWidth: true; Layout.fillHeight: true
            Layout.margins: 20

            ColumnLayout {
                Layout.fillWidth: true; Layout.fillHeight: true

                Tumbler {
                    id: tumbler
                    delegate: delegateComponent
                    model: [qsTr("Serial Port"), qsTr("Visa"), qsTr("Tcp Client"), qsTr("Tcp Server"), qsTr("Udp Socket"), qsTr("Screen"), qsTr("Camera")]
                    wrap: false
                    Layout.fillWidth: true; Layout.fillHeight: true

                    onMovingChanged: {
                        if (!moving) {
                            rootItem.portType = currentIndex
                        }
                    }
                }

                StackLayout {
                    currentIndex: rootItem.portType
                    Layout.fillWidth: true

                    ColumnLayout {
                        Layout.fillWidth: true

                        Item {
                            Layout.fillHeight: true
                        }

                        Image {
                            source: "qrc:/icon/serialPort.svg"
                            sourceSize: Qt.size(80, 80)
                            Layout.preferredWidth: 80; Layout.preferredHeight: 80
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Label {
                            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                            text: qsTr("A serial communication interface through which information transfers in or out sequentially one bit at a time.")
                            wrapMode: Text.WordWrap
                            font.pointSize: 12
                            Layout.alignment: Qt.AlignHCenter; Layout.fillWidth: true
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true

                        Item {
                            Layout.fillHeight: true
                        }

                        Image {
                            // source: "qrc:/icon/serialPort.svg"
                            sourceSize: Qt.size(80, 80)
                            Layout.preferredWidth: 80; Layout.preferredHeight: 80
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Label {
                            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                            text: qsTr("A widely used application programming interface (API) in the test and measurement (T&M) industry for communicating with instruments from a computer.")
                            wrapMode: Text.WordWrap
                            font.pointSize: 12
                            Layout.alignment: Qt.AlignHCenter; Layout.fillWidth: true
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true

                        Item {
                            Layout.fillHeight: true
                        }

                        Image {
                            source: "qrc:/icon/tcpClient.svg"
                            sourceSize: Qt.size(80, 80)
                            Layout.preferredWidth: 80; Layout.preferredHeight: 80
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Label {
                            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                            text: qsTr("A device that initiates a connection with a TCP server to send and receive reliable, ordered data over a network.")
                            wrapMode: Text.WordWrap
                            font.pointSize: 12
                            Layout.alignment: Qt.AlignHCenter; Layout.fillWidth: true
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true

                        Item {
                            Layout.fillHeight: true
                        }

                        Image {
                            source: "qrc:/icon/tcpServer.svg"
                            sourceSize: Qt.size(80, 80)
                            Layout.preferredWidth: 80; Layout.preferredHeight: 80
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Label {
                            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                            text: qsTr("A device that listens on a network port, accepts incoming connections from TCP clients, and manages reliable, ordered data exchange.")
                            wrapMode: Text.WordWrap
                            font.pointSize: 12
                            Layout.alignment: Qt.AlignHCenter; Layout.fillWidth: true
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true

                        Item {
                            Layout.fillHeight: true
                        }

                        Image {
                            source: "qrc:/icon/udpSocket.svg"
                            sourceSize: Qt.size(80, 80)
                            Layout.preferredWidth: 80; Layout.preferredHeight: 80
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Label {
                            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                            text: qsTr("A device that uses the User Datagram Protocol to send independent, connectionless messages (datagrams) over an IP network.")
                            wrapMode: Text.WordWrap
                            font.pointSize: 12
                            Layout.alignment: Qt.AlignHCenter; Layout.fillWidth: true
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true

                        Item {
                            Layout.fillHeight: true
                        }

                        Image {
                            source: "qrc:/icon/screen.svg"
                            sourceSize: Qt.size(80, 80)
                            Layout.preferredWidth: 80; Layout.preferredHeight: 80
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Label {
                            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                            text: qsTr("Capture screenshots for image processing and OCR text recognition.")
                            wrapMode: Text.WordWrap
                            font.pointSize: 12
                            Layout.alignment: Qt.AlignHCenter; Layout.fillWidth: true
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true

                        Item {
                            Layout.fillHeight: true
                        }

                        Image {
                            source: "qrc:/icon/camera.svg"
                            sourceSize: Qt.size(80, 80)
                            Layout.preferredWidth: 80; Layout.preferredHeight: 80
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Label {
                            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                            text: qsTr("Take pictures for image processing and OCR text recognition.")
                            wrapMode: Text.WordWrap
                            font.pointSize: 12
                            Layout.alignment: Qt.AlignHCenter; Layout.fillWidth: true
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true; Layout.fillHeight: true

                StackLayout {
                    currentIndex: rootItem.portType
                    Layout.fillWidth: true; Layout.fillHeight: true

                    GridLayout {
                        columns: 2
                        columnSpacing: 20; rowSpacing: 20

                        Label {
                            text: qsTr("Port Name")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        ComboBox {
                            id: serialPortNameComboBox
                            model: serialPortStandardItemModel
                            textRole: "display"
                            valueRole: "whatsThis"
                            Layout.fillWidth: true
                        }

                        Label {
                            text: qsTr("Baud Rate")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        SpinBox {
                            id: serialPortBaudRateSpinBox
                            font.pointSize: 12
                            editable: true
                            from: 1
                            to: 5000000
                            value: 115200
                            Layout.fillWidth: true
                        }

                        Label {
                            text: qsTr("Databits")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        ComboBox {
                            id: serialPortDataBitsComboBox
                            model: ListModel {
                                ListElement {
                                    text: "5"; value: 5
                                }
                                ListElement {
                                    text: "6"; value: 6
                                }
                                ListElement {
                                    text: "7"; value: 7
                                }
                                ListElement {
                                    text: "8"; value: 8
                                }
                            }
                            textRole: "text"
                            valueRole: "value"
                            currentValue: 8
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        Label {
                            text: qsTr("Parity")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        ComboBox {
                            id: serialPortParityComboBox
                            model: ListModel {
                                ListElement {
                                    text: qsTr("No"); value: 0
                                }
                                ListElement {
                                    text: qsTr("Even"); value: 2
                                }
                                ListElement {
                                    text: qsTr("Odd"); value: 3
                                }
                                ListElement {
                                    text: qsTr("Space"); value: 4
                                }
                                ListElement {
                                    text: qsTr("Mark"); value: 5
                                }
                            }
                            textRole: "text"
                            valueRole: "value"
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        Label {
                            text: qsTr("Stop Bits")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        ComboBox {
                            id: serialPortStopBitsComboBox
                            model: ListModel {
                                ListElement {
                                    text: "1"; value: 1
                                }
                                ListElement {
                                    text: "1.5"; value: 3
                                }
                                ListElement {
                                    text: "2"; value: 2
                                }
                            }
                            textRole: "text"
                            valueRole: "value"
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }
                    }
                }

                Label {
                    id: portNameValidator
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    color: "#c50f1f"
                    text: ""
                    font.pointSize: 16
                }

                Timer {
                    id: portNameValidatorTimer
                    interval: 3000
                    onTriggered: {portNameValidator.text = ""}
                }
            }

            StackLayout {
                currentIndex: {
                    if (rootItem.portType in [0, 1, 2, 3, 4]) {
                        return 0
                    } else {
                        return 1
                    }
                }
                Layout.fillWidth: true; Layout.fillHeight: true

                GridLayout {
                    columns: 2
                    columnSpacing: 20; rowSpacing: 20

                    Label {
                        text: qsTr("Tx Format")
                        font.pointSize: 12
                        Layout.fillWidth: true
                    }

                    ComboBox {
                        id: txFormatComboBox
                        model: ListModel {
                            ListElement {
                                text: qsTr("raw")
                            }
                            ListElement {
                                text: qsTr("hex")
                            }
                            ListElement {
                                text: qsTr("ascii")
                            }
                            ListElement {
                                text: qsTr("utf-8")
                            }
                        }
                        textRole: "text"
                        Layout.fillWidth: true
                    }

                    Label {
                        text: qsTr("Tx Suffix")
                        font.pointSize: 12
                        Layout.fillWidth: true
                    }

                    ComboBox {
                        id: txSuffixComboBox
                        model: ListModel {
                            ListElement {
                                text: qsTr("null")
                            }
                            ListElement {
                                text: qsTr("crlf")
                            }
                            ListElement {
                                text: qsTr("crc16 modbus")
                            }
                        }
                        textRole: "text"
                        Layout.fillWidth: true
                    }

                    Label {
                        text: qsTr("Rx Format")
                        font.pointSize: 12
                        Layout.fillWidth: true
                    }

                    ComboBox {
                        id: rxFormatComboBox
                        model: ListModel {
                            ListElement {
                                text: qsTr("raw")
                            }
                            ListElement {
                                text: qsTr("hex")
                            }
                            ListElement {
                                text: qsTr("ascii")
                            }
                            ListElement {
                                text: qsTr("utf-8")
                            }
                        }
                        textRole: "text"
                        Layout.fillWidth: true
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            Layout.margins: 20

            Button {
                text: qsTr("Prev")
                enabled: swipeView.currentIndex !== 0

                onClicked: swipeView.currentIndex = swipeView.currentIndex - 1
            }

            Item {
                Layout.fillWidth: true
            }

            PageIndicator {
                id: pageIndicator
                count: swipeView.count
                currentIndex: swipeView.currentIndex
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: swipeView.currentIndex !== 2 ? qsTr("Next") : qsTr("Confirm")
                highlighted: swipeView.currentIndex === 2

                onClicked: {
                    if (swipeView.currentIndex === 0) {
                        swipeView.currentIndex = swipeView.currentIndex + 1
                    } else if (swipeView.currentIndex === 1) {
                        switch (tumbler.currentIndex) {
                            case 0: {
                                if (!serialPortNameComboBox.currentText) {
                                    portNameValidator.text = qsTr("Invalid Port Name")
                                    portNameValidatorTimer.start()
                                    return
                                }
                            }
                        }
                        swipeView.currentIndex = swipeView.currentIndex + 1
                    } else {
                        portSetting.portSettingExport()
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        const objects = {
            "tumbler": tumbler,

            "serialPortNameComboBox": serialPortNameComboBox,
            "serialPortBaudRateSpinBox": serialPortBaudRateSpinBox,
            "serialPortDataBitsComboBox": serialPortDataBitsComboBox,
            "serialPortParityComboBox": serialPortParityComboBox,
            "serialPortStopBitsComboBox": serialPortStopBitsComboBox,

            "txFormatComboBox": txFormatComboBox,
            "txSuffixComboBox": txSuffixComboBox,
            "rxFormatComboBox": rxFormatComboBox
        };
        portSetting.propertyGet(objects)
    }
}
