import QtQuick
import QtQuick.Controls

ScrollView {
    TextArea {
        text: crashReport
        readOnly: true
        selectByMouse: true
        wrapMode: TextEdit.NoWrap
        font.family: "Consolas"
        background: null
    }
}
