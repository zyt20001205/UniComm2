#include "scriptModule/codeAssistant/codeAssistant.h"

#include <QKeyEvent>

#include "scriptModule/codeAssistant/completionWidget.h"
#include "scriptModule/codeAssistant/dwellWidget.h"

// CodeAssistant public
CodeAssistant::CodeAssistant(QWidget *parent)
    : QObject(parent),
      m_completionWidget(new CompletionWidget(parent)),
      m_dwellWidget(new DwellWidget(parent)) {
    connect(m_completionWidget, &CompletionWidget::setCursorPosition, this, &CodeAssistant::setCursorPosition);
    connect(m_completionWidget, &CompletionWidget::replaceText, this, &CodeAssistant::replaceText);
    connect(m_completionWidget, &CompletionWidget::addChar, this, &CodeAssistant::addChar);
    connect(m_completionWidget, &CompletionWidget::insertPort, this, &CodeAssistant::insertPort);
    connect(m_completionWidget, &CompletionWidget::insertDatabase, this, &CodeAssistant::insertDatabase);
    connect(m_completionWidget, &CompletionWidget::insertDatatable, this, &CodeAssistant::insertDatatable);
    connect(m_dwellWidget, &DwellWidget::replaceText, this, &CodeAssistant::replaceText);
}

void CodeAssistant::completionShow(const QVariantMap &completionSession, const QJsonArray &items) const {
    m_completionWidget->completionShow(completionSession, items);
}

void CodeAssistant::completionHide() const {
    m_completionWidget->completionHide();
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

// CodeAssistant protected
bool CodeAssistant::eventFilter(QObject *obj, QEvent *event) {
    if (m_completionWidget->isVisible()) {
        if (event->type() == QEvent::KeyPress) {
            const auto *keyEvent = static_cast<QKeyEvent *>(event);
            switch (keyEvent->key()) {
                case Qt::Key_Tab: {
                    m_completionWidget->textReplace();
                    m_completionWidget->completionHide();
                }
                    return true;
                case Qt::Key_Up: {
                    m_completionWidget->completionPrev();
                }
                    return true;
                case Qt::Key_Down: {
                    m_completionWidget->completionNext();
                }
                    return true;
                case Qt::Key_Return:
                case Qt::Key_Escape:
                case Qt::Key_Backspace:
                case Qt::Key_Left:
                case Qt::Key_Right: {
                    m_completionWidget->completionHide();
                }
                    return false;
                default:
                    return false;
            }
        }
    }

    return QObject::eventFilter(obj, event);
}
