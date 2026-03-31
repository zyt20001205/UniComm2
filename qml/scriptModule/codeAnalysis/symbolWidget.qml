import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem

    RowLayout {
        anchors.fill: parent

        Button {
            flat: true
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            icon.source: "qrc:/icon/document.svg"
            icon.width: 16; icon.height: 16
            Layout.preferredWidth: 20; Layout.preferredHeight: 20
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
                    property string detail
                    property var position

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

                        onClicked: symbolWidget.indicatorFill(breadcrumbItem.position)

                        HoverHandler {
                            onHoveredChanged: {
                                if (!hovered) {
                                    mainTooltip.text = ""
                                }
                            }
                            onPointChanged: {
                                mainTooltip.position = parent.mapToGlobal(point.position)
                                mainTooltip.text = breadcrumbItem.detail
                            }
                        }

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
                "source": symbol.source,
                "detail": symbol.detail,
                "position": symbol.position
            });
        }
    }
}