#include "document/documentModule.h"

#include <QFileInfo>
#include <QShortcut>
#include <QTextBrowser>
#include <QTimer>

#include "globals.h"
#include "analysis/codeAssistant.h"
#include "core/fileModule.h"
#include "document/module/scintillaWidget.h"
#include "document/page/imagePage.h"
#include "document/page/luaPage.h"
#include "document/page/textPage.h"
#include "document/page/welcomePage.h"

// public
DocumentModule::DocumentModule(QWidget *parent)
    : QObject(parent),
      m_documentConfig(g_workspaceConfig["documentConfig"].toObject()),
      m_welcomePage(new WelcomePage()),
      m_codeAssistant(new CodeAssistant(parent)) {
    qApp->installEventFilter(m_codeAssistant);
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
    qDebug() << QString("[%1] document module destructed").arg(timestamp);
}

void DocumentModule::propertySet(const QVariantMap &objects) {
    m_messageDialog = qvariant_cast<QObject *>(objects["mainWindowMessageDialog"]);
    m_toolTip = qvariant_cast<QObject *>(objects["mainWindowToolTip"]);
    m_breakpointEditDialog = qvariant_cast<QObject *>(objects["breakpointModuleEditDialog"]);
    m_systemPropertyDialog = qvariant_cast<QObject *>(objects["fileModulePropertyDialog"]);
    m_saveDialog = qvariant_cast<QObject *>(objects["documentModuleSaveDialog"]);
    m_editorMenu = qvariant_cast<QObject *>(objects["documentModuleEditorMenu"]);

    for (const auto &value: m_documentConfig["documentList"].toArray()) {
        documentOpen(QUrl(value.toString()));
    }
    const auto focusedUrl = QUrl(m_documentConfig["documentFocused"].toString());
    if (!focusedUrl.isEmpty() && m_pageHash.contains(focusedUrl)) {
        QTimer::singleShot(0, this, [this, focusedUrl] {
            m_pageHash[focusedUrl]->setFocus(Qt::FocusReason::MouseFocusReason);
        });
    }

    m_welcomePage->propertySet(QVariantMap());
    m_codeAssistant->propertySet(objects);
    m_codeAssistant->fontSet(m_documentConfig["fontFamily"].toString(), m_documentConfig["fontSize"].toInt());
}

void DocumentModule::documentConfigSave() {
    m_quit = true;
    // save config
    auto documentList = QJsonArray();
    for (const auto &url: m_pageHash.keys()) {
        if (auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[url])) {
            luaPage->documentSave();
            documentList.append(url.toString());
        }
        documentList.append(url.toString());
    }
    m_documentConfig["documentList"] = documentList;
    if (m_focusedUrl.isEmpty()) {
        m_documentConfig["documentFocused"] = "";
    } else {
        m_documentConfig["documentFocused"] = m_focusedUrl.toString();
    }
    g_workspaceConfig["documentConfig"] = m_documentConfig;
}

void DocumentModule::scriptFontReload(const QJsonObject &fontConfigScript) const {
    // const auto scriptFont = QFont(fontConfigScript["fontFamily"].toString(), fontConfigScript["fontSize"].toInt());
    // for (const auto &luaPage: m_pageHash) {
    //     luaPage->m_editorWidget->setFont(scriptFont);
    // }
}

void DocumentModule::scriptFontSave(const QJsonObject &fontConfigScript) {
    m_documentConfig["fontFamily"] = fontConfigScript["fontFamily"].toString();
    m_documentConfig["fontSize"] = fontConfigScript["fontSize"].toInt();
}

void DocumentModule::scriptIndicatorReload(const QJsonObject &indicatorConfigScript) const {
    // for (const auto &luaPage: m_pageHash) {
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
    m_documentConfig["indicatorErrorStyle"] = indicatorConfigScript["indicatorErrorStyle"].toInt();
    m_documentConfig["indicatorErrorColor"] = indicatorConfigScript["indicatorErrorColor"].toString();
    m_documentConfig["indicatorWarningStyle"] = indicatorConfigScript["indicatorWarningStyle"].toInt();
    m_documentConfig["indicatorWarningColor"] = indicatorConfigScript["indicatorWarningColor"].toString();
    m_documentConfig["indicatorInfoStyle"] = indicatorConfigScript["indicatorInfoStyle"].toInt();
    m_documentConfig["indicatorInfoColor"] = indicatorConfigScript["indicatorInfoColor"].toString();
    m_documentConfig["indicatorHintStyle"] = indicatorConfigScript["indicatorHintStyle"].toInt();
    m_documentConfig["indicatorHintColor"] = indicatorConfigScript["indicatorHintColor"].toString();
    // highlight
    m_documentConfig["indicatorHighlightStyle"] = indicatorConfigScript["indicatorHighlightStyle"].toInt();
    m_documentConfig["indicatorHighlightColor"] = indicatorConfigScript["indicatorHighlightColor"].toString();
    m_documentConfig["indicatorReadStyle"] = indicatorConfigScript["indicatorReadStyle"].toInt();
    m_documentConfig["indicatorReadColor"] = indicatorConfigScript["indicatorReadColor"].toString();
    m_documentConfig["indicatorWriteStyle"] = indicatorConfigScript["indicatorWriteStyle"].toInt();
    m_documentConfig["indicatorWriteColor"] = indicatorConfigScript["indicatorWriteColor"].toString();
    // search
    m_documentConfig["indicatorSearchStyle"] = indicatorConfigScript["indicatorSearchStyle"].toInt();
    m_documentConfig["indicatorSearchColor"] = indicatorConfigScript["indicatorSearchColor"].toString();
    m_documentConfig["indicatorSelectionStyle"] = indicatorConfigScript["indicatorSelectionStyle"].toInt();
    m_documentConfig["indicatorSelectionColor"] = indicatorConfigScript["indicatorSelectionColor"].toString();
    // hyperlink
    m_documentConfig["indicatorHyperlinkStyle"] = indicatorConfigScript["indicatorHyperlinkStyle"].toInt();
    m_documentConfig["indicatorHyperlinkColor"] = indicatorConfigScript["indicatorHyperlinkColor"].toString();
}

void DocumentModule::scriptMarkerReload(const QJsonObject &markerConfigScript) const {
    // for (const auto &luaPage: m_pageHash) {
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
    m_documentConfig["markerBreakpointStyle"] = markerConfigScript["markerBreakpointStyle"].toInt();
    m_documentConfig["markerBreakpointBackground"] = markerConfigScript["markerBreakpointBackground"].toString();
    m_documentConfig["markerBreakpointForeground"] = markerConfigScript["markerBreakpointForeground"].toString();
    m_documentConfig["markerDebugStyle"] = markerConfigScript["markerDebugStyle"].toInt();
    m_documentConfig["markerDebugBackground"] = markerConfigScript["markerDebugBackground"].toString();
    m_documentConfig["markerDebugForeground"] = markerConfigScript["markerDebugForeground"].toString();
}

// public: file
void DocumentModule::documentOpen(const QUrl &documentUrl) {
    // open page
    if (!m_pageHash.contains(documentUrl)) {
        const auto documentPath = documentUrl.toLocalFile();
        const QFileInfo documentInfo(documentPath);
        const auto suffix = documentInfo.suffix();
        BasePage *newPage{};
        const QStringList imageType = {"bmp", "gif", "ico", "jpeg", "jpg", "png", "svg", "tif", "tiff", "webp"};
        if (imageType.contains(suffix)) {
            newPage = new ImagePage(m_documentConfig, documentUrl);
            auto *imagePage = qobject_cast<ImagePage *>(newPage);
            imagePage->propertySet(QVariantMap{
            });
        } else if (suffix == "lua") {
            newPage = new LuaPage(m_documentConfig, documentUrl);
            auto *luaPage = qobject_cast<LuaPage *>(newPage);
            luaPage->propertySet(QVariantMap{
                {"mainWindowToolTip", QVariant::fromValue(m_toolTip)},
                {"breakpointModuleEditDialog", QVariant::fromValue(m_breakpointEditDialog)},
                {"fileModulePropertyDialog", QVariant::fromValue(m_systemPropertyDialog)},
                {"documentModuleSaveDialog", QVariant::fromValue(m_saveDialog)},
                {"documentModuleEditorMenu", QVariant::fromValue(m_editorMenu)}
            });
            connect(luaPage, &LuaPage::isFocusedChanged, this, [this, luaPage](const bool status) { documentFocus(luaPage, status); });
            connect(luaPage, &LuaPage::appendLog, this, &DocumentModule::appendLog);
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
            luaPage->diagnosticsNotification(m_diagnosticsHash[documentUrl]);
        } else {
            newPage = new TextPage(m_documentConfig, documentUrl);
            auto *textPage = qobject_cast<TextPage *>(newPage);
            textPage->propertySet(QVariantMap{
                {"mainWindowToolTip", QVariant::fromValue(m_toolTip)},
                {"fileModulePropertyDialog", QVariant::fromValue(m_systemPropertyDialog)}
            });
        }
        // path disambiguation
        bool conflict = false;
        for (const auto &url: m_pageHash.keys()) {
            if (url.fileName() == documentUrl.fileName()) {
                conflict = true;
                auto *conflictPage = m_pageHash[url];
                conflictPage->pathDisambiguation();
            }
        }
        if (conflict) {
            newPage->pathDisambiguation();
        }
        m_pageHash[documentUrl] = newPage;
        if (m_focusedUrl.isEmpty()) {
            m_welcomePage->open();
            m_welcomePage->addDockWidgetAsTab(newPage);
            m_welcomePage->close();
        } else {
            m_pageHash[m_focusedUrl]->addDockWidgetAsTab(newPage);
        }
        connect(newPage, &BasePage::destroyed, this, [this, documentUrl] { documentClose(documentUrl); });
    }
    m_pageHash[documentUrl]->raise();
    m_pageHash[documentUrl]->setFocus(Qt::FocusReason::MouseFocusReason);
}

void DocumentModule::permissionSet(const QUrl &documentUrl) {
    if (m_pageHash.contains(documentUrl)) {
        m_pageHash[documentUrl]->permissionGet();
    }
}

void DocumentModule::documentSave(const QUrl &documentUrl) {
    // TODO: text page
    if (auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->documentSave();
    }
}

QVariantHash DocumentModule::menuGet(const QString &name) {
    // TODO: text page
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[m_focusedUrl])) {
        return luaPage->menuGet(name);
    }
    return {};
}

void DocumentModule::menuRequest(const QString &request) {
    // TODO: text page
    if (auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[m_focusedUrl])) {
        luaPage->menuRequest(request);
    }
}

int DocumentModule::eolModeGet(const QUrl &documentUrl) const {
    // TODO: text page
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        return luaPage->m_editorWidget->eolModeGet();
    }
    return {};
}

void DocumentModule::eolModeSet(const QUrl &documentUrl, const int eolMode) const {
    // TODO: text page
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->m_editorWidget->eolModeSet(eolMode);
    }
}

bool DocumentModule::eolViewGet(const QUrl &documentUrl) {
    // TODO: text page
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        return luaPage->m_editorWidget->eolViewGet();
    }
    return {};
}

void DocumentModule::eolViewSet(const QUrl &documentUrl, const bool status) const {
    // TODO: text page
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->m_editorWidget->eolViewSet(status);
    }
}

// public: document
void DocumentModule::foldContractTop(const QUrl &documentUrl) {
    if (!m_pageHash.contains(documentUrl)) documentOpen(documentUrl);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->m_editorWidget->foldContractTop();
    }
}

void DocumentModule::foldContractRecursively(const QUrl &documentUrl) {
    if (!m_pageHash.contains(documentUrl)) documentOpen(documentUrl);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->m_editorWidget->foldContractRecursively();
    }
}

void DocumentModule::foldExpandRecursively(const QUrl &documentUrl) {
    if (!m_pageHash.contains(documentUrl)) documentOpen(documentUrl);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->m_editorWidget->foldExpandRecursively();
    }
}

void DocumentModule::assemblyToggle(const QUrl &documentUrl, const bool status) {
    if (auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->assemblyToggle(status);
    }
}

void DocumentModule::focusSet(const QUrl &documentUrl, const bool status) {
    // TODO: text page
    if (!m_pageHash.contains(documentUrl)) documentOpen(documentUrl);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->m_editorWidget->focusSet(status);
    }
}

void DocumentModule::indexSet(const QUrl &documentUrl, const int line, const int character) {
    // TODO: text page
    if (!m_pageHash.contains(documentUrl)) documentOpen(documentUrl);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->m_editorWidget->indexSet(line, character);
    }
}

void DocumentModule::indexGet() const {
    // TODO: text page
    QHash<QString, int> index{};
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[m_focusedUrl])) {
        index = luaPage->m_editorWidget->indexGet();
        g_cursorPosition = {
            {"url", m_focusedUrl},
            {"line", index["line"]},
            {"character", index["character"]}
        };
    }
}

QString DocumentModule::textGet(const QUrl &documentUrl, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
    // TODO: text page
    if (m_pageHash.contains(documentUrl)) {
        if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
            return luaPage->m_editorWidget->textGet(startLine, startCharacter, endLine, endCharacter);
        }
    }
    return FileModule::textGet(documentUrl, startLine, startCharacter, endLine, endCharacter);
}

void DocumentModule::indicatorFill(const QUrl &documentUrl, const int type, const int startLine, const int startCharacter, const int endLine, const int endCharacter,
                                   const int time) {
    if (!m_pageHash.contains(documentUrl)) documentOpen(documentUrl);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->m_editorWidget->indicatorFill(type, startLine, startCharacter, endLine, endCharacter, time);
    }
}

void DocumentModule::indicatorClear(const QUrl &documentUrl, const int type, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
    if (!m_pageHash.contains(documentUrl)) return;
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->m_editorWidget->indicatorClear(type, startLine, startCharacter, endLine, endCharacter);
    }
}

void DocumentModule::markerAdd(const QUrl &documentUrl, const int type, const int line, const int time) {
    if (!m_pageHash.contains(documentUrl)) documentOpen(documentUrl);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->m_editorWidget->markerAdd(type, line, time);
    }
}

void DocumentModule::markerDelete(const QUrl &documentUrl, const int type, const int line) {
    if (!m_pageHash.contains(documentUrl)) return;
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->m_editorWidget->markerDelete(type, line);
    }
}

// public: lsp
void DocumentModule::diagnosticsNotification(const QUrl &documentUrl, const QJsonArray &diagnostics) {
    m_diagnosticsHash.insert(documentUrl, diagnostics);
    if (m_pageHash.contains(documentUrl)) {
        if (auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
            luaPage->diagnosticsNotification(diagnostics);
        }
    }
}

void DocumentModule::codeActionRequest(const QUrl &documentUrl, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
    QJsonArray diagnosticArray{};
    for (const auto &value: m_diagnosticsHash[documentUrl]) {
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
                {"uri", documentUrl.toString()}
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

void DocumentModule::completionRequest(const QUrl &documentUrl, int line, int character) {
    // completion request to lua language server
    const QJsonObject completionParams{
        {
            "textDocument", QJsonObject{
                {"uri", documentUrl.toString()}
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

void DocumentModule::completionResponse(const QUrl &documentUrl, const QJsonArray &items) const {
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
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
            {"documentUrl", documentUrl},
            {"position", position},
            {"startLine", startLine},
            {"startCharacter", startCharacter},
            {"endLine", endLine},
            {"endCharacter", endCharacter},
            {"typed", typed}
        };
        m_codeAssistant->completionShow(completionSession, items);
    }
}

void DocumentModule::definitionRequest(const QUrl &documentUrl, const int line, const int character) {
    // definition request to lua language server
    const QJsonObject definitionParams{
        {
            "textDocument", QJsonObject{
                {"uri", documentUrl.toString()}
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

void DocumentModule::definitionResponse(const QUrl &documentUrl, const QJsonArray &definitions) const {
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
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
            {"documentUrl", documentUrl},
            {"position", position}
        };
        m_codeAssistant->navigationShow(navigationSession, definitions);
    }
}

void DocumentModule::documentSymbolRequest(const QUrl &documentUrl) {
    // document symbol request to lua language server
    const QJsonObject documentSymbolParams{
        {
            "textDocument", QJsonObject{
                {"uri", documentUrl.toString()}
            }
        }
    };
    emit requestJson("textDocument/documentSymbol", documentSymbolParams);
}

void DocumentModule::documentSymbolResponse(const QUrl &documentUrl, const QJsonArray &result) {
    if (auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->documentSymbolResponse(result);
    }
}

void DocumentModule::documentHighlightRequest(const QUrl &documentUrl, const int line, const int character) {
    // document highlight request to lua language server
    const QJsonObject documentHighlightParams{
        {
            "textDocument", QJsonObject{
                {"uri", documentUrl.toString()}
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

void DocumentModule::documentHighlightResponse(const QUrl &documentUrl, const QJsonArray &result) {
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->documentHighlightResponse(result);
    }
}

void DocumentModule::foldingRangeRequest(const QUrl &documentUrl) {
    // folding range request to lua language server
    const QJsonObject foldingRangeParams{
        {
            "textDocument", QJsonObject{
                {"uri", documentUrl.toString()}
            }
        }
    };
    emit requestJson("textDocument/foldingRange", foldingRangeParams);
}

void DocumentModule::foldingRangeResponse(const QUrl &documentUrl, const QJsonArray &result) const {
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->foldingRangeResponse(result);
    }
}

void DocumentModule::formattingRequest(const QUrl &documentUrl) {
    // formatting request to lua language server
    const QJsonObject formattingParams{
        {
            "textDocument", QJsonObject{
                {"uri", documentUrl.toString()}
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

void DocumentModule::formattingResponse(const QUrl &documentUrl, const QString &newText) const {
    if (newText.isEmpty()) {
        m_messageDialog->setProperty("title", tr("Information"));
        m_messageDialog->setProperty("text", tr("File already reformatted."));
        QMetaObject::invokeMethod(m_messageDialog, "open");
    } else if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->formattingResponse(newText);
    }
}

void DocumentModule::hoverRequest(const QUrl &documentUrl, int line, int character) {
    // hover request to lua language server
    const QJsonObject hoverParams{
        {
            "textDocument", QJsonObject{
                {"uri", documentUrl.toString()}
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

void DocumentModule::hoverResponse(const QUrl &documentUrl, const QString &message) const {
    const QPoint position = QCursor::pos() + QPoint(10, 10);
    // call hover show
    const QVariantHash hoverSession = {
        {"position", position}
    };
    m_codeAssistant->hoverShow(hoverSession, message);
}

void DocumentModule::implementationRequest(const QUrl &documentUrl, const int line, const int character) {
    // implementation request to lua language server
    const QJsonObject implementationParams{
        {
            "textDocument", QJsonObject{
                {"uri", documentUrl.toString()}
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

void DocumentModule::implementationResponse(const QUrl &documentUrl, const QJsonArray &implementations) const {
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
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
            {"documentUrl", documentUrl},
            {"position", position}
        };
        m_codeAssistant->navigationShow(navigationSession, implementations);
    }
}

void DocumentModule::onTypeFormattingRequest(const QUrl &documentUrl, int line, int character) {
    // on type formatting request to lua language server
    const QJsonObject onTypeFormattingParams{
        {
            "textDocument", QJsonObject{
                {"uri", documentUrl.toString()}
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

void DocumentModule::onTypeFormattingResponse(const QUrl &documentUrl, const QJsonObject &newText) const {
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->onTypeFormattingResponse(newText);
    }
}

void DocumentModule::rangeFormattingRequest(const QUrl &documentUrl, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
    // rangeFormatting request to lua language server
    const QJsonObject rangeFormattingParams{
        {
            "textDocument", QJsonObject{
                {"uri", documentUrl.toString()}
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

void DocumentModule::rangeFormattingResponse(const QUrl &documentUrl, const QString &newText) const {
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        auto text = newText;
        if (text.endsWith("\r\n")) text.chop(2);
        luaPage->rangeFormattingResponse(text);
    }
}

void DocumentModule::referencesRequest(const QUrl &documentUrl, int line, int character) {
    // references request to lua language server
    const QJsonObject referencesParams{
        {
            "textDocument", QJsonObject{
                {"uri", documentUrl.toString()}
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

void DocumentModule::referencesResponse(const QUrl &documentUrl, const QJsonArray &references) const {
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
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
            {"documentUrl", documentUrl},
            {"position", position}
        };
        m_codeAssistant->navigationShow(navigationSession, references);
    }
}

void DocumentModule::semanticTokensRequest(const QUrl &documentUrl) {
    // semantic tokens request to lua language server
    const QJsonObject semanticTokensParams{
        {
            "textDocument", QJsonObject{
                {"uri", documentUrl.toString()}
            }
        }
    };
    emit requestJson("textDocument/semanticTokens/full", semanticTokensParams);
}

void DocumentModule::semanticTokensResponse(const QUrl &documentUrl, const QJsonArray &data) const {
    if (auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->semanticTokensResponse(data);
    }
}

void DocumentModule::signatureHelpRequest(const QUrl &documentUrl, int line, int character) {
    // signature help request to lua language server
    const QJsonObject signatureHelpParams{
        {
            "textDocument", QJsonObject{
                {"uri", documentUrl.toString()}
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

void DocumentModule::signatureHelpResponse(const QUrl &documentUrl, const QJsonArray &signatures) const {
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
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
            {"documentUrl", documentUrl},
            {"position", position}
        };
        m_codeAssistant->signatureShow(signatureSession, signatures);
    }
}

void DocumentModule::typeDefinitionRequest(const QUrl &documentUrl, const int line, const int character) {
    // type definition request to lua language server
    const QJsonObject typeDefinitionParams{
        {
            "textDocument", QJsonObject{
                {"uri", documentUrl.toString()}
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

void DocumentModule::typeDefinitionResponse(const QUrl &documentUrl, const QJsonArray &typeDefinitions) const {
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
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
            {"documentUrl", documentUrl},
            {"position", position}
        };
        m_codeAssistant->navigationShow(navigationSession, typeDefinitions);
    }
}

// public: typo
void DocumentModule::spellCheckResponse(const QUrl &documentUrl, const QVariantList &typos) {
    if (!m_pageHash.contains(documentUrl)) documentOpen(documentUrl);
    if (auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->spellCheckResponse(typos);
    }
}

// private
void DocumentModule::documentFocus(BasePage *basePage, const bool status) {
    if (status) {
        m_focusedUrl = basePage->documentUrl();
        if (const auto *luaPage = qobject_cast<LuaPage *>(basePage)) {
            const QVariantHash session = {
                {"codePage", luaPage->m_editorWidget->codePageGet()},
                {"eolMode", luaPage->m_editorWidget->eolModeGet()}
            };
            emit focusDocument(m_focusedUrl, session);
        }
    } else if (const auto *luaPage = qobject_cast<LuaPage *>(basePage)) {
        luaPage->m_editorWidget->indicatorClear(INDICATOR_HIGHLIGHT);
        luaPage->m_editorWidget->indicatorClear(INDICATOR_READ);
        luaPage->m_editorWidget->indicatorClear(INDICATOR_WRITE);
    }
}

void DocumentModule::documentClose(const QUrl &documentUrl) {
    if (m_quit) return;
    m_pageHash.remove(documentUrl);
    if (m_pageHash.isEmpty()) {
        // m_welcomePage->open();
        m_focusedUrl = nullptr;
    } else {
        const auto begin = m_pageHash.begin();
        m_focusedUrl = begin.key();
    }
}

void DocumentModule::charAdd(const QUrl &documentUrl, const QChar character) const {
    if (auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->charAdd(character.toLatin1());
    }
}

void DocumentModule::textSet(const QUrl &documentUrl, const QString &text, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
    if (!m_pageHash.contains(documentUrl)) documentOpen(documentUrl);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->m_editorWidget->textSet(text, startLine, startCharacter, endLine, endCharacter);
    }
}

void DocumentModule::textSetSelected(const QUrl &documentUrl, const QString &text) {
    if (!m_pageHash.contains(documentUrl)) documentOpen(documentUrl);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash[documentUrl])) {
        luaPage->m_editorWidget->textSetSelected(text);
    }
}
