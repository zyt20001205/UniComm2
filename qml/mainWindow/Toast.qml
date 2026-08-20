pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts

Item {
    id: root
    width: 360
    height: activeToasts.count > 0 ? Math.max(1, toastColumn.implicitHeight) : 0

    readonly property int errorLevel: 0
    readonly property int warningLevel: 1
    readonly property int infoLevel: 2
    readonly property int successLevel: 3
    readonly property int maximumVisible: 5
    property int nextId

    ListModel {
        id: activeToasts
    }

    Column {
        id: toastColumn
        width: parent.width
        spacing: 8

        Repeater {
            model: activeToasts

            delegate: Item {
                id: toastDelegate
                required property int toastId
                required property int toastLevel
                required property string toastTitle
                required property string toastText
                required property int toastDuration
                width: toastColumn.width
                height: toastPopup.implicitHeight
                property real remaining: 1
                property real popupOpacity: 0
                property real popupOffset: 24
                property bool closing: false
                readonly property url iconSource: toastLevel === root.errorLevel ? "qrc:/icon/error.svg"
                                                  : toastLevel === root.warningLevel ? "qrc:/icon/warning.svg"
                                                  : toastLevel === root.infoLevel ? "qrc:/icon/info.svg"
                                                  : "qrc:/icon/checkmarkCircle.svg"
                readonly property color levelColor: toastLevel === root.errorLevel ? global.dangerBack3
                                                    : toastLevel === root.warningLevel ? global.warningBack3
                                                    : toastLevel === root.infoLevel ? global.brandBack
                                                    : global.successBack3

                Behavior on y {
                    NumberAnimation {
                        duration: 160
                        easing.type: Easing.OutCubic
                    }
                }

                ToolTip {
                    id: toastPopup
                    parent: toastDelegate
                    x: toastDelegate.popupOffset
                    y: 0
                    width: toastDelegate.width
                    padding: 0
                    closePolicy: Popup.NoAutoClose
                    delay: 0
                    timeout: -1
                    visible: true
                    opacity: toastDelegate.popupOpacity
                    enter: null
                    exit: null

                    contentItem: ColumnLayout {
                        id: toastContent
                        spacing: 0

                        RowLayout {
                            spacing: 8
                            Layout.fillWidth: true
                            Layout.leftMargin: 10
                            Layout.rightMargin: 10
                            Layout.topMargin: 10
                            Layout.bottomMargin: 8

                            IconImage {
                                source: toastDelegate.iconSource
                                color: toastDelegate.levelColor
                                fillMode: Image.PreserveAspectFit
                                sourceSize.width: 20; sourceSize.height: 20
                                Layout.preferredWidth: 20; Layout.preferredHeight: 20
                                Layout.alignment: Qt.AlignTop
                            }

                            ColumnLayout {
                                spacing: 2
                                Layout.fillWidth: true

                                Label {
                                    text: toastDelegate.toastTitle
                                    color: global.fore
                                    font.bold: true
                                    textFormat: Text.PlainText
                                    wrapMode: Text.Wrap
                                    visible: toastDelegate.toastTitle.length > 0
                                    Layout.fillWidth: true
                                }

                                Label {
                                    text: toastDelegate.toastText
                                    color: global.fore
                                    textFormat: Text.PlainText
                                    wrapMode: Text.Wrap
                                    visible: toastDelegate.toastText.length > 0
                                    Layout.fillWidth: true
                                }
                            }

                            Button {
                                flat: true
                                focusPolicy: Qt.NoFocus
                                leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                icon.source: "qrc:/icon/dismiss.svg"
                                icon.width: 12; icon.height: 12
                                Layout.preferredWidth: 24; Layout.preferredHeight: 24
                                Layout.alignment: Qt.AlignTop

                                onClicked: toastDelegate.beginDismiss()
                            }
                        }

                        ProgressBar {
                            from: 0
                            to: 1
                            value: toastDelegate.remaining
                            palette.accent: toastDelegate.levelColor
                            Layout.fillWidth: true
                            Layout.leftMargin: 2; Layout.rightMargin: 2
                        }
                    }
                }

                ParallelAnimation {
                    id: enterAnimation

                    NumberAnimation {
                        target: toastDelegate
                        property: "popupOpacity"
                        from: 0
                        to: 1
                        duration: 160
                        easing.type: Easing.OutCubic
                    }

                    NumberAnimation {
                        target: toastDelegate
                        property: "popupOffset"
                        from: 24
                        to: 0
                        duration: 160
                        easing.type: Easing.OutCubic
                    }
                }

                NumberAnimation {
                    id: progressAnimation
                    target: toastDelegate
                    property: "remaining"
                    from: 1
                    to: 0
                    duration: toastDelegate.toastDuration

                    onFinished: toastDelegate.beginDismiss()
                }

                ParallelAnimation {
                    id: exitAnimation

                    NumberAnimation {
                        target: toastDelegate
                        property: "popupOpacity"
                        to: 0
                        duration: 140
                        easing.type: Easing.InCubic
                    }

                    NumberAnimation {
                        target: toastDelegate
                        property: "popupOffset"
                        to: 24
                        duration: 140
                        easing.type: Easing.InCubic
                    }

                    onFinished: root.dismiss(toastDelegate.toastId)
                }

                function beginDismiss(): void {
                    if (closing) return
                    closing = true
                    progressAnimation.stop()
                    exitAnimation.start()
                }

                Component.onCompleted: {
                    enterAnimation.start()
                    progressAnimation.start()
                }
            }
        }
    }

    function show(level: int, title: string, text: string, displayTime: int): void {
        const normalizedLevel = level >= errorLevel && level <= successLevel ? level : infoLevel
        const duration = displayTime > 0 ? displayTime : 5000
        const toast = {
            "toastId": ++nextId,
            "toastLevel": normalizedLevel,
            "toastTitle": title === undefined || title === null ? "" : String(title),
            "toastText": text === undefined || text === null ? "" : String(text),
            "toastDuration": duration
        }
        if (activeToasts.count >= maximumVisible) activeToasts.remove(0)
        activeToasts.append(toast)
    }

    function dismiss(toastId) {
        for (let index = 0; index < activeToasts.count; ++index) {
            if (activeToasts.get(index).toastId === toastId) {
                activeToasts.remove(index)
                break
            }
        }
    }
}
