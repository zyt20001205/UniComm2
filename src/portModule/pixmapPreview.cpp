#include "portModule/pixmapPreview.h"

#include <QLabel>
#include <QVBoxLayout>

// PixmapPreview public
PixmapPreview::PixmapPreview(QWidget *parent)
    : QDialog(parent),
      m_layout(new QVBoxLayout(this)) {
}

void PixmapPreview::previewShow(const QList<QPixmap> &pixmapList) {
    previewClear();
    foreach(QPixmap pixmap, pixmapList) {
        auto *label = new QLabel(); // NOLINT
        m_layout->addWidget(label);
        label->setPixmap(pixmap);
    }
    this->show();
}

// PixmapPreview private
void PixmapPreview::previewClear() const {
    QLayoutItem *item;
    while ((item = m_layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}
