#include "portModule/pixmapPreview.h"

#include <QLabel>
#include <QVBoxLayout>

// public
PixmapPreview::PixmapPreview(QWidget *parent)
    : QDialog(parent),
      m_layout(new QVBoxLayout(this)) {
}

void PixmapPreview::previewShow(const QList<QPixmap> &pixmapList) const {
    previewClear();
    foreach(QPixmap pixmap, pixmapList) {
        auto *label = new QLabel(); // NOLINT
        m_layout->addWidget(label);
        label->setPixmap(pixmap);
    }
}

// private
void PixmapPreview::previewClear() const {
    QLayoutItem *item;
    while ((item = m_layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}
