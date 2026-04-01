import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 6; anchors.rightMargin: 6; anchors.bottomMargin: 6

        Item {
            Layout.preferredWidth: 600; Layout.preferredHeight: 24

            TextField {
                id: searchTextField
                anchors.fill: parent
                rightPadding: 120

                onTextChanged: searchWidget.searchRequest()
            }

            RowLayout {
                anchors.right: parent.right
                spacing: 0

                Button {
                    Layout.preferredWidth: 24; Layout.preferredHeight: 24
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    flat: true
                    icon.source: "qrc:/icon/close.svg"
                    icon.width: 12; icon.height: 12

                    onClicked: searchTextField.clear()
                }

                Button {
                    id: matchCaseButton
                    Layout.preferredWidth: 24; Layout.preferredHeight: 24
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    checkable: true
                    flat: true
                    icon.source: "qrc:/icon/matchCase.svg"
                    icon.width: 16; icon.height: 16

                    onClicked: {
                        searchWidget.searchFlagsSet(matchCaseButton.checked, wholeWordButton.checked, wordStartButton.checked, regExpButton.checked)
                        searchWidget.searchRequest()
                    }

                    HoverHandler {
                        onHoveredChanged: {
                            if (!hovered) {
                                mainTooltip.text = ""
                            }
                        }
                        onPointChanged: {
                            mainTooltip.position = parent.mapToGlobal(point.position)
                            mainTooltip.text = parent.checked ? qsTr("Disable Match Case") : qsTr("Enable Match Case")
                        }
                    }
                }

                Button {
                    id: wholeWordButton
                    Layout.preferredWidth: 24; Layout.preferredHeight: 24
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    checkable: true
                    flat: true
                    icon.source: "qrc:/icon/wholeWord.svg"
                    icon.width: 16; icon.height: 16

                    onClicked: {
                        searchWidget.searchFlagsSet(matchCaseButton.checked, wholeWordButton.checked, wordStartButton.checked, regExpButton.checked)
                        searchWidget.searchRequest(searchTextField.text)
                    }

                    HoverHandler {
                        onHoveredChanged: {
                            if (!hovered) {
                                mainTooltip.text = ""
                            }
                        }
                        onPointChanged: {
                            mainTooltip.position = parent.mapToGlobal(point.position)
                            mainTooltip.text = parent.checked ? qsTr("Disable Whole Word") : qsTr("Enable Whole Word")
                        }
                    }
                }

                Button {
                    id: wordStartButton
                    Layout.preferredWidth: 24; Layout.preferredHeight: 24
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    checkable: true
                    flat: true
                    icon.source: "qrc:/icon/wordStart.svg"
                    icon.width: 16; icon.height: 16

                    onClicked: {
                        searchWidget.searchFlagsSet(matchCaseButton.checked, wholeWordButton.checked, wordStartButton.checked, regExpButton.checked)
                        searchWidget.searchRequest(searchTextField.text)
                    }

                    HoverHandler {
                        onHoveredChanged: {
                            if (!hovered) {
                                mainTooltip.text = ""
                            }
                        }
                        onPointChanged: {
                            mainTooltip.position = parent.mapToGlobal(point.position)
                            mainTooltip.text = parent.checked ? qsTr("Disable Word Start") : qsTr("Enable Word Start")
                        }
                    }
                }

                Button {
                    id: regExpButton
                    Layout.preferredWidth: 24; Layout.preferredHeight: 24
                    leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                    checkable: true
                    flat: true
                    icon.source: "qrc:/icon/regExp.svg"
                    icon.width: 16; icon.height: 16

                    onClicked: {
                        searchWidget.searchFlagsSet(matchCaseButton.checked, wholeWordButton.checked, wordStartButton.checked, regExpButton.checked)
                        searchWidget.searchRequest(searchTextField.text)
                    }

                    HoverHandler {
                        onHoveredChanged: {
                            if (!hovered) {
                                mainTooltip.text = ""
                            }
                        }
                        onPointChanged: {
                            mainTooltip.position = parent.mapToGlobal(point.position)
                            mainTooltip.text = parent.checked ? qsTr("Disable Regular Expression") : qsTr("Enable Regular Expression")
                        }
                    }
                }
            }
        }

        Button {
            id: prevButton
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            enabled: false
            flat: true
            icon.source: "qrc:/icon/arrowUp.svg"
            icon.width: 16; icon.height: 16

            onClicked: searchWidget.searchPrev()

            HoverHandler {
                onHoveredChanged: {
                    if (!hovered) {
                        mainTooltip.text = ""
                    }
                }
                onPointChanged: {
                    mainTooltip.position = parent.mapToGlobal(point.position)
                    mainTooltip.text = qsTr("Search Previous")
                }
            }
        }

        Button {
            id: nextButton
            Layout.preferredWidth: 24; Layout.preferredHeight: 24
            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
            enabled: false
            flat: true
            icon.source: "qrc:/icon/arrowDown.svg"
            icon.width: 16; icon.height: 16

            onClicked: searchWidget.searchNext()

            HoverHandler {
                onHoveredChanged: {
                    if (!hovered) {
                        mainTooltip.text = ""
                    }
                }
                onPointChanged: {
                    mainTooltip.position = parent.mapToGlobal(point.position)
                    mainTooltip.text = qsTr("Search Next")
                }
            }
        }

        Label {
            id: searchLabel
            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            text: "0/0"
            Layout.preferredWidth: 80; Layout.preferredHeight: 24
        }

        Item {
            Layout.fillWidth: true
        }
    }

    Component.onCompleted: {
        const objects = {
            "searchTextField": searchTextField,
            "prevButton": prevButton,
            "nextButton": nextButton,
            "searchLabel": searchLabel
        };
        searchWidget.propertyGet(objects)
    }
}