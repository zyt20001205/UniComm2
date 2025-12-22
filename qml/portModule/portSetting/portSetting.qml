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
                }
            }

            ColumnLayout {
                Layout.fillWidth: true; Layout.fillHeight: true

                StackLayout {
                    currentIndex: rootItem.portType
                    Layout.fillWidth: true; Layout.fillHeight: true

                    Loader {
                        sourceComponent: serialPortPage
                        Layout.fillWidth: true; Layout.fillHeight: true
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true; Layout.fillHeight: true
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

            PageIndicator {
                id: pageIndicator
                count: swipeView.count
                currentIndex: swipeView.currentIndex
            }

            Button {
                text: swipeView.currentIndex !== 2 ? qsTr("Next") : qsTr("Confirm")
                highlighted: swipeView.currentIndex === 2

                onClicked: {
                    if (swipeView.currentIndex !== 2) {
                        swipeView.currentIndex = swipeView.currentIndex + 1
                    }
                }
            }
        }
    }

    Component {
        id: serialPortPage

        GridLayout {
            anchors.fill: parent
            columns: 2; rows: 2
            columnSpacing: 10; rowSpacing: 10

            Label {
                text: qsTr("Port Name")
                font.pointSize: 12
            }
        }
    }
}
