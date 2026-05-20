import QtQuick
import QtQuick.Pdf

Item {
    id: rootItem
    anchors.fill: parent

    PdfMultiPageView {
        anchors.fill: parent
        document: PdfDocument {
            id: pdf
        }

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
            palette {
                mid: global.stroke
                dark: global.strokePressed
            }
        }
    }

    Component.onCompleted: {
        const objects = {
            "pdf": pdf,
        };
        pdfPage.propertyGet(objects)
    }
}