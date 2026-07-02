import QtMultimedia
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts
import QtQuick.Shapes

Item {
    id: rootItem
    property int portType: 0
    property var patterns: {
        "ipv4": /^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/,
        "ipv6": /[0-9a-fA-F]{1,4}(:[0-9a-fA-F]{1,4}){3}/
    }

    Rectangle {
        anchors.fill: parent
        color: global.back
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

            // port type selection
            ColumnLayout {
                Layout.fillWidth: true; Layout.fillHeight: true

                Tumbler {
                    id: tumbler
                    currentIndex: rootItem.portType
                    delegate: delegateComponent
                    model: [qsTr("Serial Port"), qsTr("Visa"), qsTr("Tcp Client"), qsTr("Ssl Client"), qsTr("Tcp Server"), qsTr("Udp Socket"), qsTr("Vedio Stream")]
                    wrap: false
                    Layout.fillWidth: true; Layout.fillHeight: true

                    onMovingChanged: {
                        if (!moving) {
                            rootItem.portType = currentIndex
                        }
                    }
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
                    Layout.fillWidth: true; Layout.fillHeight: false

                    IconImage {
                        visible: source !== ""
                        source: sourceGet()
                        sourceSize: Qt.size(80, 80)
                        color: global.fore
                        Layout.preferredWidth: 80; Layout.preferredHeight: 80
                        Layout.alignment: Qt.AlignHCenter

                        function sourceGet() {
                            switch (rootItem.portType) {
                                case 0:
                                    return "qrc:/icon/serialPort.svg"
                                case 1:
                                    return ""
                                case 2:
                                    return "qrc:/icon/tcpClient.svg"
                                case 3:
                                    return ""
                                case 4:
                                    return "qrc:/icon/tcpServer.svg"
                                case 5:
                                    return "qrc:/icon/udpSocket.svg"
                                case 6:
                                    return "qrc:/icon/video.svg"
                                default:
                                    return ""
                            }
                        }
                    }

                    Label {
                        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                        text: textGet()
                        wrapMode: Text.WordWrap
                        font.pointSize: 12
                        Layout.alignment: Qt.AlignHCenter; Layout.fillWidth: true

                        function textGet() {
                            switch (rootItem.portType) {
                                case 0:
                                    return qsTr("A serial communication interface through which information transfers in or out sequentially one bit at a time.")
                                case 1:
                                    return qsTr("A widely used application programming interface (API) in the test and measurement (T&M) industry for communicating with instruments from a computer.")
                                case 2:
                                    return qsTr("A device that initiates a connection with a TCP server to send and receive reliable, ordered data over a network.")
                                case 3:
                                    return qsTr("A secure client that establishes encrypted connections with SSL/TLS servers to ensure data confidentiality and integrity during transmission.")
                                case 4:
                                    return qsTr("A device that listens on a network port, accepts incoming connections from TCP clients, and manages reliable, ordered data exchange.")
                                case 5:
                                    return qsTr("A device that uses the User Datagram Protocol to send independent, connectionless messages (datagrams) over an IP network.")
                                case 6:
                                    return qsTr("Video stream for image processing and OCR text recognition.")
                                default:
                                    return ""
                            }
                        }
                    }
                }
            }

            // port setting
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
                            font.pointSize: 12
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
                            font.pointSize: 12
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
                            Layout.fillWidth: true
                        }

                        Label {
                            text: qsTr("Parity")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        ComboBox {
                            id: serialPortParityComboBox
                            font.pointSize: 12
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
                            Layout.fillWidth: true
                        }

                        Label {
                            text: qsTr("Stop Bits")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        ComboBox {
                            id: serialPortStopBitsComboBox
                            font.pointSize: 12
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
                            font.pointSize: 12
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
                            font.pointSize: 12
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

                    // ssl client
                    GridLayout {
                        columns: 2
                        columnSpacing: 20; rowSpacing: 20

                        Label {
                            text: qsTr("Port Name")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        TextField {
                            id: sslClientNameTextField
                            font.pointSize: 12
                            placeholderText: qsTr("Ssl Client")
                            Layout.fillWidth: true
                        }

                        Label {
                            text: qsTr("Remote Host")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        TextField {
                            id: sslClientRemoteHostTextField
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        Label {
                            text: qsTr("Remote Port")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        SpinBox {
                            id: sslClientRemotePortSpinBox
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
                            font.pointSize: 12
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
                            font.pointSize: 12
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
                            font.pointSize: 12
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
                            font.pointSize: 12
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

                    // video stream
                    GridLayout {
                        columns: 2
                        columnSpacing: 20; rowSpacing: 20

                        Label {
                            text: qsTr("Port Name")
                            font.pointSize: 12
                            Layout.fillWidth: true
                        }

                        ComboBox {
                            id: videoStreamNameComboBox
                            font.pointSize: 12
                            model: videoStreamStandardItemModel
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

            // port format
            StackLayout {
                currentIndex: {
                    if (rootItem.portType in [0, 1, 2, 3, 4, 5]) {
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
                                text: qsTr("modbus crc"); value: "modbus crc"
                            }
                            ListElement {
                                text: qsTr("modbus lrc"); value: "modbus lrc"
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

                    Label {
                        text: qsTr("Buffer Size")
                        font.pointSize: 12
                        Layout.fillWidth: true
                    }

                    SpinBox {
                        id: bufferSizeSpinBox
                        font.pointSize: 12
                        editable: true
                        from: 1
                        to: 1048576 // 1MB
                        Layout.fillWidth: true
                    }
                }

                // image
                SplitView {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    orientation: Qt.Horizontal
                    handle: Item {
                        implicitWidth: 5

                        Rectangle {
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            width: 1
                            color: global.stroke
                        }
                    }

                    ColumnLayout {
                        SplitView.fillWidth: true; SplitView.fillHeight: true

                        Flickable {
                            clip: true
                            contentWidth: videoOutput.width
                            contentHeight: videoOutput.height
                            Layout.fillWidth: true; Layout.fillHeight: true

                            ScrollBar.vertical: ScrollBar {
                                policy: ScrollBar.AsNeeded
                                palette {
                                    mid: global.stroke
                                    dark: global.strokePressed
                                }
                            }
                            ScrollBar.horizontal: ScrollBar {
                                policy: ScrollBar.AsNeeded
                                palette {
                                    mid: global.stroke
                                    dark: global.strokePressed
                                }
                            }

                            VideoOutput {
                                id: videoOutput
                                property var indicatorList: []
                            }

                            Rectangle {
                                id: roiSelection
                                anchors.fill: videoOutput
                                color: global.stroke
                                opacity: 0.2
                                visible: step !== 0
                                property int step: 0
                                property var roiList: []

                                TapHandler {
                                    acceptedButtons: Qt.LeftButton
                                    enabled: roiSelection.visible

                                    onTapped: (eventPoint) => {
                                        const position = eventPoint.position;
                                        roiSelection.roiRecord(position)
                                    }
                                }

                                Shortcut {
                                    enabled: roiSelection.visible
                                    sequence: "Esc"

                                    onActivated: roiSelection.roiStop()
                                }

                                HoverHandler {
                                    onHoveredChanged: {
                                        if (!hovered) {
                                            mainToolTip.text = ""
                                        }
                                    }
                                    onPointChanged: {
                                        mainToolTip.position = parent.mapToGlobal(point.position)
                                        mainToolTip.text = qsTr("%1, %2\nSelect %3/%4  Esc: Exit")
                                            .arg(Math.round(point.position.x))
                                            .arg(Math.round(point.position.y))
                                            .arg(roiSelection.roiList.length + 1)
                                            .arg(roiSelection.step)
                                    }
                                }

                                function roiRecord(position) {
                                    roiList.push(position)
                                    if (roiList.length === step) {
                                        if (step === 2) {
                                            let ix = Math.round(Math.min(roiList[0].x, roiList[1].x))
                                            let iy = Math.round(Math.min(roiList[0].y, roiList[1].y))
                                            let iw = Math.round(Math.abs(roiList[1].x - roiList[0].x))
                                            let ih = Math.round(Math.abs(roiList[1].y - roiList[0].y))
                                            portSetting.roiInsert([ix, iy, iw, ih])
                                        } else if (step === 4) {
                                            let ix0 = Math.round(roiList[0].x)
                                            let iy0 = Math.round(roiList[0].y)
                                            let ix1 = Math.round(roiList[1].x)
                                            let iy1 = Math.round(roiList[1].y)
                                            let ix2 = Math.round(roiList[2].x)
                                            let iy2 = Math.round(roiList[2].y)
                                            let ix3 = Math.round(roiList[3].x)
                                            let iy3 = Math.round(roiList[3].y)
                                            portSetting.roiInsert([ix0, iy0, ix1, iy1, ix2, iy2, ix3, iy3])
                                        }
                                        roiSelection.roiStop()
                                    }
                                }

                                function roiStart(step) {
                                    roiSelection.step = step
                                }

                                function roiStop() {
                                    roiSelection.step = 0
                                    roiSelection.roiList = []
                                    mainToolTip.text = ""
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true

                            Button {
                                flat: true
                                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                icon.source: "qrc:/icon/rectangle.svg"
                                icon.width: 32; icon.height: 32
                                Layout.preferredWidth: 48; Layout.preferredHeight: 48

                                onClicked: roiSelection.roiStart(2)

                                HoverHandler {
                                    onHoveredChanged: {
                                        if (!hovered) {
                                            mainToolTip.text = ""
                                        }
                                    }
                                    onPointChanged: {
                                        mainToolTip.position = parent.mapToGlobal(point.position)
                                        mainToolTip.text = qsTr("Rectangular")
                                    }
                                }
                            }

                            Button {
                                flat: true
                                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                icon.source: "qrc:/icon/quadrilateral.svg"
                                icon.width: 32; icon.height: 32
                                Layout.preferredWidth: 48; Layout.preferredHeight: 48

                                onClicked: roiSelection.roiStart(4)

                                HoverHandler {
                                    onHoveredChanged: {
                                        if (!hovered) {
                                            mainToolTip.text = ""
                                        }
                                    }
                                    onPointChanged: {
                                        mainToolTip.position = parent.mapToGlobal(point.position)
                                        mainToolTip.text = qsTr("Quadrilateral")
                                    }
                                }
                            }

                            Item {
                                Layout.fillWidth: true
                            }
                        }
                    }

                    SplitView {
                        SplitView.preferredWidth: 300; SplitView.fillHeight: true
                        orientation: Qt.Vertical
                        handle: Item {
                            implicitHeight: 5

                            Rectangle {
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: 1
                                color: global.stroke
                            }
                        }

                        // preview
                        ColumnLayout {
                            SplitView.fillWidth: true; SplitView.preferredHeight: 200

                            Label {
                                leftPadding: 6
                                horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                                font.pixelSize: 20
                                text: qsTr("Image Preview")
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }

                            Flickable {
                                clip: true
                                contentWidth: previewImage.width
                                contentHeight: previewImage.height
                                Layout.fillWidth: true; Layout.fillHeight: true

                                ScrollBar.vertical: ScrollBar {
                                    policy: ScrollBar.AsNeeded
                                    palette {
                                        mid: global.stroke
                                        dark: global.strokePressed
                                    }
                                }
                                ScrollBar.horizontal: ScrollBar {
                                    policy: ScrollBar.AsNeeded
                                    palette {
                                        mid: global.stroke
                                        dark: global.strokePressed
                                    }
                                }

                                Image {
                                    id: previewImage
                                    property int selectedRow: -1
                                }

                                Timer {
                                    interval: 200 // 5Hz
                                    repeat: true
                                    running: !roiModel.empty

                                    onTriggered: portSetting.previewLoad(previewImage.selectedRow, recognitionComboBox.currentIndex)
                                }
                            }
                        }

                        // roi
                        ColumnLayout {
                            SplitView.fillWidth: true; SplitView.preferredHeight: 200

                            Label {
                                leftPadding: 6
                                horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                                font.pixelSize: 20
                                text: qsTr("ROI")
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }

                            Item {
                                Layout.fillWidth: true; Layout.fillHeight: true

                                Item {
                                    anchors.fill: parent
                                    visible: roiModel.empty

                                    RowLayout {
                                        anchors.centerIn: parent

                                        Label {
                                            text: qsTr("Create ROI first.")
                                            font.pixelSize: 16
                                            Layout.alignment: Qt.AlignVCenter
                                        }

                                        IconImage {
                                            source: "qrc:/icon/rectangle.svg"
                                            color: global.fore
                                            Layout.alignment: Qt.AlignVCenter
                                        }
                                    }
                                }

                                Loader {
                                    id: roiTableLoader
                                    anchors.fill: parent
                                    sourceComponent: roiTableComponent
                                }

                                Component {
                                    id: roiTableComponent

                                    Item {
                                        anchors.fill: parent
                                        visible: !roiModel.empty

                                        VerticalHeaderView {
                                            id: roiVerticalHeaderView
                                            anchors.left: parent.left
                                            width: 24; height: parent.height
                                            syncView: roiTableView
                                            clip: true
                                            interactive: false
                                            movableRows: true
                                            delegate: VerticalHeaderViewDelegate {
                                                id: roiVerticalHeaderViewDelegate
                                                implicitWidth: roiVerticalHeaderView.width; implicitHeight: 24
                                                padding: 0

                                                contentItem: Rectangle {
                                                    width: 24; height: 24
                                                    color: global.back

                                                    IconImage {
                                                        anchors.centerIn: parent
                                                        width: 16; height: 16
                                                        color: global.fore
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
                                                color: global.stroke
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
                                            model: roiModel
                                            contentWidth: width
                                            property int hoveredRow: -1
                                            property int selectedRow: -1

                                            ScrollBar.vertical: ScrollBar {
                                                policy: ScrollBar.AsNeeded
                                                palette {
                                                    mid: global.stroke
                                                    dark: global.strokePressed
                                                }
                                            }

                                            Rectangle {
                                                anchors.fill: parent
                                                color: global.stroke
                                            }

                                            delegate: Item {
                                                implicitWidth: parent.width; implicitHeight: 24
                                                required property int row

                                                Rectangle {
                                                    anchors.fill: parent
                                                    color: global.back
                                                }

                                                Rectangle {
                                                    anchors.fill: parent
                                                    radius: 6
                                                    color: global.backHover
                                                    opacity: roiTableView.hoveredRow === row ? 1 : 0
                                                    Behavior on opacity {
                                                        NumberAnimation {
                                                            duration: 150
                                                        }
                                                    }
                                                }

                                                Rectangle {
                                                    anchors.fill: parent
                                                    radius: 6
                                                    color: roiTableView.selectedRow === row ? global.backSelected : "transparent"
                                                }

                                                Label {
                                                    id: label
                                                    anchors.fill: parent
                                                    leftPadding: 6
                                                    horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                                                    text: model.display || ""
                                                    elide: Text.ElideRight
                                                }

                                                HoverHandler {
                                                    id: hoverHandler
                                                }

                                                TapHandler {
                                                    acceptedButtons: Qt.LeftButton
                                                    gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                                                    onTapped: {
                                                        roiTableView.selectedRow = row
                                                        previewImage.selectedRow = row
                                                    }
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

                                            TapHandler {
                                                acceptedButtons: Qt.LeftButton
                                                gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                                                onTapped: {
                                                    roiTableView.selectedRow = -1
                                                    previewImage.selectedRow = -1
                                                }
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
                            }
                        }

                        // pipeline
                        ColumnLayout {
                            SplitView.fillWidth: true; SplitView.preferredHeight: 200

                            RowLayout {
                                Layout.fillWidth: true

                                Label {
                                    leftPadding: 6
                                    horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                                    font.pixelSize: 20
                                    text: qsTr("Image Pipeline")
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }

                                Button {
                                    flat: true
                                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                    icon.source: "qrc:/icon/add.svg"
                                    icon.width: 24; icon.height: 24
                                    Layout.preferredWidth: 24; Layout.preferredHeight: 24

                                    onClicked: pipelineMenu.popup()

                                    HoverHandler {
                                        onHoveredChanged: {
                                            if (!hovered) {
                                                mainToolTip.text = ""
                                            }
                                        }
                                        onPointChanged: {
                                            mainToolTip.position = parent.mapToGlobal(point.position)
                                            mainToolTip.text = qsTr("Add Pipeline")
                                        }
                                    }
                                }
                            }

                            Item {
                                Layout.fillWidth: true; Layout.fillHeight: true

                                Item {
                                    anchors.fill: parent
                                    visible: pipelineModel.empty

                                    RowLayout {
                                        anchors.centerIn: parent

                                        Label {
                                            text: qsTr("Raw output.")
                                            font.pixelSize: 16
                                            Layout.alignment: Qt.AlignVCenter
                                        }
                                    }
                                }

                                Loader {
                                    id: pipelineTableLoader
                                    anchors.fill: parent
                                    sourceComponent: pipelineTableComponent
                                }

                                Component {
                                    id: pipelineTableComponent

                                    Item {
                                        anchors.fill: parent
                                        visible: !pipelineModel.empty

                                        VerticalHeaderView {
                                            id: pipelineVerticalHeaderView
                                            anchors.left: parent.left
                                            width: 24; height: parent.height
                                            syncView: pipelineTableView
                                            clip: true
                                            interactive: false
                                            movableRows: true
                                            delegate: VerticalHeaderViewDelegate {
                                                id: pipelineVerticalHeaderViewDelegate
                                                implicitWidth: pipelineVerticalHeaderView.width; implicitHeight: 24
                                                padding: 0

                                                contentItem: Rectangle {
                                                    width: 24; height: 24
                                                    color: global.back

                                                    IconImage {
                                                        anchors.centerIn: parent
                                                        width: 16; height: 16
                                                        color: global.fore
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
                                                color: global.stroke
                                            }

                                            Timer {
                                                id: timer
                                                interval: 10
                                                onTriggered: {
                                                    let index = -1
                                                    let distance = -1
                                                    let currentDistance;
                                                    for (let i = 0; i < pipelineVerticalHeaderView.moves.length; ++i) {
                                                        let move = pipelineVerticalHeaderView.moves[i]
                                                        currentDistance = Math.abs(move.oldVisualIndex - move.newVisualIndex)
                                                        if (currentDistance > distance) {
                                                            distance = currentDistance
                                                            index = i
                                                        }
                                                    }
                                                    let move = pipelineVerticalHeaderView.moves[index]
                                                    portSetting.pipelineSwap(move.oldVisualIndex, move.newVisualIndex)
                                                    pipelineVerticalHeaderView.moves = []
                                                }
                                            }

                                            onRowMoved: (logicalIndex, oldVisualIndex, newVisualIndex) => {
                                                moves.push({oldVisualIndex, newVisualIndex})
                                                timer.restart()
                                            }
                                        }

                                        TableView {
                                            id: pipelineTableView
                                            anchors.left: pipelineVerticalHeaderView.right; anchors.right: parent.right
                                            height: parent.height
                                            alternatingRows: false
                                            clip: true
                                            editTriggers: TableView.NoEditTriggers
                                            rowSpacing: 1
                                            model: pipelineModel
                                            contentWidth: width

                                            ScrollBar.vertical: ScrollBar {
                                                policy: ScrollBar.AsNeeded
                                                palette {
                                                    mid: global.stroke
                                                    dark: global.strokePressed
                                                }
                                            }

                                            Rectangle {
                                                anchors.fill: parent
                                                color: global.stroke
                                            }

                                            delegate: DelegateChooser {
                                                role: "display"

                                                DelegateChoice {
                                                    roleValue: qsTr("Scale")
                                                    delegate: scaleDelegate
                                                }

                                                DelegateChoice {
                                                    roleValue: qsTr("Threshold")
                                                    delegate: thresholdDelegate
                                                }
                                            }

                                            Component {
                                                id: scaleDelegate

                                                Item {
                                                    id: scaleItem
                                                    implicitWidth: parent.width; implicitHeight: 80
                                                    property var session: model.whatsThis

                                                    Rectangle {
                                                        anchors.fill: parent
                                                        color: global.back
                                                    }

                                                    ColumnLayout {
                                                        anchors.fill: parent
                                                        anchors.leftMargin: 10

                                                        RowLayout {

                                                            Label {
                                                                leftPadding: 6
                                                                horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                                                                text: model.display || ""
                                                                elide: Text.ElideRight
                                                                Layout.fillWidth: true
                                                            }

                                                            ComboBox {
                                                                id: scaleComboBox
                                                                currentIndex: session.interpolation
                                                                model: ListModel {
                                                                    ListElement {
                                                                        text: qsTr("Nearest")
                                                                    }
                                                                    ListElement {
                                                                        text: qsTr("Linear")
                                                                    }
                                                                    ListElement {
                                                                        text: qsTr("Cubic")
                                                                    }
                                                                    ListElement {
                                                                        text: qsTr("Area")
                                                                    }
                                                                    ListElement {
                                                                        text: qsTr("Lanczos4")
                                                                    }
                                                                    ListElement {
                                                                        text: qsTr("Linear Exact")
                                                                    }
                                                                    ListElement {
                                                                        text: qsTr("Nearest Exact")
                                                                    }
                                                                }
                                                                textRole: "text"
                                                                property bool initialized: false

                                                                Component.onCompleted: {
                                                                    initialized = true
                                                                }

                                                                onCurrentTextChanged: {
                                                                    if (!scaleComboBox.initialized) return
                                                                    scaleItem.session.interpolation = scaleComboBox.currentIndex
                                                                    const index = pipelineModel.index(row, 0);
                                                                    pipelineModel.setData(index, scaleItem.session, Qt.WhatsThisRole)
                                                                }
                                                            }
                                                        }

                                                        Slider {
                                                            id: scaleSlider
                                                            value: session.ratio
                                                            from: -5
                                                            to: 5
                                                            stepSize: 1
                                                            snapMode: Slider.SnapOnRelease
                                                            Layout.fillWidth: true
                                                            ToolTip.text: "x" + ratio.toString()
                                                            ToolTip.visible: hovered
                                                            property real ratio: 1

                                                            onMoved: {
                                                                switch (value) {
                                                                    case -5:
                                                                        ratio = 0.1
                                                                        break
                                                                    case -4:
                                                                        ratio = 0.25
                                                                        break
                                                                    case -3:
                                                                        ratio = 0.3
                                                                        break
                                                                    case -2:
                                                                        ratio = 0.5
                                                                        break
                                                                    case -1:
                                                                        ratio = 0.75
                                                                        break
                                                                    case 0:
                                                                        ratio = 1
                                                                        break
                                                                    case 1:
                                                                        ratio = 1.5
                                                                        break
                                                                    case 2:
                                                                        ratio = 2
                                                                        break
                                                                    case 3:
                                                                        ratio = 3
                                                                        break
                                                                    case 4:
                                                                        ratio = 5
                                                                        break
                                                                    case 5:
                                                                        ratio = 10
                                                                        break
                                                                }
                                                                scaleItem.session.ratio = scaleSlider.value
                                                                const index = pipelineModel.index(row, 0);
                                                                pipelineModel.setData(index, scaleItem.session, Qt.WhatsThisRole)
                                                            }
                                                        }
                                                    }

                                                    TapHandler {
                                                        acceptedButtons: Qt.RightButton
                                                        gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                                                        onSingleTapped: {
                                                            pipelineMenu.pipelineIndex = model.row
                                                            pipelineMenu.popup()
                                                        }
                                                    }
                                                }
                                            }

                                            Component {
                                                id: thresholdDelegate

                                                Item {
                                                    id: thresholdItem
                                                    implicitWidth: parent.width; implicitHeight: 80
                                                    property var session: model.whatsThis

                                                    Rectangle {
                                                        anchors.fill: parent
                                                        color: global.back
                                                    }

                                                    ColumnLayout {
                                                        anchors.fill: parent
                                                        anchors.leftMargin: 10

                                                        RowLayout {

                                                            Label {
                                                                leftPadding: 6
                                                                horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                                                                text: model.display || ""
                                                                elide: Text.ElideRight
                                                                Layout.fillWidth: true
                                                            }

                                                            ComboBox {
                                                                id: thresholdComboBox
                                                                currentIndex: session.mode
                                                                model: ListModel {
                                                                    ListElement {
                                                                        text: qsTr("Binary")
                                                                    }
                                                                    ListElement {
                                                                        text: qsTr("Binary Inv")
                                                                    }
                                                                    ListElement {
                                                                        text: qsTr("Trunc")
                                                                    }
                                                                    ListElement {
                                                                        text: qsTr("To Zero")
                                                                    }
                                                                    ListElement {
                                                                        text: qsTr("To Zero Inv")
                                                                    }
                                                                }
                                                                textRole: "text"
                                                                property bool initialized: false

                                                                Component.onCompleted: {
                                                                    initialized = true
                                                                }

                                                                onCurrentTextChanged: {
                                                                    if (!thresholdComboBox.initialized) return
                                                                    thresholdItem.session.mode = thresholdComboBox.currentIndex
                                                                    const index = pipelineModel.index(row, 0);
                                                                    pipelineModel.setData(index, thresholdItem.session, Qt.WhatsThisRole)
                                                                }
                                                            }
                                                        }

                                                        RangeSlider {
                                                            id: thresholdSlider
                                                            first.value: session.thresh
                                                            second.value: session.maxval
                                                            from: 0
                                                            to: 255
                                                            Layout.fillWidth: true
                                                            ToolTip.text: first.value.toFixed(0) + " ~ " + second.value.toFixed(0)
                                                            ToolTip.visible: hovered

                                                            first.onMoved: {
                                                                thresholdItem.session.thresh = Math.round(thresholdSlider.first.value)
                                                                const index = pipelineModel.index(row, 0);
                                                                pipelineModel.setData(index, thresholdItem.session, Qt.WhatsThisRole)
                                                            }

                                                            second.onMoved: {
                                                                thresholdItem.session.maxval = Math.round(thresholdSlider.second.value)
                                                                const index = pipelineModel.index(row, 0);
                                                                pipelineModel.setData(index, thresholdItem.session, Qt.WhatsThisRole)
                                                            }
                                                        }
                                                    }

                                                    TapHandler {
                                                        acceptedButtons: Qt.RightButton
                                                        gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds

                                                        onSingleTapped: {
                                                            pipelineMenu.pipelineIndex = model.row
                                                            pipelineMenu.popup()
                                                        }
                                                    }
                                                }
                                            }
                                        }

                                        Menu {
                                            id: pipelineMenu
                                            property int pipelineIndex

                                            MenuItem {
                                                text: qsTr("Delete")
                                                icon.source: "qrc:/icon/delete.svg"
                                                icon.width: 16; icon.height: 16

                                                onTriggered: portSetting.pipelineRemove(pipelineMenu.pipelineIndex)
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // recognition
                        ColumnLayout {
                            SplitView.fillWidth: true

                            RowLayout {
                                Layout.fillWidth: true

                                Label {
                                    leftPadding: 6
                                    horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                                    font.pixelSize: 20
                                    text: qsTr("Recognition")
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }

                                ComboBox {
                                    id: recognitionComboBox
                                    model: ListModel {
                                        ListElement {
                                            text: qsTr("OCR")
                                        }
                                        ListElement {
                                            text: qsTr("Corner ShiTomasi")
                                        }
                                        ListElement {
                                            text: qsTr("Corner Harris")
                                        }
                                    }
                                    textRole: "text"
                                }
                            }
                        }

                        // spring
                        Item {
                            SplitView.fillWidth: true; SplitView.fillHeight: true
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
                                }
                                // else if (!(rootItem.patterns["ipv4"].test(tcpClientRemoteHostTextField.text)
                                //     || rootItem.patterns["ipv6"].test(tcpClientRemoteHostTextField.text))) {
                                //     portNameValidator.text = qsTr("Invalid Remote Host")
                                //     portNameValidatorTimer.start()
                                //     return
                                // }
                            }
                                break
                            case 3: {
                                if (!sslClientNameTextField.text) {
                                    portNameValidator.text = qsTr("Invalid Port Name")
                                    portNameValidatorTimer.start()
                                    return
                                }
                                if (!sslClientRemoteHostTextField.text) {
                                    portNameValidator.text = qsTr("Empty Remote Host")
                                    portNameValidatorTimer.start()
                                    return
                                }
                                // else if (!(rootItem.patterns["ipv4"].test(sslClientRemoteHostTextField.text)
                                //     || rootItem.patterns["ipv6"].test(sslClientRemoteHostTextField.text))) {
                                //     portNameValidator.text = qsTr("Invalid Remote Host")
                                //     portNameValidatorTimer.start()
                                //     return
                                // }
                            }
                                break
                            case 4: {
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
                            case 5: {
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
                                }
                                // else if (!(rootItem.patterns["ipv4"].test(udpSocketRemoteHostTextField.text)
                                //     || rootItem.patterns["ipv6"].test(udpSocketRemoteHostTextField.text))) {
                                //     portNameValidator.text = qsTr("Invalid Remote Host")
                                //     portNameValidatorTimer.start()
                                //     return
                                // }
                            }
                                break
                            case 6: {
                                portSetting.videoCapture()
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

    // roi
    function roiReload() {
        roiTableLoader.active = false
        roiTableLoader.active = true
    }

    // pipeline
    Menu {
        id: pipelineMenu

        MenuItem {
            text: qsTr("Scale")
            icon.source: "qrc:/icon/resize.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                let session = {}
                session.type = 0
                session.ratio = 0
                session.interpolation = 1
                portSetting.pipelineInsert(session)
            }
        }

        MenuItem {
            text: qsTr("Threshold")
            icon.source: "qrc:/icon/contrast.svg"
            icon.width: 16; icon.height: 16

            onTriggered: {
                let session = {}
                session.type = 1
                session.thresh = 0
                session.maxval = 255
                session.mode = 0
                portSetting.pipelineInsert(session)
            }
        }
    }

    function pipelineReload() {
        pipelineTableLoader.active = false
        pipelineTableLoader.active = true
    }

    // indicator
    Component {
        id: indicatorRectComponent

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

            TapHandler {
                acceptedButtons: Qt.LeftButton

                onTapped: {
                    const pos = mapToItem(previewPopup.parent, width, 0)
                    previewPopup.x = pos.x
                    previewPopup.y = pos.y
                    previewImage.currentIndex = index
                    previewPopup.open()
                }
            }
        }
    }

    Component {
        id: indicatorQuadComponent

        Shape {
            id: indicatorQuadShape
            property int x0
            property int y0
            property int x1
            property int y1
            property int x2
            property int y2
            property int x3
            property int y3
            property int index

            ShapePath {
                strokeColor: "#0078d4"
                strokeWidth: 1
                fillColor: "#80a9d3f2"
                startX: x0; startY: y0

                PathLine {
                    x: x1; y: y1
                }
                PathLine {
                    x: x2; y: y2
                }
                PathLine {
                    x: x3; y: y3
                }
                PathLine {
                    x: x0; y: y0
                }

                // Label {
                //     text: index.toString()
                //     font.bold: true
                //     font.pixelSize: 20
                //     background: Rectangle {
                //         color: "#0078d4"
                //     }
                // }
            }

            Item {
                x: indicatorQuadShape.boundingRect.x
                y: indicatorQuadShape.boundingRect.y
                width: indicatorQuadShape.boundingRect.width
                height: indicatorQuadShape.boundingRect.height

                TapHandler {
                    acceptedButtons: Qt.LeftButton

                    onTapped: {
                        const pos = mapToItem(previewPopup.parent, width, 0)
                        previewPopup.x = pos.x
                        previewPopup.y = pos.y
                        previewImage.currentIndex = index
                        previewPopup.open()
                    }
                }
            }
        }
    }

    function indicatorReload() {
        for (let i = 0; i < videoOutput.indicatorList.length; ++i) {
            videoOutput.indicatorList[i].destroy()
        }
        videoOutput.indicatorList = []
        for (let row = 0; row < roiModel.rowCount(); ++row) {
            const index = roiModel.index(row, 0)
            const roi = roiModel.data(index, Qt.WhatsThisRole)
            var indicator
            if (roi.length === 4) {
                // rect roi
                indicator = indicatorRectComponent.createObject(videoOutput, {
                    x: roi[0],
                    y: roi[1],
                    width: roi[2],
                    height: roi[3],
                    index: row
                });
            } else if (roi.length === 8) {
                // quad roi
                indicator = indicatorQuadComponent.createObject(videoOutput, {
                    x0: roi[0],
                    y0: roi[1],
                    x1: roi[2],
                    y1: roi[3],
                    x2: roi[4],
                    y2: roi[5],
                    x3: roi[6],
                    y3: roi[7],
                    index: row
                });
            }
            videoOutput.indicatorList.push(indicator)
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
            // ssl client
            "sslClientNameTextField": sslClientNameTextField,
            "sslClientRemoteHostTextField": sslClientRemoteHostTextField,
            "sslClientRemotePortSpinBox": sslClientRemotePortSpinBox,
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
            // video stream
            "videoStreamNameComboBox": videoStreamNameComboBox,
            // format
            "txFormatComboBox": txFormatComboBox,
            "txSuffixComboBox": txSuffixComboBox,
            "rxFormatComboBox": rxFormatComboBox,
            "bufferSizeSpinBox": bufferSizeSpinBox,
            // image
            "videoSink": videoOutput.videoSink,
            "previewImage": previewImage,
            "recognitionComboBox": recognitionComboBox
        };
        portSetting.propertyGet(objects)
    }
}
