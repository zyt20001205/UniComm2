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
            text: workspaceName
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
                        Layout.preferredWidth: breadcrumbButtonTextMetrics.width + 8; Layout.preferredHeight: 20

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

        Button {
            id: positionButton
            flat: true
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            Layout.preferredWidth: positionButtonTextMetrics.width + 8; Layout.preferredHeight: 20

            // onClicked:

            TextMetrics {
                id: positionButtonTextMetrics
                text: positionButton.text
                font: positionButton.font
            }
        }

        Button {
            id: threadButton
            flat: true
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            text: qsTr("Idle")
            Layout.preferredWidth: threadButtonTextMetrics.width + 8; Layout.preferredHeight: 20

            // onClicked:

            TextMetrics {
                id: threadButtonTextMetrics
                text: threadButton.text
                font: threadButton.font
            }
        }
    }

    Component.onCompleted: {
        const objects = {
            "positionButton": positionButton,
            "threadButton": threadButton
        };
        statusModule.propertyGet(objects)
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