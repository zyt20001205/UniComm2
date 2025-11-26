#include "scriptModule/codeAssistant/codeAssistant.h"

#include "scriptModule/codeAssistant/dwellWidget.h"

// CodeAssistant Public
CodeAssistant::CodeAssistant(QWidget *parent)
    : QObject(parent),
      m_dwellWidget(new DwellWidget(parent)) {
    connect(m_dwellWidget, &DwellWidget::replaceText, this, &CodeAssistant::replaceText);
}

void CodeAssistant::dwellShowDiagnostic(const QUrl &scriptUrl, const QString &message) const {
    m_dwellWidget->dwellShowDiagnostic(scriptUrl, message);
}

void CodeAssistant::dwellShowHover(const QString &message) const {
    m_dwellWidget->dwellShowHover(message);
}

void CodeAssistant::dwellHide() const {
    m_dwellWidget->dwellHide();
}

void CodeAssistant::dwellLeave() const {
    m_dwellWidget->dwellLeave();
}
