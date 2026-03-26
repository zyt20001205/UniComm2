import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    height: 24

    RowLayout {
        anchors.fill: parent

        Button {
            flat: true
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            icon.source: "qrc:/icon/document.svg"
            icon.width: 16; icon.height: 16
            Layout.preferredWidth: 20; Layout.preferredHeight: 20

            // onClicked:
        }

        RowLayout {
            id: symbolBreadcrumb

            Component {
                id: breadcrumbComponent

                RowLayout {
                    id: breadcrumbItem
                    Layout.alignment: Qt.AlignVCenter
                    property string text
                    property string source

                    Image {
                        source: "qrc:/icon/arrowCollapse.svg"
                        sourceSize.width: 16
                        sourceSize.height: 16
                        Layout.preferredHeight: 16
                    }

                    Button {
                        id: breadcrumbButton
                        flat: true
                        leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                        icon.source: breadcrumbItem.source
                        icon.width: 16; icon.height: 16
                        text: breadcrumbItem.text
                        Layout.preferredWidth: breadcrumbButtonTextMetrics.width + 28; Layout.preferredHeight: 20

                        // onClicked:

                        TextMetrics {
                            id: breadcrumbButtonTextMetrics
                            text: breadcrumbButton.text
                            font: breadcrumbButton.font
                        }
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }

    function symbolLoad(symbolList) {
        symbolBreadcrumb.children = [];
        for (const symbol of symbolList) {
            const item = breadcrumbComponent.createObject(symbolBreadcrumb, {
                "text": symbol.text,
                "source": symbol.source
            });
        }
    }
}