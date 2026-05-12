#include "document/page/luaPage.h"

#include <QVBoxLayout>

#include "globals.h"
#include "analysis/symbolWidget.h"
#include "document/module/scintillaWidget.h"

// public
LuaPage::LuaPage(const QJsonObject &documentConfig, const QUrl &documentUrl)
    : BasePage(documentUrl),
      m_codeWidget(new CodeWidget(documentConfig, documentUrl, this)),
      m_symbolWidget(new SymbolWidget(this)) {
    auto *widget = new QWidget(); // NOLINT
    setWidget(widget);
    auto *layout = new QVBoxLayout(widget); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_codeWidget);
    layout->addWidget(m_symbolWidget);

    connect(m_codeWidget, &EditorWidget::appendLog, this, &LuaPage::appendLog);
    connect(m_codeWidget, &EditorWidget::changeSavepoint, this, &LuaPage::savepointChange);
    connect(m_codeWidget, &EditorWidget::changeSelection, this, &LuaPage::changeSelection);
    connect(m_codeWidget, &CodeWidget::startThread, this, &LuaPage::startThread);
    connect(m_codeWidget, &CodeWidget::insertBreakpoint, this, &LuaPage::insertBreakpoint);
    connect(m_codeWidget, &CodeWidget::notificationJson, this, &LuaPage::notificationJson);
    connect(m_codeWidget, &CodeWidget::requestCompletion, this, &LuaPage::requestCompletion);
    connect(m_codeWidget, &CodeWidget::requestDefinition, this, &LuaPage::requestDefinition);
    connect(m_codeWidget, &CodeWidget::requestDocumentHighlight, this, &LuaPage::requestDocumentHighlight);
    connect(m_codeWidget, &CodeWidget::requestDocumentSymbol, this, &LuaPage::requestDocumentSymbol);
    connect(m_codeWidget, &CodeWidget::requestFoldingRange, this, &LuaPage::requestFoldingRange);
    connect(m_codeWidget, &CodeWidget::requestFormatting, this, &LuaPage::requestFormatting);
    connect(m_codeWidget, &CodeWidget::requestHover, this, &LuaPage::requestHover);
    connect(m_codeWidget, &CodeWidget::requestImplementation, this, &LuaPage::requestImplementation);
    connect(m_codeWidget, &CodeWidget::requestOnTypeFormatting, this, &LuaPage::requestOnTypeFormatting);
    connect(m_codeWidget, &CodeWidget::requestReferences, this, &LuaPage::requestReferences);
    connect(m_codeWidget, &CodeWidget::requestSemanticTokens, this, &LuaPage::requestSemanticTokens);
    connect(m_codeWidget, &CodeWidget::requestSignatureHelp, this, &LuaPage::requestSignatureHelp);
    connect(m_codeWidget, &CodeWidget::requestSpellCheck, this, &LuaPage::requestSpellCheck);
    connect(m_codeWidget, &CodeWidget::requestTypeDefinition, this, &LuaPage::requestTypeDefinition);
    connect(m_codeWidget, &CodeWidget::showDiagnostic, this, &LuaPage::showDiagnostic);
    connect(m_codeWidget, &CodeWidget::showDocumentSymbol, this, [this](const int line, const int character) { m_symbolWidget->documentSymbolShow(m_symbol, line, character); });
    connect(m_symbolWidget, &SymbolWidget::appendLog, this, &LuaPage::appendLog);
    connect(m_symbolWidget, &SymbolWidget::setFocus, handler(), &ScintillaWidget::focusSet);
    connect(m_symbolWidget, &SymbolWidget::setIndex, handler(), &ScintillaWidget::indexSet);
    connect(m_symbolWidget, &SymbolWidget::fillIndicator, handler(), &ScintillaWidget::indicatorFill);
}

void LuaPage::propertySet(const QVariantHash &objects) {
    m_saveDialog = qvariant_cast<QObject *>(objects["documentModuleSaveDialog"]);
    m_codeWidget->propertySet(QVariantHash{
        {"global", objects["global"]},
        {"mainWindowToolTip", objects["mainWindowToolTip"]},
        {"breakpointModuleEditDialog", objects["breakpointModuleEditDialog"]},
        {"documentModuleGotoDialog", objects["documentModuleGotoDialog"]},
        {"documentModuleEditorMenu", objects["documentModuleEditorMenu"]}
    });
    m_symbolWidget->propertySet(QVariantHash{
        {"global", objects["global"]},
        {"mainWindowToolTip", objects["mainWindowToolTip"]}
    });
}

void LuaPage::documentSave() {
    m_codeWidget->documentSave();
}

QVariantHash LuaPage::menuGet(const QString &name) const {
    return m_codeWidget->menuGet(name);
}

void LuaPage::menuRequest(const QString &request) const {
    m_codeWidget->menuRequest(request);
}

void LuaPage::diagnosticsNotification(const QJsonArray &diagnostics) const {
    m_codeWidget->diagnosticsNotification(diagnostics);
}

void LuaPage::documentHighlightResponse(const QJsonArray &result) const {
    m_codeWidget->documentHighlightResponse(result);
}

void LuaPage::documentSymbolResponse(const QJsonArray &result) {
    if (!m_documentUrl.toString().endsWith(".lua")) return;
    m_symbol = result;
}

void LuaPage::foldingRangeResponse(const QJsonArray &result) const {
    m_codeWidget->foldingRangeResponse(result);
}

void LuaPage::formattingResponse(const QString &newText) const {
    m_codeWidget->formattingResponse(newText);
}

void LuaPage::onTypeFormattingResponse(const QJsonObject &newText) const {
    m_codeWidget->onTypeFormattingResponse(newText);
}

void LuaPage::rangeFormattingResponse(const QString &newText) const {
    m_codeWidget->rangeFormattingResponse(newText);
}

void LuaPage::semanticTokensResponse(const QJsonArray &data) const {
    m_codeWidget->semanticTokensResponse(data);
}

void LuaPage::spellCheckResponse(const QVariantList &typos) const {
    m_codeWidget->spellCheckResponse(typos);
}

// protected
bool LuaPage::documentClose() {
    bool status = true;
    if (handler()->modifyGet()) {
        m_saveDialog->setProperty("documentUrl", m_documentUrl);
        m_saveDialog->setProperty("documentName", m_documentUrl.fileName());
        QMetaObject::invokeMethod(m_saveDialog, "open");
        const auto eventloop = new QEventLoop(this);
        const auto conn = connect(m_saveDialog, SIGNAL(closed()), eventloop, SLOT(quit()));
        eventloop->exec();
        disconnect(conn);
        delete eventloop;
        status = m_saveDialog->property("status").toBool();
    }
    return status;
}

// private
void LuaPage::savepointChange(const bool status) {
    const QString pageName = title();
    if (status) {
        setTitle(pageName + "*");
    } else {
        setTitle(pageName.chopped(1));
    }
}
