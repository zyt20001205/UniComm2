import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true; Layout.preferredHeight: 24
            Layout.alignment: Qt.AlignTop

            Button {
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                icon.source: "qrc:/icon/stop.svg"
                icon.width: 16; icon.height: 16
                ToolTip.text: qsTr("Terminate")
                ToolTip.visible: hovered
                onClicked: debugModule.stateSet(combobox.currentText, 0)
            }

            Button {
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                icon.source: "qrc:/icon/play.svg"
                icon.width: 16; icon.height: 16
                ToolTip.text: qsTr("Resume")
                ToolTip.visible: hovered
                onClicked: debugModule.stateSet(combobox.currentText, 1)
            }

            Button {
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                icon.source: "qrc:/icon/pause.svg"
                icon.width: 16; icon.height: 16
                ToolTip.text: qsTr("Pause")
                ToolTip.visible: hovered
                // onClicked: logModule.timestampToggle(checked)
            }

            Button {
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                icon.source: "qrc:/icon/debugStepOver.svg"
                icon.width: 16; icon.height: 16
                ToolTip.text: qsTr("Step Over")
                ToolTip.visible: hovered
                // onClicked: logModule.timestampToggle(checked)
            }

            Button {
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                icon.source: "qrc:/icon/debugStepInto.svg"
                icon.width: 16; icon.height: 16
                ToolTip.text: qsTr("Step Into")
                ToolTip.visible: hovered
                // onClicked: logModule.timestampToggle(checked)
            }

            Button {
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                icon.source: "qrc:/icon/debugStepOut.svg"
                icon.width: 16; icon.height: 16
                ToolTip.text: qsTr("Step Out")
                ToolTip.visible: hovered
                // onClicked: logModule.timestampToggle(checked)
            }

            Button {
                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                icon.source: "qrc:/icon/debugContinue.svg"
                icon.width: 16; icon.height: 16
                ToolTip.text: qsTr("Run To Cursor")
                ToolTip.visible: hovered
                // onClicked: logModule.timestampToggle(checked)
            }
        }

        ComboBox {
            id: combobox
            Layout.fillWidth: true;
            Layout.alignment: Qt.AlignTop
            model: stringListModel
            textRole: "display"
        }
    }

    Connections {
        target: stringListModel

        function onRowsInserted() {
            combobox.currentIndex = stringListModel.rowCount() - 1
        }

        function onRowsRemoved() {
            combobox.currentIndex = stringListModel.rowCount() - 1
        }
    }
}