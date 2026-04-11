#include "document/documentModule.h"

#include <QFileInfo>
#include <QShortcut>
#include <QTextBrowser>
#include <QTimer>

#include "globals.h"
#include "core/fileModule.h"
#include "port/portModule.h"
#include "document/page/luaPage.h"
#include "document/page/welcomePage.h"
#include "analysis/codeAssistant.h"
#include "document/module/scintillaWidget.h"

// public
DocumentModule::DocumentModule(QWidget *parent)
    : QObject(parent),
      m_scriptConfig(g_workspaceConfig["scriptConfig"].toObject()),
      m_welcomePage(new WelcomePage()),
      m_codeAssistant(new CodeAssistant(parent)) {
    m_welcomePage->setObjectName("welcomePage");
    connect(m_welcomePage, &WelcomePage::openWorkspace, this, &DocumentModule::openWorkspace);
    connect(this, &DocumentModule::responseCodeAction, m_codeAssistant, &CodeAssistant::codeActionShow);
    connect(m_codeAssistant, &CodeAssistant::addChar, this, &DocumentModule::charAdd);
    connect(m_codeAssistant, &CodeAssistant::setIndex, this, &DocumentModule::indexSet);
    connect(m_codeAssistant, &CodeAssistant::getText, this, &DocumentModule::textGet);
    connect(m_codeAssistant, &CodeAssistant::setText, this, &DocumentModule::textSet);
    connect(m_codeAssistant, &CodeAssistant::setTextSelected, this, &DocumentModule::textSetSelected);
    connect(m_codeAssistant, &CodeAssistant::insertIndicator, this, &DocumentModule::indicatorFill);
    connect(m_codeAssistant, &CodeAssistant::requestCodeAction, this, &DocumentModule::codeActionRequest);
}

DocumentModule::~DocumentModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] script module destructed").arg(timestamp);
}

void DocumentModule::propertySet(const QVariantMap &objects) {
    m_messageDialog = qvariant_cast<QObject *>(objects["mainWindowMessageDialog"]);
    m_toolTip = qvariant_cast<QObject *>(objects["mainWindowToolTip"]);
    m_breakpointEditDialog = qvariant_cast<QObject *>(objects["breakpointModuleEditDialog"]);
    m_systemPropertyDialog = qvariant_cast<QObject *>(objects["fileModulePropertyDialog"]);
    m_editorMenu = qvariant_cast<QObject *>(objects["documentModuleEditorMenu"]);

    for (const auto &value: m_scriptConfig["scriptList"].toArray()) {
        scriptOpen(QUrl(value.toString()));
    }
    const auto focusedUrl = QUrl(m_scriptConfig["scriptFocused"].toString());
    if (!focusedUrl.isEmpty() && m_luaPageHash.contains(focusedUrl)) {
        QTimer::singleShot(0, this, [this, focusedUrl] {
            m_luaPageHash[focusedUrl]->setFocus(Qt::FocusReason::MouseFocusReason);
        });
    }

    m_welcomePage->propertySet(QVariantMap());
    m_codeAssistant->propertySet(objects);
    m_codeAssistant->fontSet(m_scriptConfig["fontFamily"].toString(), m_scriptConfig["fontSize"].toInt());
}

void DocumentModule::scriptConfigSave() {
    // save config
    auto scriptList = QJsonArray();
    for (const auto &url: m_luaPageHash.keys()) {
        LuaPage *luaPage = m_luaPageHash[url];
        luaPage->scriptSave();
        scriptList.append(url.toString());
    }
    m_scriptConfig["scriptList"] = scriptList;
    if (m_focusedPage.isEmpty()) {
        m_scriptConfig["scriptFocused"] = "";
    } else {
        m_scriptConfig["scriptFocused"] = m_focusedPage.toString();
    }
    g_workspaceConfig["scriptConfig"] = m_scriptConfig;
}

void DocumentModule::scriptFontReload(const QJsonObject &fontConfigScript) const {
    // const auto scriptFont = QFont(fontConfigScript["fontFamily"].toString(), fontConfigScript["fontSize"].toInt());
    // for (const auto &luaPage: m_luaPageHash) {
    //     luaPage->m_editorWidget->setFont(scriptFont);
    // }
}

void DocumentModule::scriptFontSave(const QJsonObject &fontConfigScript) {
    m_scriptConfig["fontFamily"] = fontConfigScript["fontFamily"].toString();
    m_scriptConfig["fontSize"] = fontConfigScript["fontSize"].toInt();
}

void DocumentModule::scriptIndicatorReload(const QJsonObject &indicatorConfigScript) const {
    // for (const auto &luaPage: m_luaPageHash) {
    //     // diagnostic
    //     luaPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorErrorStyle"].toInt()), INDICATOR_ERROR);
    //     luaPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorErrorColor"].toString()), INDICATOR_ERROR);
    //     luaPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_ERROR);
    //     luaPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorWarningStyle"].toInt()), INDICATOR_WARNING);
    //     luaPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorWarningColor"].toString()), INDICATOR_WARNING);
    //     luaPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_WARNING);
    //     luaPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorInfoStyle"].toInt()), INDICATOR_INFO);
    //     luaPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorInfoColor"].toString()), INDICATOR_INFO);
    //     luaPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_INFO);
    //     luaPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorHintStyle"].toInt()), INDICATOR_HINT);
    //     luaPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorHintColor"].toString()), INDICATOR_HINT);
    //     luaPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_HINT);
    //     // highlight
    //     luaPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorHighlightStyle"].toInt()), INDICATOR_HIGHLIGHT);
    //     luaPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorHighlightColor"].toString()), INDICATOR_HIGHLIGHT);
    //     luaPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_HIGHLIGHT);
    //     luaPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorReadStyle"].toInt()), INDICATOR_READ);
    //     luaPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorReadColor"].toString()), INDICATOR_READ);
    //     luaPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_READ);
    //     luaPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorWriteStyle"].toInt()), INDICATOR_WRITE);
    //     luaPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorWriteColor"].toString()), INDICATOR_WRITE);
    //     luaPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_WRITE);
    //     // search
    //     luaPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorSearchStyle"].toInt()), INDICATOR_SEARCH);
    //     luaPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorSearchColor"].toString()), INDICATOR_SEARCH);
    //     luaPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_SEARCH);
    //     luaPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorSelectionStyle"].toInt()), INDICATOR_SELECTION);
    //     luaPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorSelectionColor"].toString()), INDICATOR_SELECTION);
    //     luaPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_SELECTION);
    //     // hyperlink
    //     luaPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorHyperlinkStyle"].toInt()), INDICATOR_HYPERLINK);
    //     luaPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorHyperlinkColor"].toString()), INDICATOR_HYPERLINK);
    //     luaPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_HYPERLINK);
    //     // recolor
    //     luaPage->m_editorWidget->recolor();
    // }
}

void DocumentModule::scriptIndicatorSave(const QJsonObject &indicatorConfigScript) {
    // diagnostic
    m_scriptConfig["indicatorErrorStyle"] = indicatorConfigScript["indicatorErrorStyle"].toInt();
    m_scriptConfig["indicatorErrorColor"] = indicatorConfigScript["indicatorErrorColor"].toString();
    m_scriptConfig["indicatorWarningStyle"] = indicatorConfigScript["indicatorWarningStyle"].toInt();
    m_scriptConfig["indicatorWarningColor"] = indicatorConfigScript["indicatorWarningColor"].toString();
    m_scriptConfig["indicatorInfoStyle"] = indicatorConfigScript["indicatorInfoStyle"].toInt();
    m_scriptConfig["indicatorInfoColor"] = indicatorConfigScript["indicatorInfoColor"].toString();
    m_scriptConfig["indicatorHintStyle"] = indicatorConfigScript["indicatorHintStyle"].toInt();
    m_scriptConfig["indicatorHintColor"] = indicatorConfigScript["indicatorHintColor"].toString();
    // highlight
    m_scriptConfig["indicatorHighlightStyle"] = indicatorConfigScript["indicatorHighlightStyle"].toInt();
    m_scriptConfig["indicatorHighlightColor"] = indicatorConfigScript["indicatorHighlightColor"].toString();
    m_scriptConfig["indicatorReadStyle"] = indicatorConfigScript["indicatorReadStyle"].toInt();
    m_scriptConfig["indicatorReadColor"] = indicatorConfigScript["indicatorReadColor"].toString();
    m_scriptConfig["indicatorWriteStyle"] = indicatorConfigScript["indicatorWriteStyle"].toInt();
    m_scriptConfig["indicatorWriteColor"] = indicatorConfigScript["indicatorWriteColor"].toString();
    // search
    m_scriptConfig["indicatorSearchStyle"] = indicatorConfigScript["indicatorSearchStyle"].toInt();
    m_scriptConfig["indicatorSearchColor"] = indicatorConfigScript["indicatorSearchColor"].toString();
    m_scriptConfig["indicatorSelectionStyle"] = indicatorConfigScript["indicatorSelectionStyle"].toInt();
    m_scriptConfig["indicatorSelectionColor"] = indicatorConfigScript["indicatorSelectionColor"].toString();
    // hyperlink
    m_scriptConfig["indicatorHyperlinkStyle"] = indicatorConfigScript["indicatorHyperlinkStyle"].toInt();
    m_scriptConfig["indicatorHyperlinkColor"] = indicatorConfigScript["indicatorHyperlinkColor"].toString();
}

void DocumentModule::scriptMarkerReload(const QJsonObject &markerConfigScript) const {
    // for (const auto &luaPage: m_luaPageHash) {
    //     luaPage->m_editorWidget->markerDefine(static_cast<QsciScintilla::MarkerSymbol>(markerConfigScript["markerBreakpointStyle"].toInt()), MARKER_BREAKPOINT_ENABLED);
    //     luaPage->m_editorWidget->setMarkerBackgroundColor(QColor(markerConfigScript["markerBreakpointBackground"].toString()), MARKER_BREAKPOINT_ENABLED);
    //     luaPage->m_editorWidget->setMarkerForegroundColor(QColor(markerConfigScript["markerBreakpointForeground"].toString()), MARKER_BREAKPOINT_ENABLED);
    //     luaPage->m_editorWidget->markerDefine(static_cast<QsciScintilla::MarkerSymbol>(markerConfigScript["markerDebugStyle"].toInt()), MARKER_DEBUG);
    //     luaPage->m_editorWidget->setMarkerBackgroundColor(QColor(markerConfigScript["markerDebugBackground"].toString()), MARKER_DEBUG);
    //     luaPage->m_editorWidget->setMarkerForegroundColor(QColor(markerConfigScript["markerDebugForeground"].toString()), MARKER_DEBUG);
    //     // recolor
    //     luaPage->m_editorWidget->recolor();
    // }
}

void DocumentModule::scriptMarkerSave(const QJsonObject &markerConfigScript) {
    m_scriptConfig["markerBreakpointStyle"] = markerConfigScript["markerBreakpointStyle"].toInt();
    m_scriptConfig["markerBreakpointBackground"] = markerConfigScript["markerBreakpointBackground"].toString();
    m_scriptConfig["markerBreakpointForeground"] = markerConfigScript["markerBreakpointForeground"].toString();
    m_scriptConfig["markerDebugStyle"] = markerConfigScript["markerDebugStyle"].toInt();
    m_scriptConfig["markerDebugBackground"] = markerConfigScript["markerDebugBackground"].toString();
    m_scriptConfig["markerDebugForeground"] = markerConfigScript["markerDebugForeground"].toString();
}

// public: file
void DocumentModule::scriptOpen(const QUrl &scriptUrl) {
    // open page
    if (!m_luaPageHash.contains(scriptUrl)) {
        // create script page
        auto *luaPage = new LuaPage(m_scriptConfig, scriptUrl);
        luaPage->propertySet(QVariantMap{
            {"mainWindowToolTip", QVariant::fromValue(m_toolTip)},
            {"breakpointModuleEditDialog", QVariant::fromValue(m_breakpointEditDialog)},
            {"fileModulePropertyDialog", QVariant::fromValue(m_systemPropertyDialog)},
            {"documentModuleEditorMenu", QVariant::fromValue(m_editorMenu)}
        });
        luaPage->setObjectName(scriptUrl.toString());
        // check same file name
        bool conflict = false;
        for (const auto &url: m_luaPageHash.keys()) {
            if (url.fileName() == scriptUrl.fileName()) {
                conflict = true;
                LuaPage *conflictPage = m_luaPageHash[url];
                conflictPage->pathDisambiguation();
            }
        }
        if (conflict) {
            luaPage->pathDisambiguation();
        }
        // insert url to hash
        m_luaPageHash[scriptUrl] = luaPage;
        connect(luaPage, &LuaPage::isFocusedChanged, this, [this, luaPage](const bool status) { scriptFocus(luaPage, status); });
        connect(luaPage, &LuaPage::appendLog, this, &DocumentModule::appendLog);
        connect(luaPage, &LuaPage::closeScript, this, &DocumentModule::scriptClose);
        connect(luaPage, &LuaPage::startThread, this, &DocumentModule::startThread);
        connect(luaPage, &LuaPage::changeSelection, this, &DocumentModule::changeSelection);
        connect(luaPage, &LuaPage::insertBreakpoint, this, &DocumentModule::insertBreakpoint);
        connect(luaPage, &LuaPage::removeBreakpoint, this, &DocumentModule::removeBreakpoint);
        connect(luaPage, &LuaPage::requestCompletion, this, &DocumentModule::completionRequest);
        connect(luaPage, &LuaPage::requestDefinition, this, &DocumentModule::definitionRequest);
        connect(luaPage, &LuaPage::requestDocumentHighlight, this, &DocumentModule::documentHighlightRequest);
        connect(luaPage, &LuaPage::requestDocumentSymbol, this, &DocumentModule::documentSymbolRequest);
        connect(luaPage, &LuaPage::requestFoldingRange, this, &DocumentModule::foldingRangeRequest);
        connect(luaPage, &LuaPage::requestFormatting, this, &DocumentModule::formattingRequest);
        connect(luaPage, &LuaPage::requestHover, this, &DocumentModule::hoverRequest);
        connect(luaPage, &LuaPage::requestImplementation, this, &DocumentModule::implementationRequest);
        connect(luaPage, &LuaPage::requestOnTypeFormatting, this, &DocumentModule::onTypeFormattingRequest);
        connect(luaPage, &LuaPage::requestReferences, this, &DocumentModule::referencesRequest);
        connect(luaPage, &LuaPage::requestSemanticTokens, this, &DocumentModule::semanticTokensRequest);
        connect(luaPage, &LuaPage::requestSignatureHelp, this, &DocumentModule::signatureHelpRequest);
        connect(luaPage, &LuaPage::requestSpellCheck, this, &DocumentModule::requestSpellCheck);
        connect(luaPage, &LuaPage::requestTypeDefinition, this, &DocumentModule::typeDefinitionRequest);
        connect(luaPage, &LuaPage::notificationJson, this, &DocumentModule::notificationJson);
        connect(luaPage, &LuaPage::showDiagnostic, m_codeAssistant, &CodeAssistant::diagnosticShow);
        qApp->installEventFilter(m_codeAssistant);
        if (m_focusedPage.isEmpty()) {
            m_welcomePage->open();
            m_welcomePage->addDockWidgetAsTab(luaPage);
            m_welcomePage->close();
        } else {
            m_luaPageHash[m_focusedPage]->addDockWidgetAsTab(luaPage);
        }
        luaPage->diagnosticsNotification(m_diagnosticsHash[scriptUrl]);
    }
    m_luaPageHash[scriptUrl]->raise();
    m_luaPageHash[scriptUrl]->setFocus(Qt::FocusReason::MouseFocusReason);
}

QVariantHash DocumentModule::menuGet(const QString &name) {
    return m_luaPageHash[m_focusedPage]->menuGet(name);
}

void DocumentModule::menuRequest(const QString &request) {
    m_luaPageHash[m_focusedPage]->menuRequest(request);
}

int DocumentModule::eolModeGet(const QUrl &scriptUrl) const {
    return m_luaPageHash[scriptUrl]->m_editorWidget->eolModeGet();
}

void DocumentModule::eolModeSet(const QUrl &scriptUrl, const int eolMode) const {
    m_luaPageHash[scriptUrl]->m_editorWidget->eolModeSet(eolMode);
}

bool DocumentModule::eolViewGet(const QUrl &scriptUrl) {
    return m_luaPageHash[scriptUrl]->m_editorWidget->eolViewGet();
}

void DocumentModule::eolViewSet(const QUrl &scriptUrl, const bool status) const {
    m_luaPageHash[scriptUrl]->m_editorWidget->eolViewSet(status);
}

// public: document
void DocumentModule::foldContractTop(const QUrl &scriptUrl) {
    if (!m_luaPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    m_luaPageHash[scriptUrl]->m_editorWidget->foldContractTop();
}

void DocumentModule::foldContractRecursively(const QUrl &scriptUrl) {
    if (!m_luaPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    m_luaPageHash[scriptUrl]->m_editorWidget->foldContractRecursively();
}

void DocumentModule::foldExpandRecursively(const QUrl &scriptUrl) {
    if (!m_luaPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    m_luaPageHash[scriptUrl]->m_editorWidget->foldExpandRecursively();
}

void DocumentModule::assemblyToggle(const QUrl &scriptUrl, const bool status) {
    m_luaPageHash[scriptUrl]->assemblyToggle(status);
}

void DocumentModule::focusSet(const QUrl &scriptUrl, const bool status) {
    if (!m_luaPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    m_luaPageHash[scriptUrl]->m_editorWidget->focusSet(status);
}

void DocumentModule::indexSet(const QUrl &scriptUrl, const int line, const int character) {
    if (!m_luaPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    m_luaPageHash[scriptUrl]->m_editorWidget->indexSet(line, character);
}

void DocumentModule::indexGet() const {
    const auto scriptUrl = m_focusedPage;
    const auto index = m_luaPageHash[scriptUrl]->m_editorWidget->indexGet();
    g_cursorPosition = {
        {"url", scriptUrl},
        {"line", index["line"]},
        {"character", index["character"]}
    };
}

QString DocumentModule::textGet(const QUrl &scriptUrl, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
    if (m_luaPageHash.contains(scriptUrl)) {
        return m_luaPageHash[scriptUrl]->m_editorWidget->textGet(startLine, startCharacter, endLine, endCharacter);
    }
    return FileModule::textGet(scriptUrl, startLine, startCharacter, endLine, endCharacter);
}

void DocumentModule::indicatorFill(const QUrl &scriptUrl, const int type, const int startLine, const int startCharacter, const int endLine, const int endCharacter, const int time) {
    if (!m_luaPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    m_luaPageHash[scriptUrl]->m_editorWidget->indicatorFill(type, startLine, startCharacter, endLine, endCharacter, time);
}

void DocumentModule::indicatorClear(const QUrl &scriptUrl, const int type, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
    if (!m_luaPageHash.contains(scriptUrl)) return;
    m_luaPageHash[scriptUrl]->m_editorWidget->indicatorClear(type, startLine, startCharacter, endLine, endCharacter);
}

void DocumentModule::markerAdd(const QUrl &scriptUrl, const int type, const int line, const int time) {
    if (!m_luaPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    m_luaPageHash[scriptUrl]->m_editorWidget->markerAdd(type, line, time);
}

void DocumentModule::markerDelete(const QUrl &scriptUrl, const int type, const int line) {
    if (!m_luaPageHash.contains(scriptUrl)) return;
    m_luaPageHash[scriptUrl]->m_editorWidget->markerDelete(type, line);
}

// public: lsp
void DocumentModule::diagnosticsNotification(const QUrl &scriptUrl, const QJsonArray &diagnostics) {
    m_diagnosticsHash.insert(scriptUrl, diagnostics);
    if (m_luaPageHash.contains(scriptUrl)) {
        m_luaPageHash[scriptUrl]->diagnosticsNotification(diagnostics);
    }
}

void DocumentModule::codeActionRequest(const QUrl &scriptUrl, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
    QJsonArray diagnosticArray{};
    for (const auto &value: m_diagnosticsHash[scriptUrl]) {
        const QJsonObject diagnostic = value.toObject();
        const QJsonObject range = diagnostic["range"].toObject();
        const QJsonObject start = range["start"].toObject();
        const QJsonObject end = range["end"].toObject();
        if (startLine == start["line"].toInt() && startCharacter == start["character"].toInt() && endLine == end["line"].toInt() && endCharacter == end["character"].toInt()) {
            diagnosticArray.append(diagnostic);
        }
    }
    // code action request to lua language server
    const QJsonObject codeActionParams{
        {
            "textDocument", QJsonObject{
                {"uri", scriptUrl.toString()}
            }
        },
        {
            "range", QJsonObject{
                {
                    "start", QJsonObject{
                        {"line", startLine},
                        {"character", startCharacter}
                    }
                },
                {
                    "end", QJsonObject{
                        {"line", endLine},
                        {"character", endCharacter}
                    }
                }
            }
        },
        {
            "context", QJsonObject{
                {"diagnostics", diagnosticArray}
            }
        }
    };
    emit requestJson("textDocument/codeAction", codeActionParams);
}

// codeActionResponse is sent to codeAssistant module

void DocumentModule::completionRequest(const QUrl &scriptUrl, int line, int character) {
    // completion request to lua language server
    const QJsonObject completionParams{
        {
            "textDocument", QJsonObject{
                {"uri", scriptUrl.toString()}
            }
        },
        {
            "position", QJsonObject{
                {"line", line},
                {"character", character}
            }
        }
    };
    emit requestJson("textDocument/completion", completionParams);
}

void DocumentModule::completionResponse(const QUrl &scriptUrl, const QJsonArray &items) const {
    const auto *luaPage = m_luaPageHash[scriptUrl];
    const auto *scintilla = static_cast<ScintillaWidget *>(luaPage->m_editorWidget);
    const auto wordIndex = scintilla->wordIndexGet();
    const auto startLine = wordIndex["startLine"];
    const auto startCharacter = wordIndex["startCharacter"];
    const auto endLine = wordIndex["endLine"];
    const auto endCharacter = wordIndex["endCharacter"];
    const auto typed = scintilla->indexGet()["character"] - startCharacter;
    const auto height = scintilla->heightGet();
    const auto point = scintilla->pointGet(startLine, startCharacter);
    const auto x = point["x"];
    const auto y = point["y"] + height;
    const QPoint position = scintilla->mapToGlobal(QPoint(x, y));
    // call completion show
    const QVariantHash completionSession = {
        {"scriptUrl", scriptUrl},
        {"position", position},
        {"startLine", startLine},
        {"startCharacter", startCharacter},
        {"endLine", endLine},
        {"endCharacter", endCharacter},
        {"typed", typed}
    };
    m_codeAssistant->completionShow(completionSession, items);
}

void DocumentModule::definitionRequest(const QUrl &scriptUrl, const int line, const int character) {
    // definition request to lua language server
    const QJsonObject definitionParams{
        {
            "textDocument", QJsonObject{
                {"uri", scriptUrl.toString()}
            }
        },
        {
            "position", QJsonObject{
                {"line", line},
                {"character", character}
            }
        }
    };
    emit requestJson("textDocument/definition", definitionParams);
}

void DocumentModule::definitionResponse(const QUrl &scriptUrl, const QJsonArray &definitions) const {
    const auto *luaPage = m_luaPageHash[scriptUrl];
    const auto *scintilla = static_cast<ScintillaWidget *>(luaPage->m_editorWidget);
    const auto wordIndex = scintilla->wordIndexGet();
    const auto startLine = wordIndex["startLine"];
    const auto startCharacter = wordIndex["startCharacter"];
    const auto height = scintilla->heightGet();
    const auto point = scintilla->pointGet(startLine, startCharacter);
    const auto x = point["x"];
    const auto y = point["y"] + height;
    const QPoint position = scintilla->mapToGlobal(QPoint(x, y));
    // call navigation show
    const QVariantHash navigationSession = {
        {"type", "definition"},
        {"scriptUrl", scriptUrl},
        {"position", position}
    };
    m_codeAssistant->navigationShow(navigationSession, definitions);
}

void DocumentModule::documentSymbolRequest(const QUrl &scriptUrl) {
    // document symbol request to lua language server
    const QJsonObject documentSymbolParams{
        {
            "textDocument", QJsonObject{
                {"uri", scriptUrl.toString()}
            }
        }
    };
    emit requestJson("textDocument/documentSymbol", documentSymbolParams);
}

void DocumentModule::documentSymbolResponse(const QUrl &scriptUrl, const QJsonArray &result) {
    m_luaPageHash[scriptUrl]->documentSymbolResponse(result);
}

void DocumentModule::documentHighlightRequest(const QUrl &scriptUrl, const int line, const int character) {
    // document highlight request to lua language server
    const QJsonObject documentHighlightParams{
        {
            "textDocument", QJsonObject{
                {"uri", scriptUrl.toString()}
            }
        },
        {
            "position", QJsonObject{
                {"line", line},
                {"character", character}
            }
        }
    };
    emit requestJson("textDocument/documentHighlight", documentHighlightParams);
}

void DocumentModule::documentHighlightResponse(const QUrl &scriptUrl, const QJsonArray &result) {
    m_luaPageHash[scriptUrl]->documentHighlightResponse(result);
}

void DocumentModule::foldingRangeRequest(const QUrl &scriptUrl) {
    // folding range request to lua language server
    const QJsonObject foldingRangeParams{
        {
            "textDocument", QJsonObject{
                {"uri", scriptUrl.toString()}
            }
        }
    };
    emit requestJson("textDocument/foldingRange", foldingRangeParams);
}

void DocumentModule::foldingRangeResponse(const QUrl &scriptUrl, const QJsonArray &result) const {
    m_luaPageHash[scriptUrl]->foldingRangeResponse(result);
}

void DocumentModule::formattingRequest(const QUrl &scriptUrl) {
    // formatting request to lua language server
    const QJsonObject formattingParams{
        {
            "textDocument", QJsonObject{
                {"uri", scriptUrl.toString()}
            }
        },
        {
            "options", QJsonObject{
                {"tabSize", 4},
                {"insertSpaces", true},
                {"trimTrailingWhitespace", true},
                {"insertFinalNewline", true}
            }
        }
    };
    emit requestJson("textDocument/formatting", formattingParams);
}

void DocumentModule::formattingResponse(const QUrl &scriptUrl, const QString &newText) const {
    if (newText.isEmpty()) {
        m_messageDialog->setProperty("title", tr("Information"));
        m_messageDialog->setProperty("text", tr("File already reformatted."));
        QMetaObject::invokeMethod(m_messageDialog, "open");
    } else {
        m_luaPageHash[scriptUrl]->formattingResponse(newText);
    }
}

void DocumentModule::hoverRequest(const QUrl &scriptUrl, int line, int character) {
    // hover request to lua language server
    const QJsonObject hoverParams{
        {
            "textDocument", QJsonObject{
                {"uri", scriptUrl.toString()}
            }
        },
        {
            "position", QJsonObject{
                {"line", line},
                {"character", character}
            }
        }
    };
    emit requestJson("textDocument/hover", hoverParams);
}

void DocumentModule::hoverResponse(const QUrl &scriptUrl, const QString &message) const {
    const QPoint position = QCursor::pos() + QPoint(10, 10);
    // call hover show
    const QVariantHash hoverSession = {
        {"position", position}
    };
    m_codeAssistant->hoverShow(hoverSession, message);
}

void DocumentModule::implementationRequest(const QUrl &scriptUrl, const int line, const int character) {
    // implementation request to lua language server
    const QJsonObject implementationParams{
        {
            "textDocument", QJsonObject{
                {"uri", scriptUrl.toString()}
            }
        },
        {
            "position", QJsonObject{
                {"line", line},
                {"character", character}
            }
        }
    };
    emit requestJson("textDocument/implementation", implementationParams);
}

void DocumentModule::implementationResponse(const QUrl &scriptUrl, const QJsonArray &implementations) const {
    const auto *luaPage = m_luaPageHash[scriptUrl];
    const auto *scintilla = static_cast<ScintillaWidget *>(luaPage->m_editorWidget);
    const auto wordIndex = scintilla->wordIndexGet();
    const auto startLine = wordIndex["startLine"];
    const auto startCharacter = wordIndex["startCharacter"];
    const auto height = scintilla->heightGet();
    const auto point = scintilla->pointGet(startLine, startCharacter);
    const auto x = point["x"];
    const auto y = point["y"] + height;
    const QPoint position = scintilla->mapToGlobal(QPoint(x, y));
    // call navigation show
    const QVariantHash navigationSession = {
        {"type", "implementation"},
        {"scriptUrl", scriptUrl},
        {"position", position}
    };
    m_codeAssistant->navigationShow(navigationSession, implementations);
}

void DocumentModule::onTypeFormattingRequest(const QUrl &scriptUrl, int line, int character) {
    // on type formatting request to lua language server
    const QJsonObject onTypeFormattingParams{
        {
            "textDocument", QJsonObject{
                {"uri", scriptUrl.toString()}
            }
        },
        {
            "position", QJsonObject{
                {"line", line},
                {"character", character}
            }
        },
        {"ch", "\n"},
        {
            "options", QJsonObject{
                {"tabSize", 4},
                {"insertSpaces", true},
                {"trimTrailingWhitespace", true},
                {"insertFinalNewline", true}
            }
        }
    };
    emit requestJson("textDocument/onTypeFormatting", onTypeFormattingParams);
}

void DocumentModule::onTypeFormattingResponse(const QUrl &scriptUrl, const QJsonObject &newText) const {
    m_luaPageHash[scriptUrl]->onTypeFormattingResponse(newText);
}

void DocumentModule::rangeFormattingRequest(const QUrl &scriptUrl, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
    // rangeFormatting request to lua language server
    const QJsonObject rangeFormattingParams{
        {
            "textDocument", QJsonObject{
                {"uri", scriptUrl.toString()}
            }
        },
        {
            "range", QJsonObject{
                {
                    "start", QJsonObject{
                        {"line", startLine},
                        {"character", startCharacter}
                    }
                },
                {
                    "end", QJsonObject{
                        {"line", endLine},
                        {"character", endCharacter}
                    }
                }
            }
        },
        {
            "options", QJsonObject{
                {"tabSize", 4},
                {"insertSpaces", true},
                {"trimTrailingWhitespace", true},
                {"insertFinalNewline", true}
            }
        }
    };
    emit requestJson("textDocument/rangeFormatting", rangeFormattingParams);
}

void DocumentModule::rangeFormattingResponse(const QUrl &scriptUrl, const QString &newText) const {
    auto text = newText;
    if (text.endsWith("\r\n")) text.chop(2);
    m_luaPageHash[scriptUrl]->rangeFormattingResponse(text);
}

void DocumentModule::referencesRequest(const QUrl &scriptUrl, int line, int character) {
    // references request to lua language server
    const QJsonObject referencesParams{
        {
            "textDocument", QJsonObject{
                {"uri", scriptUrl.toString()}
            }
        },
        {
            "position", QJsonObject{
                {"line", line},
                {"character", character}
            }
        },
        {
            "context", QJsonObject{
                {"includeDeclaration", false}
            }
        }
    };
    emit requestJson("textDocument/references", referencesParams);
}

void DocumentModule::referencesResponse(const QUrl &scriptUrl, const QJsonArray &references) const {
    const auto *luaPage = m_luaPageHash[scriptUrl];
    const auto *scintilla = static_cast<ScintillaWidget *>(luaPage->m_editorWidget);
    const auto wordIndex = scintilla->wordIndexGet();
    const auto startLine = wordIndex["startLine"];
    const auto startCharacter = wordIndex["startCharacter"];
    const auto height = scintilla->heightGet();
    const auto point = scintilla->pointGet(startLine, startCharacter);
    const auto x = point["x"];
    const auto y = point["y"] + height;
    const QPoint position = scintilla->mapToGlobal(QPoint(x, y));
    // call navigation show
    const QVariantHash navigationSession = {
        {"type", "reference"},
        {"scriptUrl", scriptUrl},
        {"position", position}
    };
    m_codeAssistant->navigationShow(navigationSession, references);
}

void DocumentModule::semanticTokensRequest(const QUrl &scriptUrl) {
    // semantic tokens request to lua language server
    const QJsonObject semanticTokensParams{
        {
            "textDocument", QJsonObject{
                {"uri", scriptUrl.toString()}
            }
        }
    };
    emit requestJson("textDocument/semanticTokens/full", semanticTokensParams);
}

void DocumentModule::semanticTokensResponse(const QUrl &scriptUrl, const QJsonArray &data) const {
    m_luaPageHash[scriptUrl]->semanticTokensResponse(data);
}

void DocumentModule::signatureHelpRequest(const QUrl &scriptUrl, int line, int character) {
    // signature help request to lua language server
    const QJsonObject signatureHelpParams{
        {
            "textDocument", QJsonObject{
                {"uri", scriptUrl.toString()}
            }
        },
        {
            "position", QJsonObject{
                {"line", line},
                {"character", character}
            }
        }
    };
    emit requestJson("textDocument/signatureHelp", signatureHelpParams);
}

void DocumentModule::signatureHelpResponse(const QUrl &scriptUrl, const QJsonArray &signatures) const {
    const auto *luaPage = m_luaPageHash[scriptUrl];
    const auto *scintilla = static_cast<ScintillaWidget *>(luaPage->m_editorWidget);
    const auto wordIndex = scintilla->wordIndexGet();
    const auto startLine = wordIndex["startLine"];
    const auto startCharacter = wordIndex["startCharacter"];
    const auto point = scintilla->pointGet(startLine, startCharacter - 1);
    const auto x = point["x"];
    const auto y = point["y"];
    const QPoint position = scintilla->mapToGlobal(QPoint(x, y));
    // call signature show
    const QVariantHash signatureSession = {
        {"scriptUrl", scriptUrl},
        {"position", position}
    };
    m_codeAssistant->signatureShow(signatureSession, signatures);
}

void DocumentModule::typeDefinitionRequest(const QUrl &scriptUrl, const int line, const int character) {
    // type definition request to lua language server
    const QJsonObject typeDefinitionParams{
        {
            "textDocument", QJsonObject{
                {"uri", scriptUrl.toString()}
            }
        },
        {
            "position", QJsonObject{
                {"line", line},
                {"character", character}
            }
        }
    };
    emit requestJson("textDocument/typeDefinition", typeDefinitionParams);
}

void DocumentModule::typeDefinitionResponse(const QUrl &scriptUrl, const QJsonArray &typeDefinitions) const {
    const auto *luaPage = m_luaPageHash[scriptUrl];
    const auto *scintilla = static_cast<ScintillaWidget *>(luaPage->m_editorWidget);
    const auto wordIndex = scintilla->wordIndexGet();
    const auto startLine = wordIndex["startLine"];
    const auto startCharacter = wordIndex["startCharacter"];
    const auto height = scintilla->heightGet();
    const auto point = scintilla->pointGet(startLine, startCharacter);
    const auto x = point["x"];
    const auto y = point["y"] + height;
    const QPoint position = scintilla->mapToGlobal(QPoint(x, y));
    // call navigation show
    const QVariantHash navigationSession = {
        {"type", "typeDefinition"},
        {"scriptUrl", scriptUrl},
        {"position", position}
    };
    m_codeAssistant->navigationShow(navigationSession, typeDefinitions);
}

// public: typo
void DocumentModule::spellCheckResponse(const QUrl &scriptUrl, const QVariantList &typos) {
    if (!m_luaPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    m_luaPageHash[scriptUrl]->spellCheckResponse(typos);
}

// private
void DocumentModule::scriptFocus(const LuaPage *luaPage, const bool status) {
    if (status) {
        m_focusedPage = luaPage->m_scriptUrl;
        const QVariantHash session = {
            {"codePage", luaPage->m_editorWidget->codePageGet()},
            {"eolMode", luaPage->m_editorWidget->eolModeGet()}
        };
        emit focusScript(luaPage->m_scriptUrl, session);
    } else {
        luaPage->m_editorWidget->indicatorClear(INDICATOR_HIGHLIGHT);
        luaPage->m_editorWidget->indicatorClear(INDICATOR_READ);
        luaPage->m_editorWidget->indicatorClear(INDICATOR_WRITE);
    }
}

void DocumentModule::scriptClose(const QUrl &scriptUrl) {
    m_luaPageHash.remove(scriptUrl);
    if (m_luaPageHash.isEmpty()) {
        m_welcomePage->open();
        m_focusedPage = nullptr;
    } else {
        const auto begin = m_luaPageHash.begin();
        m_focusedPage = begin.key();
    }
}

void DocumentModule::charAdd(const QUrl &scriptUrl, const QChar character) const {
    m_luaPageHash[scriptUrl]->charAdd(character.toLatin1());
}

void DocumentModule::textSet(const QUrl &scriptUrl, const QString &text, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
    if (!m_luaPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    m_luaPageHash[scriptUrl]->m_editorWidget->textSet(text, startLine, startCharacter, endLine, endCharacter);
}

void DocumentModule::textSetSelected(const QUrl &scriptUrl, const QString &text) {
    if (!m_luaPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    m_luaPageHash[scriptUrl]->m_editorWidget->textSetSelected(text);
}
