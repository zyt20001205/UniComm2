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
                    model: [qsTr("Serial Port"), qsTr("Visa(WIP)"), qsTr("Tcp Client"), qsTr("Tcp Server"), qsTr("Udp Socket"), qsTr("Screen"), qsTr("Camera")]
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

                    GridLayout {
                        columns: 2
                        columnSpacing: 20; rowSpacing: 20

                        Label {
                            text: qsTr("Port Name")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        ComboBox {
                            id: visaNameComboBox
                            model: visaStandardItemModel
                            textRole: "display"
                            valueRole: "whatsThis"
                            Layout.fillWidth: true
                        }
                    }

                    GridLayout {
                        columns: 2
                        columnSpacing: 20; rowSpacing: 20

                        Label {
                            text: qsTr("Port Name")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        TextField {
                            id: tcpClientNameTextField
                            placeholderText: qsTr("Tcp Client")
                            Layout.fillWidth: true
                        }

                        Label {
                            text: qsTr("Remote Host")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        TextField {
                            id: tcpClientRemoteHostTextField
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        Label {
                            text: qsTr("Remote Port")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        SpinBox {
                            id: tcpClientRemotePortSpinBox
                            font.pointSize: 12
                            editable: true
                            from: 0
                            to: 65535
                            Layout.fillWidth: true
                        }
                    }

                    GridLayout {
                        columns: 2
                        columnSpacing: 20; rowSpacing: 20

                        Label {
                            text: qsTr("Port Name")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        TextField {
                            id: tcpServerNameTextField
                            placeholderText: qsTr("Tcp Server")
                            Layout.fillWidth: true
                        }

                        Label {
                            text: qsTr("Local Host")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        TextField {
                            id: tcpServerLocalHostTextField
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        Label {
                            text: qsTr("Local Port")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        SpinBox {
                            id: tcpServerLocalPortSpinBox
                            font.pointSize: 12
                            editable: true
                            from: 0
                            to: 65535
                            Layout.fillWidth: true
                        }
                    }

                    GridLayout {
                        columns: 2
                        columnSpacing: 20; rowSpacing: 20

                        Label {
                            text: qsTr("Port Name")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        TextField {
                            id: udpSocketNameTextField
                            placeholderText: qsTr("Udp Socket")
                            Layout.fillWidth: true
                        }

                        Label {
                            text: qsTr("Local Host")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        TextField {
                            id: udpSocketLocalHostTextField
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        Label {
                            text: qsTr("Local Port")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        SpinBox {
                            id: udpSocketLocalPortSpinBox
                            font.pointSize: 12
                            editable: true
                            from: 0
                            to: 65535
                            Layout.fillWidth: true
                        }

                        Label {
                            text: qsTr("Remote Host")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        TextField {
                            id: udpSocketRemoteHostTextField
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        Label {
                            text: qsTr("Remote Port")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        SpinBox {
                            id: udpSocketRemotePortSpinBox
                            font.pointSize: 12
                            editable: true
                            from: 0
                            to: 65535
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
                        text: qsTr("Tx Suffix")
                        font.pointSize: 12
                        Layout.fillWidth: true
                    }

                    ComboBox {
                        id: txSuffixComboBox
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

                    Label {
                        text: qsTr("Rx Format")
                        font.pointSize: 12
                        Layout.fillWidth: true
                    }

                    ComboBox {
                        id: rxFormatComboBox
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
                flat: true

                onClicked: swipeView.currentIndex = swipeView.currentIndex - 1
            }

            PageIndicator {
                id: pageIndicator
                count: swipeView.count
                currentIndex: swipeView.currentIndex
                Layout.fillWidth: false
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                background: Rectangle{
                    color: "transparent"
                }
            }

            Button {
                text: swipeView.currentIndex !== 2 ? qsTr("Next") : qsTr("Confirm")
                highlighted: swipeView.currentIndex === 2
                flat: true

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
                                break
                            case 1: {
                                if (!visaNameComboBox.currentText) {
                                    portNameValidator.text = qsTr("Invalid Port Name")
                                    portNameValidatorTimer.start()
                                    return
                                }
                            }
                                break
                            case 2: {
                                if (!tcpClientNameTextField.text) {
                                    portNameValidator.text = qsTr("Invalid Port Name")
                                    portNameValidatorTimer.start()
                                    return
                                }
                                if (!tcpClientRemoteHostTextField.text) {
                                    portNameValidator.text = qsTr("Invalid Remote Host")
                                    portNameValidatorTimer.start()
                                    return
                                }
                            }
                                break
                            case 3: {
                                if (!tcpServerNameTextField.text) {
                                    portNameValidator.text = qsTr("Invalid Port Name")
                                    portNameValidatorTimer.start()
                                    return
                                }
                                if (!tcpServerLocalHostTextField.text) {
                                    portNameValidator.text = qsTr("Invalid Local Host")
                                    portNameValidatorTimer.start()
                                    return
                                }
                            }
                                break
                            case 4: {
                                if (!udpSocketNameTextField.text) {
                                    portNameValidator.text = qsTr("Invalid Port Name")
                                    portNameValidatorTimer.start()
                                    return
                                }
                                if (!udpSocketLocalHostTextField.text) {
                                    portNameValidator.text = qsTr("Invalid Local Host")
                                    portNameValidatorTimer.start()
                                    return
                                }
                                if (!udpSocketRemoteHostTextField.text) {
                                    portNameValidator.text = qsTr("Invalid Remote Host")
                                    portNameValidatorTimer.start()
                                    return
                                }
                            }
                                break
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
            "swipeView": swipeView,
            "tumbler": tumbler,
            // serial port
            "serialPortNameComboBox": serialPortNameComboBox,
            "serialPortBaudRateSpinBox": serialPortBaudRateSpinBox,
            "serialPortDataBitsComboBox": serialPortDataBitsComboBox,
            "serialPortParityComboBox": serialPortParityComboBox,
            "serialPortStopBitsComboBox": serialPortStopBitsComboBox,
            // visa
            "visaNameComboBox": visaNameComboBox,
            // tcp client
            "tcpClientNameTextField": tcpClientNameTextField,
            "tcpClientRemoteHostTextField": tcpClientRemoteHostTextField,
            "tcpClientRemotePortSpinBox": tcpClientRemotePortSpinBox,
            // tcp server
            "tcpServerNameTextField": tcpServerNameTextField,
            "tcpServerLocalHostTextField": tcpServerLocalHostTextField,
            "tcpServerLocalPortSpinBox": tcpServerLocalPortSpinBox,
            // udp socket
            "udpSocketNameTextField": udpSocketNameTextField,
            "udpSocketLocalHostTextField": udpSocketLocalHostTextField,
            "udpSocketLocalPortSpinBox": udpSocketLocalPortSpinBox,
            "udpSocketRemoteHostTextField": udpSocketRemoteHostTextField,
            "udpSocketRemotePortSpinBox": udpSocketRemotePortSpinBox,
            // format
            "txFormatComboBox": txFormatComboBox,
            "txSuffixComboBox": txSuffixComboBox,
            "rxFormatComboBox": rxFormatComboBox
        };
        portSetting.propertyGet(objects)
    }
}
