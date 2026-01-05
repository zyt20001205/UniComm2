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
            icon.source: "qrc:/icon/home.svg"
            icon.width: 16; icon.height: 16
            text: qsTr("Workspace")
            Layout.preferredHeight: 20

            // onClicked:
        }

        RowLayout {
            id: pathBreadcrumb

            Component {
                id: breadcrumbComponent

                RowLayout {
                    id: breadcrumbItem
                    Layout.alignment: Qt.AlignVCenter
                    property string text

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
                        text: breadcrumbItem.text
                        Layout.preferredWidth: textMetrics.width + 8; Layout.preferredHeight: 20

                        // onClicked:

                        TextMetrics {
                            id: textMetrics
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

    function scriptPathLoad(pathList) {
        pathBreadcrumb.children = [];
        for (const path of pathList) {
            const item = breadcrumbComponent.createObject(pathBreadcrumb, {
                "text": path
            });
        }
    }
}