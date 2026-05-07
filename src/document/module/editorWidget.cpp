#include "document/module/editorWidget.h"

#include <QVBoxLayout>

// public
EditorWidget::EditorWidget(const QVariantHash &session, QWidget *parent)
    : QWidget(parent) {
    auto *layout = new QVBoxLayout(this); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
}