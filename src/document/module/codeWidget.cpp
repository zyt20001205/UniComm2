#include "document/module/codeWidget.h"

#include <QVBoxLayout>

#include "analysis/symbolWidget.h"

// public
CodeWidget::CodeWidget(const QJsonObject& documentConfig, const QUrl& documentUrl, QWidget* parent)
    : QWidget(parent),
      m_editorWidget(new EditorWidget(documentConfig, documentUrl, this)),
      m_symbolWidget(new SymbolWidget(this)){
    auto *layout = new QVBoxLayout(this); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_editorWidget);
    layout->addWidget(m_symbolWidget);
}

void CodeWidget::propertySet(const QVariantHash& objects) {
}
