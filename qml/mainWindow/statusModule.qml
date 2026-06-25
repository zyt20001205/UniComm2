import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts

Item {
    id: rootItem
    height: 24
    property bool modelVisible: backgroundModel.rowCount() > 0

    Rectangle {
        anchors.fill: parent
        color: global.back
    }

    RowLayout {
        anchors.fill: parent

        Button {
            flat: true
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            icon.source: "qrc:/icon/home.svg"
            icon.width: 16; icon.height: 16
            text: workspaceName
            Layout.preferredHeight: 20
        }

        RowLayout {
            id: pathBreadcrumb

            Component {
                id: breadcrumbComponent

                RowLayout {
                    id: breadcrumbItem
                    Layout.alignment: Qt.AlignVCenter
                    property string text

                    IconImage {
                        color: global.fore
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

        RowLayout {
            visible: modelVisible

            Button {
                text: backgroundModel.title
                flat: true
                Layout.preferredHeight: 24

                onClicked: statusModule.backgroundInfo(taskId)
            }

            ProgressBar {
                indeterminate: true
                Layout.preferredWidth: 160
            }

            Button {
                visible: taskId !== -1
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                flat: true
                icon.source: "qrc:/icon/dismiss.svg"
                icon.width: 12; icon.height: 12
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                property int taskId: backgroundModel.taskId

                onClicked: statusModule.backgroundAbort(taskId)
            }
        }

        Button {
            id: positionButton
            flat: true
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            Layout.preferredWidth: positionButtonTextMetrics.width + 8; Layout.preferredHeight: 20

            onClicked: statusModule.documentGoto(documentUrl)

            TextMetrics {
                id: positionButtonTextMetrics
                text: positionButton.text
                font: positionButton.font
            }
        }

        Button {
            id: eolModeButton
            flat: true
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            Layout.preferredWidth: eolModeButtonTextMetrics.width + 8; Layout.preferredHeight: 20

            onClicked: {
                eolModeMenu.eolModeButton = eolModeButton
                eolModeMenu.documentUrl = documentUrl
                const globalPos = eolModeButton.mapToGlobal(0, -eolModeMenu.height);
                const localPos = eolModeMenu.parent.mapFromGlobal(globalPos.x, globalPos.y);
                eolModeMenu.popup(localPos.x, localPos.y)
            }

            TextMetrics {
                id: eolModeButtonTextMetrics
                text: eolModeButton.text
                font: eolModeButton.font
            }
        }

        Button {
            id: codePageButton
            flat: true
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            Layout.preferredWidth: codePageButtonTextMetrics.width + 8; Layout.preferredHeight: 20

            // onClicked:

            TextMetrics {
                id: codePageButtonTextMetrics
                text: codePageButton.text
                font: codePageButton.font
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

    Connections {
        target: backgroundModel

        function onRowsInserted() {
            modelVisible = true
        }

        function onRowsRemoved() {
            modelVisible = backgroundModel.rowCount() > 0
        }

        function onModelReset() {
            modelVisible = false
        }
    }

    function documentPathLoad(pathList) {
        pathBreadcrumb.children = [];
        for (const path of pathList) {
            const item = breadcrumbComponent.createObject(pathBreadcrumb, {
                "text": path
            });
        }
    }

    Component.onCompleted: {
        const objects = {
            "positionButton": positionButton,
            "eolModeButton": eolModeButton,
            "codePageButton": codePageButton,
            "threadButton": threadButton
        };
        statusModule.propertyGet(objects)
    }
}