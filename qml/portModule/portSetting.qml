import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    property int portType: 0
    property var patterns: {
        "ipv4": /^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/,
        "ipv6": /[0-9a-fA-F]{1,4}(:[0-9a-fA-F]{1,4}){3}/
    }

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
                    currentIndex: rootItem.portType
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
                    Layout.fillWidth: true; Layout.fillHeight: false

                    ColumnLayout {

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

                    // serial port
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

                    // visa
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

                    // tcp client
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

                    // tcp server
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

                        ComboBox {
                            id: tcpServerLocalHostComboBox
                            model: localHostStandardItemModel
                            textRole: "display"
                            valueRole: "whatsThis"
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

                    // udp socket
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

                        ComboBox {
                            id: udpSocketLocalHostComboBox
                            model: localHostStandardItemModel
                            textRole: "display"
                            valueRole: "whatsThis"
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

                    // screen
                    GridLayout {
                        columns: 2
                        columnSpacing: 20; rowSpacing: 20

                        Label {
                            text: qsTr("Port Name")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        ComboBox {
                            id: screenNameComboBox
                            model: screenStandardItemModel
                            textRole: "display"
                            valueRole: "whatsThis"
                            Layout.fillWidth: true
                        }
                    }

                    // camera
                    GridLayout {
                        columns: 2
                        columnSpacing: 20; rowSpacing: 20

                        Label {
                            text: qsTr("Port Name")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        ComboBox {
                            id: cameraNameComboBox
                            model: cameraStandardItemModel
                            textRole: "display"
                            valueRole: "whatsThis"
                            Layout.fillWidth: true
                        }
                    }
                }

                Label {
                    id: portNameValidator
                    Layout.fillWidth: true; Layout.fillHeight: false
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    color: "#c50f1f"
                    text: ""
                    font.pointSize: 16
                }

                Timer {
                    id: portNameValidatorTimer
                    interval: 3000
                    onTriggered: portNameValidator.text = ""
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

                // format
                GridLayout {
                    columns: 2
                    columnSpacing: 20; rowSpacing: 20
                    Layout.fillWidth: true; Layout.fillHeight: true

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

                // image
                RowLayout {
                    Layout.fillWidth: true; Layout.fillHeight: true

                    ColumnLayout {
                        Layout.fillWidth: true; Layout.fillHeight: true

                        ScrollView {
                            Layout.fillWidth: true; Layout.fillHeight: true
                            contentWidth: captureImage.width; contentHeight: captureImage.height

                            Image {
                                id: captureImage
                                property var indicatorList: []
                            }

                            Rectangle {
                                id: captureSelection
                                anchors.fill: captureImage
                                color: "white"
                                opacity: 0.5
                                visible: false
                                property bool roi: false
                                property point anchorLT
                                property point anchorRB

                                TapHandler {
                                    acceptedButtons: Qt.LeftButton
                                    enabled: captureSelection.visible

                                    onTapped: (eventPoint) => {
                                        const position = eventPoint.position;
                                        if (!captureSelection.roi) {
                                            captureSelection.anchorLT = position
                                            captureSelection.roi = true
                                            hintLabel.text = qsTr("Select second anchor")
                                        } else {
                                            captureSelection.anchorRB = position
                                            captureSelection.roi = false
                                            captureSelection.visible = false
                                            let ix = Math.round(Math.min(captureSelection.anchorLT.x, captureSelection.anchorRB.x))
                                            let iy = Math.round(Math.min(captureSelection.anchorLT.y, captureSelection.anchorRB.y))
                                            let iw = Math.abs(captureSelection.anchorRB.x - captureSelection.anchorLT.x)
                                            let ih = Math.abs(captureSelection.anchorRB.y - captureSelection.anchorLT.y)
                                            portSetting.roiInsert(ix, iy, iw, ih)
                                            hintLabel.text = qsTr("")
                                        }
                                    }
                                }

                                TapHandler {
                                    acceptedButtons: Qt.RightButton
                                    enabled: captureSelection.visible

                                    onTapped: captureSelection.visible = false
                                }
                            }
                        }

                        RowLayout {

                            ToolBar {
                                Layout.fillWidth: true; Layout.fillHeight: false

                                RowLayout {

                                    ToolButton {
                                        Layout.preferredWidth: 48; Layout.preferredHeight: 48
                                        leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                        icon.source: "qrc:/icon/refresh.svg"
                                        icon.width: 32; icon.height: 32
                                        checkable: true
                                        ToolTip.text: qsTr("Auto Refresh")
                                        ToolTip.visible: hovered

                                        Timer {
                                            interval: 1000
                                            running: parent.checked
                                            repeat: true
                                            onTriggered: {
                                                if (rootItem.portType === 5) {
                                                    portSetting.screenCapture()
                                                } else {
                                                    portSetting.cameraCapture()
                                                }
                                            }
                                        }
                                    }

                                    ToolSeparator {
                                    }

                                    ToolButton {
                                        Layout.preferredWidth: 48; Layout.preferredHeight: 48
                                        leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                        icon.source: "qrc:/icon/squareHint.svg"
                                        icon.width: 32; icon.height: 32
                                        ToolTip.text: qsTr("Rectangular")
                                        ToolTip.visible: hovered

                                        onClicked: {
                                            captureSelection.visible = true
                                            hintLabel.text = qsTr("Select first anchor")
                                        }
                                    }
                                }
                            }

                            Label {
                                id: hintLabel
                                font.pixelSize: 20
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.preferredWidth: 300; Layout.fillHeight: true

                        RowLayout {

                            Label {
                                text: qsTr("Region of Interest")
                                font.pixelSize: 20
                            }

                            Image {
                                source: "qrc:/icon/roi.svg"
                            }
                        }

                        Component {
                            id: roiTableComponent

                            Item {
                                anchors.fill: parent

                                VerticalHeaderView {
                                    id: roiVerticalHeaderView
                                    anchors.left: parent.left
                                    width: 32; height: parent.height
                                    syncView: roiTableView
                                    clip: true
                                    interactive: false
                                    movableRows: true
                                    delegate: VerticalHeaderViewDelegate {
                                        id: roiVerticalHeaderViewDelegate
                                        implicitWidth: roiVerticalHeaderView.width; implicitHeight: 32
                                        padding: 0

                                        contentItem: Rectangle {
                                            width: 32; height: 32
                                            color: "white"

                                            Image {
                                                width: 16; height: 16
                                                anchors.centerIn: parent
                                                source: "qrc:/icon/drag.svg"
                                            }
                                        }

                                        HoverHandler {
                                            onHoveredChanged: cursorShape = Qt.OpenHandCursor
                                        }
                                    }
                                    property var moves: []

                                    Rectangle {
                                        anchors.fill: parent
                                        color: "#e0e0e0"
                                        z: -1
                                    }

                                    Timer {
                                        id: timer
                                        interval: 10
                                        onTriggered: {
                                            let index = -1
                                            let distance = -1
                                            let currentDistance;
                                            for (let i = 0; i < roiVerticalHeaderView.moves.length; ++i) {
                                                let move = roiVerticalHeaderView.moves[i]
                                                currentDistance = Math.abs(move.oldVisualIndex - move.newVisualIndex)
                                                if (currentDistance > distance) {
                                                    distance = currentDistance
                                                    index = i
                                                }
                                            }
                                            let move = roiVerticalHeaderView.moves[index]
                                            portSetting.roiSwap(move.oldVisualIndex, move.newVisualIndex)
                                            roiVerticalHeaderView.moves = []
                                        }
                                    }

                                    onRowMoved: (logicalIndex, oldVisualIndex, newVisualIndex) => {
                                        moves.push({oldVisualIndex, newVisualIndex})
                                        timer.restart()
                                    }
                                }

                                TableView {
                                    id: roiTableView
                                    anchors.left: roiVerticalHeaderView.right; anchors.right: parent.right
                                    height: parent.height
                                    alternatingRows: false
                                    clip: true
                                    editTriggers: TableView.NoEditTriggers
                                    rowSpacing: 1
                                    model: roiStandardItemModel
                                    contentWidth: width
                                    delegate: ItemDelegate {
                                        implicitWidth: parent.width; implicitHeight: 32
                                        text: model.display
                                        font.pixelSize: 16
                                        background: Rectangle {
                                            color: "white"
                                        }

                                        Rectangle {
                                            id: highlightRect
                                            anchors.fill: parent
                                            radius: 2
                                            color: "#f5f5f5"
                                            opacity: hoverHandler.hovered ? 1 : 0
                                            Behavior on opacity {
                                                NumberAnimation {
                                                    duration: 150
                                                }
                                            }
                                        }

                                        HoverHandler {
                                            id: hoverHandler
                                        }

                                        TapHandler {
                                            acceptedButtons: Qt.RightButton
                                            gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                                            onSingleTapped: {
                                                roiMenu.roiIndex = model.row
                                                roiMenu.popup()
                                            }
                                        }
                                    }

                                    Rectangle {
                                        anchors.fill: parent
                                        color: "#e0e0e0"
                                        z: -1
                                    }
                                }

                                Menu {
                                    id: roiMenu
                                    property int roiIndex

                                    MenuItem {
                                        text: qsTr("Delete")
                                        icon.source: "qrc:/icon/delete.svg"
                                        icon.width: 16; icon.height: 16

                                        onTriggered: portSetting.roiRemove(roiMenu.roiIndex)
                                    }
                                }
                            }
                        }

                        Loader {
                            id: roiTableLoader
                            Layout.fillWidth: true; Layout.fillHeight: true
                            sourceComponent: roiTableComponent
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: false
            Layout.alignment: Qt.AlignHCenter
            Layout.margins: 20

            Button {
                text: qsTr("Prev")
                enabled: swipeView.currentIndex !== 0
                flat: true

                onClicked: {
                    swipeView.currentIndex = swipeView.currentIndex - 1
                    portSetting.dialogResize(600, 500)
                }
            }

            PageIndicator {
                id: pageIndicator
                count: swipeView.count
                currentIndex: swipeView.currentIndex
                Layout.fillWidth: false
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                background: Rectangle {
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
                        switch (rootItem.portType) {
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
                                    portNameValidator.text = qsTr("Empty Remote Host")
                                    portNameValidatorTimer.start()
                                    return
                                } else if (!(rootItem.patterns["ipv4"].test(tcpClientRemoteHostTextField.text)
                                    || rootItem.patterns["ipv6"].test(tcpClientRemoteHostTextField.text))) {
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
                                if (!tcpServerLocalHostComboBox.currentText) {
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
                                if (!udpSocketLocalHostComboBox.currentText) {
                                    portNameValidator.text = qsTr("Invalid Local Host")
                                    portNameValidatorTimer.start()
                                    return
                                }
                                if (!udpSocketRemoteHostTextField.text) {
                                    portNameValidator.text = qsTr("Invalid Remote Host")
                                    portNameValidatorTimer.start()
                                    return
                                } else if (!(rootItem.patterns["ipv4"].test(udpSocketRemoteHostTextField.text)
                                    || rootItem.patterns["ipv6"].test(udpSocketRemoteHostTextField.text))) {
                                    portNameValidator.text = qsTr("Invalid Remote Host")
                                    portNameValidatorTimer.start()
                                    return
                                }
                            }
                                break
                            case 5: {
                                portSetting.screenCapture()
                            }
                                break
                            case 6: {
                                portSetting.cameraCapture()
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
            "tcpServerLocalHostComboBox": tcpServerLocalHostComboBox,
            "tcpServerLocalPortSpinBox": tcpServerLocalPortSpinBox,
            // udp socket
            "udpSocketNameTextField": udpSocketNameTextField,
            "udpSocketLocalHostComboBox": udpSocketLocalHostComboBox,
            "udpSocketLocalPortSpinBox": udpSocketLocalPortSpinBox,
            "udpSocketRemoteHostTextField": udpSocketRemoteHostTextField,
            "udpSocketRemotePortSpinBox": udpSocketRemotePortSpinBox,
            // screen
            "screenNameComboBox": screenNameComboBox,
            // camera
            "cameraNameComboBox": cameraNameComboBox,
            // format
            "txFormatComboBox": txFormatComboBox,
            "txSuffixComboBox": txSuffixComboBox,
            "rxFormatComboBox": rxFormatComboBox,
            // image
            "captureImage": captureImage
        };
        portSetting.propertyGet(objects)
    }

    function roiReload() {
        roiTableLoader.active = false
        roiTableLoader.active = true
    }

    Component {
        id: indicatorComponent

        Rectangle {
            color: "#a9d3f2"
            opacity: 0.5
            border.color: "#0078d4"; border.width: 1
            property int index

            Label {
                text: index.toString()
                font.bold: true
                font.pixelSize: 20
                background: Rectangle {
                    color: "#0078d4"
                }
            }
        }
    }

    function indicatorReload() {
        for (let i = 0; i < captureImage.indicatorList.length; ++i) {
            captureImage.indicatorList[i].destroy()
        }
        captureImage.indicatorList = []
        for (let row = 0; row < roiStandardItemModel.rowCount(); ++row) {
            const index = roiStandardItemModel.index(row, 0);
            const position = roiStandardItemModel.data(index, Qt.WhatsThisRole);
            const indicator = indicatorComponent.createObject(captureImage, {
                x: position[0],
                y: position[1],
                width: position[2],
                height: position[3],
                index: row
            });
            captureImage.indicatorList.push(indicator)
        }
    }
}
