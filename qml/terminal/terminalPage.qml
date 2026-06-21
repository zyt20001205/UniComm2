import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: rootItem
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: global.back
    }

    FontMetrics {
        id: terminalFontMetrics
        font: textArea.font
    }

    ScrollView {
        id: scrollView
        anchors.fill: parent

        ScrollBar.vertical: ScrollBar {
            x: parent.mirrored ? 0 : parent.width - width
            y: parent.topPadding
            height: parent.availableHeight
            active: parent.ScrollBar.horizontal.active
            policy: ScrollBar.AsNeeded
            palette {
                mid: global.stroke
                dark: global.strokePressed
            }
        }

        ScrollBar.horizontal: ScrollBar {
            x: parent.leftPadding
            y: parent.height - height
            width: parent.availableWidth
            active: parent.ScrollBar.vertical.active
            policy: ScrollBar.AsNeeded
            palette {
                mid: global.stroke
                dark: global.strokePressed
            }
        }

        TextArea {
            id: textArea
            textFormat: TextEdit.PlainText
            verticalAlignment: TextEdit.AlignTop
            wrapMode: TextEdit.NoWrap
            onWidthChanged: rootItem.terminalResize()
            onHeightChanged: rootItem.terminalResize()
            onFontChanged: rootItem.terminalResize()

            Keys.onPressed: (event) => {
                event.accepted = terminalPage.terminalInput(event.key, event.modifiers, event.text)
            }
        }
    }

    function terminalResize() {
        const charWidth = Math.max(1, terminalFontMetrics.advanceWidth("M"))
        const lineHeight = Math.max(1, terminalFontMetrics.lineSpacing)
        const availableWidth = Math.max(1, textArea.width - textArea.leftPadding - textArea.rightPadding)
        const availableHeight = Math.max(1, textArea.height - textArea.topPadding - textArea.bottomPadding)
        terminalPage.terminalResize(
            Math.max(1, Math.floor(availableHeight / lineHeight)),
            Math.max(1, Math.floor(availableWidth / charWidth))
        )
    }
    
    Component.onCompleted: {
        const objects = {
            "textArea": textArea
        };
        terminalPage.propertyGet(objects)
        terminalResize()
        textArea.forceActiveFocus()
    }
}
