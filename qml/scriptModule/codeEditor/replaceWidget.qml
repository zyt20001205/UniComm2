import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 6; anchors.rightMargin: 6; anchors.bottomMargin: 6

        TextField {
            id: replaceTextField
            Layout.preferredWidth: 600; Layout.preferredHeight: 24
        }

        Button {
            id: replaceButton
            Layout.preferredWidth: replaceButtonTextMetrics.width + 8; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            text: qsTr("Replace")

            onClicked: replaceWidget.textReplace(replaceTextField.text)

            TextMetrics {
                id: replaceButtonTextMetrics
                text: replaceButton.text
                font: replaceButton.font
            }
        }

        Button {
            id: replaceAllButton
            Layout.preferredWidth: replaceAllButtonTextMetrics.width + 8; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            highlighted: true
            text: qsTr("Replace All")

            onClicked: replaceWidget.textReplaceAll(replaceTextField.text)

            TextMetrics {
                id: replaceAllButtonTextMetrics
                text: replaceAllButton.text
                font: replaceAllButton.font
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }
}