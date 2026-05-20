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
    }

    Component.onCompleted: {
        const objects = {
            "pdf": pdf,
        };
        pdfPage.propertyGet(objects)
    }
}