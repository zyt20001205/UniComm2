#include "scriptModule/codeAnalysis/codeAssistant.h"

#include <QJsonObject>
#include <QKeyEvent>

#include "scriptModule/codeAnalysis/completionWidget.h"
#include "scriptModule/codeAnalysis/dwellWidget.h"
#include "scriptModule/codeAnalysis/navigationWidget.h"
#include "scriptModule/codeAnalysis/positionWidget.h"
#include "scriptModule/codeAnalysis/signatureWidget.h"

// CodeAssistant public
CodeAssistant::CodeAssistant(QWidget *parent)
    : QObject(parent),
      m_completionWidget(new CompletionWidget(parent)),
      m_dwellWidget(new DwellWidget(parent)),
      m_navigationWidget(new NavigationWidget(parent)),
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
    connect(m_dwellWidget, &DwellWidget::requestCodeAction, this, &CodeAssistant::requestCodeAction);
    connect(m_navigationWidget, &NavigationWidget::setCursorPosition, this, &CodeAssistant::setCursorPosition);
    connect(m_navigationWidget, &NavigationWidget::getText, this, &CodeAssistant::getText);
    connect(m_navigationWidget, &NavigationWidget::insertIndicator, this, &CodeAssistant::insertIndicator);
    connect(m_positionWidget, &PositionWidget::insertText, this, &CodeAssistant::insertText);
}

void CodeAssistant::propertySet(const QVariantMap &objects) const {
    m_completionWidget->propertySet(QVariantMap{
        {"scriptModuleCompletionToolTip", objects["scriptModuleCompletionToolTip"]},
        {"scriptModuleCompletionTableView", objects["scriptModuleCompletionTableView"]},
        {"scriptModuleCompletionDetailTableView", objects["scriptModuleCompletionDetailTableView"]}
    });
    m_dwellWidget->propertySet(QVariantMap{
        {"scriptModuleDwellToolTip", objects["scriptModuleDwellToolTip"]}
    });
    m_signatureWidget->propertySet(QVariantMap{
        {"scriptModuleSignatureToolTip", objects["scriptModuleSignatureToolTip"]},
        {"scriptModuleSignatureLabel", objects["scriptModuleSignatureLabel"]}
    });
}

void CodeAssistant::fontSet(const QString &family, const int pointSize) const {
    m_completionWidget->fontSet(family, pointSize);
    m_signatureWidget->fontSet(family, pointSize);
}

void CodeAssistant::completionShow(const QVariantHash &completionSession, const QJsonArray &items) const {
    m_completionWidget->completionShow(completionSession, items);
}

void CodeAssistant::completionHide() const {
    m_completionWidget->completionHide();
}

void CodeAssistant::dwellShowDiagnostic(const QUrl &scriptUrl, const QString &message) const {
    m_dwellWidget->dwellShowDiagnostic(scriptUrl, message);
}

void CodeAssistant::hoverShow(const QVariantHash &hoverSession, const QString &message) const {
    m_dwellWidget->hoverShow(hoverSession, message);
}

void CodeAssistant::dwellShowCodeAction(const QUrl &scriptUrl, const QJsonArray &result) const {
    m_dwellWidget->dwellShowCodeAction(scriptUrl, result);
}

void CodeAssistant::dwellHide() const {
    m_dwellWidget->dwellHide();
}

void CodeAssistant::dwellLeave() const {
    m_dwellWidget->dwellLeave();
}

void CodeAssistant::navigationShow(const QVariantHash &navigationSession, const QJsonArray &navigations) const {
    m_navigationWidget->navigationShow(navigationSession, navigations);
}

void CodeAssistant::navigationResponse(const QString &hint) const {
    m_navigationWidget->navigationResponse(hint);
}

void CodeAssistant::positionShow(const QVariantMap &positionSession) const {
    m_positionWidget->positionShow(positionSession);
}

void CodeAssistant::signatureShow(const QVariantHash &signatureSession, const QJsonObject &signature) const {
    m_signatureWidget->signatureShow(signatureSession, signature);
}

// CodeAssistant protected
bool CodeAssistant::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
            // hide keys
            case Qt::Key_Escape:
            case Qt::Key_Backspace:
            case Qt::Key_Left:
            case Qt::Key_Right: {
                m_completionWidget->completionHide();
                m_signatureWidget->signatureHide();
            }
                return false;
            // selection keys
            case Qt::Key_Up: {
                if (m_completionWidget->isVisible()) {
                    m_completionWidget->completionPrev();
                    return true;
                } else {
                    m_signatureWidget->signatureHide();
                    return false;
                }
            }
            case Qt::Key_Down: {
                if (m_completionWidget->isVisible()) {
                    m_completionWidget->completionNext();
                    return true;
                } else {
                    m_signatureWidget->signatureHide();
                    return false;
                }
            }
            // completion keys
            case Qt::Key_Tab: {
                if (m_completionWidget->isVisible()) {
                    m_completionWidget->textReplace();
                    m_completionWidget->completionHide();
                    return true;
                }
                else {
                    return false;
                }
            }

            default:
                return false;
        }
    }
    if (m_navigationWidget->isVisible()) {
        if (event->type() == QEvent::KeyPress) {
            const auto *keyEvent = static_cast<QKeyEvent *>(event);
            switch (keyEvent->key()) {
                case Qt::Key_Up: {
                    m_navigationWidget->navigationPrev();
                }
                    return true;
                case Qt::Key_Down: {
                    m_navigationWidget->navigationNext();
                }
                    return true;
                case Qt::Key_Return:
                case Qt::Key_Escape:
                case Qt::Key_Backspace:
                case Qt::Key_Left:
                case Qt::Key_Right: {
                    m_navigationWidget->navigationHide();
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
    return QObject::eventFilter(obj, event);
}
