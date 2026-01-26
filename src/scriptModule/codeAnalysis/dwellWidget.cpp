#include "scriptModule/codeAnalysis/dwellWidget.h"

#include <QMenu>
#include <QTextBrowser>
#include <QTimer>
#include <QVBoxLayout>

#include "globals.h"
#include "scriptModule/nuspellModule.h"

// DwellWidget public
DwellWidget::DwellWidget(QWidget *parent)
    : QObject(parent) {
}

void DwellWidget::propertySet(const QVariantMap &objects) {
    m_tooltip = qvariant_cast<QObject *>(objects["scriptModuleDwellToolTip"]);
    m_tooltip->setProperty("dwellWidget", QVariant::fromValue(this));
    m_diagnosticTextArea = qvariant_cast<QObject *>(objects["scriptModuleDwellDiagnosticTextArea"]);
    m_hoverTextArea = qvariant_cast<QObject *>(objects["scriptModuleDwellHoverTextArea"]);
    m_suggestionMenu = qvariant_cast<QObject *>(objects["scriptModuleDwellSuggestionMenu"]);
}

void DwellWidget::diagnosticShow(const QVariantHash &diagnosticSession, const QString &message) {
    m_scriptUrl = diagnosticSession["scriptUrl"].toUrl();
    m_diagnosticTextArea->setProperty("text", message);
    const auto position = diagnosticSession["position"].toPoint();
    m_tooltip->setProperty("position", position);
    QMetaObject::invokeMethod(m_tooltip, "open");
}

void DwellWidget::hoverShow(const QVariantHash &hoverSession, const QString &message) const {
    m_hoverTextArea->setProperty("text", message);
    const auto position = hoverSession["position"].toPoint();
    m_tooltip->setProperty("position", position);
    QMetaObject::invokeMethod(m_tooltip, "open");
}

void DwellWidget::dwellShowCodeAction(const QUrl &scriptUrl, const QJsonArray &result) const {
    qDebug() << result;
}

void DwellWidget::dwellHide() const {
    QMetaObject::invokeMethod(m_tooltip, "close");
}

void DwellWidget::linkClick(const QUrl &commandLine) {
    // qDebug() << commandLine;
    const QString command = commandLine.scheme();
    if (command == "requestcodeaction") {
        const QStringList arguments = commandLine.path().split('/');
        m_diagnosticLineFrom = arguments[1].toInt();
        m_diagnosticIndexFrom = arguments[2].toInt();
        m_diagnosticLineTo = arguments[3].toInt();
        m_diagnosticIndexTo = arguments[4].toInt();
        emit requestCodeAction(m_scriptUrl, m_diagnosticLineFrom, m_diagnosticIndexFrom, m_diagnosticLineTo, m_diagnosticIndexTo);
    } else if (command == "requestspellsuggest") {
        const QString word = commandLine.host();
        const QStringList arguments = commandLine.path().split('/');
        m_typoLineFrom = arguments[1].toInt();
        m_typoIndexFrom = arguments[2].toInt();
        m_typoLineTo = arguments[3].toInt();
        m_typoIndexTo = arguments[4].toInt();
        const QStringList suggestions = g_nuspell->spellSuggestRequest(word);
        m_tooltip->setProperty("suggestions", suggestions);
        QMetaObject::invokeMethod(m_suggestionMenu, "open");
    }
}

void DwellWidget::textReplace(const QString &text) {
    emit replaceText(m_scriptUrl, text, m_typoLineFrom, m_typoIndexFrom, m_typoLineTo, m_typoIndexTo);
}
