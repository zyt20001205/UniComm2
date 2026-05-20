import QtQuick
import QtQuick.Pdf

Item {
    id: rootItem
    anchors.fill: parent

    PdfMultiPageView {
        document: PdfDocument {
            id: pdf
        }
    }

    Component.onCompleted: {
        const objects = {
            "pdf": pdf,
        };
        imagePage.propertyGet(objects)
    }
}