import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
    Layout.fillWidth: true; Layout.fillHeight: true
    spacing: 0

    ColumnLayout {
        Layout.preferredWidth: 24; Layout.fillHeight: true
        Layout.alignment: Qt.AlignTop
        Layout.margins: 4

        Button {
            id: timestampButton
            objectName: "timestampButton"
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            checkable: true
            icon.source: "qrc:/icon/clock.svg"
            icon.width: 16; icon.height: 16

            onClicked: {
                logModule.timestampToggle(checked)
            }
        }

        Button {
            id: heightButton
            objectName: "heightButton"
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            icon.source: "qrc:/icon/autoFitHeight.svg"
            icon.width: 16; icon.height: 16
        }

        Button {
            id: saveButton
            objectName: "saveButton"
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            icon.source: "qrc:/icon/save.svg"
            icon.width: 16; icon.height: 16

            onClicked: {
                logModule.logSave()
            }
        }

        Button {
            id: clearButton
            objectName: "clearButton"
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            icon.source: "qrc:/icon/delete.svg"
            icon.width: 16; icon.height: 16

            onClicked: {
                logTextArea.clear()
            }
        }
    }

    ScrollView {
        id: view
        Layout.fillWidth: true; Layout.fillHeight: true
        Layout.topMargin: 4; Layout.rightMargin: 4; Layout.bottomMargin: 4

        TextArea {
            id: logTextArea
            objectName: "logTextArea"
            textFormat: TextEdit.RichText
            verticalAlignment: TextEdit.AlignTop
        }
    }
}