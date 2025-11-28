#include "scriptModule/codeAssistant/codeAssistant.h"

#include <QKeyEvent>

#include "scriptModule/codeAssistant/completionWidget.h"
#include "scriptModule/codeAssistant/dwellWidget.h"
#include "scriptModule/codeAssistant/gotoWidget.h"
#include "scriptModule/codeAssistant/positionWidget.h"
#include "scriptModule/codeAssistant/signatureWidget.h"

// CodeAssistant public
CodeAssistant::CodeAssistant(QWidget *parent)
    : QObject(parent),
      m_completionWidget(new CompletionWidget(parent)),
      m_dwellWidget(new DwellWidget(parent)),
      m_gotoWidget(new GotoWidget(parent)),
      m_positionWidget(new PositionWidget(parent)),
      m_signatureWidget(new SignatureWidget(parent)) {
    connect(m_completionWidget, &CompletionWidget::setCursorPosition, this, &CodeAssistant::setCursorPosition);
    connect(m_completionWidget, &CompletionWidget::replaceText, this, &CodeAssistant::replaceText);
    connect(m_completionWidget, &CompletionWidget::addChar, this, &CodeAssistant::addChar);
    connect(m_completionWidget, &CompletionWidget::insertPort, this, &CodeAssistant::insertPort);
    connect(m_completionWidget, &CompletionWidget::insertDatabase, this, &CodeAssistant::insertDatabase);
    connect(m_completionWidget, &CompletionWidget::insertDatatable, this, &CodeAssistant::insertDatatable);
    connect(m_completionWidget, &CompletionWidget::showPosition, m_positionWidget, &PositionWidget::positionShow);
    connect(m_dwellWidget, &DwellWidget::replaceText, this, &CodeAssistant::replaceText);
    connect(m_gotoWidget, &GotoWidget::setCursorPosition, this, &CodeAssistant::setCursorPosition);
    connect(m_gotoWidget, &GotoWidget::getText, this, &CodeAssistant::getText);
    connect(m_gotoWidget, &GotoWidget::insertIndicator, this, &CodeAssistant::insertIndicator);
    connect(m_positionWidget, &PositionWidget::insertText, this, &CodeAssistant::insertText);
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

void CodeAssistant::gotoShowDefinition(const QVariantMap &gotoSession, const QJsonArray &definitions) const {
    m_gotoWidget->gotoShowDefinition(gotoSession, definitions);
}

void CodeAssistant::gotoShowImplementation(const QVariantMap &gotoSession, const QJsonArray &implementations) const {
    m_gotoWidget->gotoShowImplementation(gotoSession, implementations);
}

void CodeAssistant::gotoShowReferences(const QVariantMap &gotoSession, const QJsonArray &references) const {
    m_gotoWidget->gotoShowReferences(gotoSession, references);
}

void CodeAssistant::gotoShowTypeDefinition(const QVariantMap &gotoSession, const QJsonArray &typeDefinitions) const {
    m_gotoWidget->gotoShowTypeDefinition(gotoSession, typeDefinitions);
}

void CodeAssistant::gotoResponse(const QString &hint) const {
    m_gotoWidget->gotoResponse(hint);
}

void CodeAssistant::positionShow(const QVariantMap &positionSession) const {
    m_positionWidget->positionShow(positionSession);
}

void CodeAssistant::signatureShow(const QVariantMap &signatureSession, const QJsonObject &signature) const {
    m_signatureWidget->signatureShow(signatureSession, signature);
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
    if (m_gotoWidget->isVisible()) {
        if (event->type() == QEvent::KeyPress) {
            const auto *keyEvent = static_cast<QKeyEvent *>(event);
            switch (keyEvent->key()) {
                case Qt::Key_Up: {
                    m_gotoWidget->gotoPrev();
                }
                    return true;
                case Qt::Key_Down: {
                    m_gotoWidget->gotoNext();
                }
                    return true;
                case Qt::Key_Return:
                case Qt::Key_Escape:
                case Qt::Key_Backspace:
                case Qt::Key_Left:
                case Qt::Key_Right: {
                    m_gotoWidget->gotoHide();
                }
                    return false;
                default:
                    return false;
            }
        }
    }
    if (m_positionWidget->isVisible()) {
        if (event->type() == QEvent::MouseButtonPress) {
            const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_positionWidget->textReplace();
                m_positionWidget->positionHide();
                return true;
            }
        }
    }
    if (m_signatureWidget->isVisible()) {
        if (event->type() == QEvent::KeyPress) {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            switch (keyEvent->key()) {
                case Qt::Key_Up:
                case Qt::Key_Down:
                case Qt::Key_Left:
                case Qt::Key_Right:
                case Qt::Key_Escape:
                    m_signatureWidget->signatureHide();
                    return false;
                default:
                    return false;
            }
        }
    }
    return QObject::eventFilter(obj, event);
}
