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
      m_config(g_workspaceConfig["documentConfig"].toObject()),
      m_welcomePage(new WelcomePage()),
      m_codeAssistant(new CodeAssistant(parent)) {
    m_navigationHistory = QVariantHash{
        {"index", -1},
        {"list", QVariantList{}}
    };
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
    connect(m_codeAssistant, &CodeAssistant::recordNavigation, this, &DocumentModule::navigationRecord);
}

DocumentModule::~DocumentModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] document module destructed").arg(timestamp);
}

void DocumentModule::propertySet(const QVariantHash &objects) {
    m_global = qvariant_cast<QObject *>(objects["global"]);
    m_messageDialog = qvariant_cast<QObject *>(objects["mainWindowMessageDialog"]);
    m_toolTip = qvariant_cast<QObject *>(objects["mainWindowToolTip"]);
    m_breakpointEditDialog = qvariant_cast<QObject *>(objects["breakpointModuleEditDialog"]);
    m_systemPropertyDialog = qvariant_cast<QObject *>(objects["fileModulePropertyDialog"]);
    m_gotoDialog = qvariant_cast<QObject *>(objects["documentModuleGotoDialog"]);
    m_saveDialog = qvariant_cast<QObject *>(objects["documentModuleSaveDialog"]);
    m_editorMenu = qvariant_cast<QObject *>(objects["documentModuleEditorMenu"]);

    for (const auto &value: m_config["documentList"].toArray()) {
        documentOpen(QUrl(value.toString()));
    }
    const auto focusedUrl = QUrl(m_config["documentFocused"].toString());
    if (!focusedUrl.isEmpty() && m_pageHash.contains(focusedUrl)) {
        QTimer::singleShot(0, this, [this, focusedUrl] {
            m_pageHash[focusedUrl]->setFocus(Qt::FocusReason::MouseFocusReason);
        });
    }

    m_welcomePage->propertySet(QVariantHash{
        {"global", QVariant::fromValue(m_global)}
    });
    m_codeAssistant->propertySet(objects);
    m_codeAssistant->fontSet(m_config["fontFamily"].toString(), m_config["fontSize"].toInt());
}

void DocumentModule::documentConfigSave() {
    // save config
    QJsonArray documentList{};
    for (const auto &url: m_pageHash.keys()) {
        documentSave(url);
        documentList.append(url.toString());
    }
    m_config["documentList"] = documentList;
    if (m_focusedUrl.isEmpty()) {
        m_config["documentFocused"] = "";
    } else {
        m_config["documentFocused"] = m_focusedUrl.toString();
    }
    g_workspaceConfig["documentConfig"] = m_config;
}

void DocumentModule::scriptFontReload(const QJsonObject &fontConfigScript) const {
    // const auto scriptFont = QFont(fontConfigScript["fontFamily"].toString(), fontConfigScript["fontSize"].toInt());
    // for (const auto &luaPage: m_pageHash) {
    //     luaPage->m_editorWidget->setFont(scriptFont);
    // }
}

void DocumentModule::scriptFontSave(const QJsonObject &fontConfigScript) {
    m_config["fontFamily"] = fontConfigScript["fontFamily"].toString();
    m_config["fontSize"] = fontConfigScript["fontSize"].toInt();
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
            newPage = new ImagePage(m_config, documentUrl);
            auto *imagePage = qobject_cast<ImagePage *>(newPage);
            imagePage->propertySet(QVariantHash{
            });
        } else if (suffix == "lua") {
            newPage = new LuaPage(m_config, documentUrl);
            auto *luaPage = qobject_cast<LuaPage *>(newPage);
            luaPage->propertySet(QVariantHash{
                {"global", QVariant::fromValue(m_global)},
                {"mainWindowToolTip", QVariant::fromValue(m_toolTip)},
                {"breakpointModuleEditDialog", QVariant::fromValue(m_breakpointEditDialog)},
                {"fileModulePropertyDialog", QVariant::fromValue(m_systemPropertyDialog)},
                {"documentModuleGotoDialog", QVariant::fromValue(m_gotoDialog)},
                {"documentModuleSaveDialog", QVariant::fromValue(m_saveDialog)},
                {"documentModuleEditorMenu", QVariant::fromValue(m_editorMenu)}
            });
            connect(luaPage, &LuaPage::isFocusedChanged, this, [this, luaPage](const bool status) { documentFocus(luaPage, status); });
            connect(luaPage, &LuaPage::appendLog, this, &DocumentModule::appendLog);
            connect(luaPage, &LuaPage::startThread, this, &DocumentModule::startThread);
            connect(luaPage, &LuaPage::changeSelection, this, &DocumentModule::changeSelection);
            connect(luaPage, &LuaPage::insertBreakpoint, this, &DocumentModule::insertBreakpoint);
            connect(luaPage, &LuaPage::removeBreakpoint, this, &DocumentModule::removeBreakpoint);
            connect(luaPage, &LuaPage::notificationJson, this, &DocumentModule::notificationJson);
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
            connect(luaPage, &LuaPage::showDiagnostic, m_codeAssistant, &CodeAssistant::diagnosticShow);
            luaPage->diagnosticsNotification(m_diagnosticsHash[documentUrl]);
        } else {
            newPage = new TextPage(m_config, documentUrl);
            auto *textPage = qobject_cast<TextPage *>(newPage);
            textPage->propertySet(QVariantHash{
                {"global", QVariant::fromValue(m_global)},
                {"mainWindowToolTip", QVariant::fromValue(m_toolTip)},
                {"fileModulePropertyDialog", QVariant::fromValue(m_systemPropertyDialog)},
                {"documentModuleGotoDialog", QVariant::fromValue(m_gotoDialog)},
                {"documentModuleSaveDialog", QVariant::fromValue(m_saveDialog)},
            });
            connect(textPage, &TextPage::isFocusedChanged, this, [this, textPage](const bool status) { documentFocus(textPage, status); });
            connect(textPage, &TextPage::appendLog, this, &DocumentModule::appendLog);
            connect(textPage, &TextPage::changeSelection, this, &DocumentModule::changeSelection);
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
        connect(newPage, &BasePage::closeDocument, this, &DocumentModule::documentClose);
        emit appendLog(LogLevel::Info, "document opened", QString("<a href='%1'>%2</a>").arg(documentUrl.toString(), documentUrl.toString()));
    }
    m_pageHash[documentUrl]->raise();
    m_pageHash[documentUrl]->setFocus(Qt::FocusReason::MouseFocusReason);
}

QSet<QString> DocumentModule::documentList() const {
    QSet<QString> keys{};
    for (const auto &url: m_pageHash.keys()) {
        keys.insert(url.toString());
    }
    return keys;
}

QString DocumentModule::documentFocused() const {
    return m_focusedUrl.toString();
}

void DocumentModule::documentSave(const QUrl &documentUrl) const {
    if (m_pageHash.contains(documentUrl)) m_pageHash.value(documentUrl)->documentSave();
}

void DocumentModule::permissionSet(const QUrl &documentUrl) const {
    if (m_pageHash.contains(documentUrl)) m_pageHash.value(documentUrl)->permissionGet();
}

QVariantHash DocumentModule::menuGet(const QString &name) {
    // TODO: text page
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(m_focusedUrl))) {
        if (name == "nav") {
            auto menuSession = luaPage->menuGet(name);
            menuSession.insert("prev", m_navigationHistory["index"].toInt() > 0);
            menuSession.insert("next", m_navigationHistory["index"].toInt() < m_navigationHistory["list"].toList().size() - 1);
            // qDebug() << menuSession["documentUrl"] << menuSession["line"] << menuSession["character"] << menuSession["navigation"];
            return menuSession;
        }
        return luaPage->menuGet(name);
    }
    return {};
}

void DocumentModule::menuRequest(const QString &request) const {
    // TODO: text page
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(m_focusedUrl))) luaPage->menuRequest(request);
}

int DocumentModule::eolModeGet(const QUrl &documentUrl) const {
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) return luaPage->handler()->eolModeGet();
    if (const auto *textPage = qobject_cast<TextPage *>(m_pageHash.value(documentUrl))) return textPage->handler()->eolModeGet();
    return {};
}

void DocumentModule::eolModeSet(const QUrl &documentUrl, const int eolMode) const {
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) luaPage->handler()->eolModeSet(eolMode);
    else if (const auto *textPage = qobject_cast<TextPage *>(m_pageHash.value(documentUrl))) textPage->handler()->eolModeSet(eolMode);
}

bool DocumentModule::eolViewGet(const QUrl &documentUrl) const {
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) return luaPage->handler()->eolViewGet();
    if (const auto *textPage = qobject_cast<TextPage *>(m_pageHash.value(documentUrl))) return textPage->handler()->eolViewGet();
    return {};
}

void DocumentModule::eolViewSet(const QUrl &documentUrl, const bool status) const {
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) luaPage->handler()->eolViewSet(status);
    else if (const auto *textPage = qobject_cast<TextPage *>(m_pageHash.value(documentUrl))) textPage->handler()->eolViewSet(status);
}

// public: document
void DocumentModule::foldContractTop(const QUrl &documentUrl) {
    if (!m_pageHash.contains(documentUrl)) documentOpen(documentUrl);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) luaPage->handler()->foldContractTop();
}

void DocumentModule::foldContractRecursively(const QUrl &documentUrl) {
    if (!m_pageHash.contains(documentUrl)) documentOpen(documentUrl);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) luaPage->handler()->foldContractRecursively();
}

void DocumentModule::foldExpandRecursively(const QUrl &documentUrl) {
    if (!m_pageHash.contains(documentUrl)) documentOpen(documentUrl);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) luaPage->handler()->foldExpandRecursively();
}

void DocumentModule::navigationPrev() {
    const auto index = m_navigationHistory["index"].toInt() - 1;
    if (index < 0) return;
    m_navigationHistory["index"] = index;
    const QVariantHash navigationSession = m_navigationHistory["list"].toList()[index].toHash();
    indexSet(navigationSession["documentUrl"].toUrl(), navigationSession["line"].toInt(), navigationSession["character"].toInt());
}

void DocumentModule::navigationNext() {
    const auto index = m_navigationHistory["index"].toInt() + 1;
    if (index >= m_navigationHistory["list"].toList().size()) return;
    m_navigationHistory["index"] = index;
    const QVariantHash navigationSession = m_navigationHistory["list"].toList()[index].toHash();
    indexSet(navigationSession["documentUrl"].toUrl(), navigationSession["line"].toInt(), navigationSession["character"].toInt());
}

void DocumentModule::assemblyToggle(const QUrl &documentUrl, const bool status) {
    if (auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) {
        // luaPage->assemblyToggle(status);
    }
}

void DocumentModule::focusSet(const QUrl &documentUrl, const bool status) {
    if (!m_pageHash.contains(documentUrl)) documentOpen(documentUrl);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) luaPage->handler()->focusSet(status);
    else if (const auto *textPage = qobject_cast<TextPage *>(m_pageHash.value(documentUrl))) textPage->handler()->focusSet(status);
}

void DocumentModule::indexSet(const QUrl &documentUrl, const int line, const int character) {
    if (documentUrl != m_focusedUrl) documentOpen(documentUrl);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) luaPage->handler()->indexSet(line, character);
    else if (const auto *textPage = qobject_cast<TextPage *>(m_pageHash.value(documentUrl))) textPage->handler()->indexSet(line, character);
}

void DocumentModule::indexGet() const {
    QHash<QString, int> index{};
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(m_focusedUrl))) {
        index = luaPage->handler()->indexGet();
        g_cursorPosition = {
            {"url", m_focusedUrl},
            {"line", index["line"]},
            {"character", index["character"]}
        };
    }
}

QString DocumentModule::textGet(const QUrl &documentUrl, const int startLine, const int startCharacter, const int endLine, const int endCharacter) const {
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) return luaPage->handler()->textGet(startLine, startCharacter, endLine, endCharacter);
    return FileModule::textGet(documentUrl, startLine, startCharacter, endLine, endCharacter);
}

void DocumentModule::textSet(const QUrl &documentUrl, const QString &text, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
    if (!m_pageHash.contains(documentUrl)) documentOpen(documentUrl);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) {
        luaPage->handler()->textSet(text, startLine, startCharacter, endLine, endCharacter);
    }
}

void DocumentModule::indicatorFill(const QUrl &documentUrl, const int type, const int startLine, const int startCharacter, const int endLine, const int endCharacter,
                                   const int time) {
    if (!m_pageHash.contains(documentUrl)) documentOpen(documentUrl);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) luaPage->handler()->indicatorFill(
        type, startLine, startCharacter, endLine, endCharacter, time);
}

void DocumentModule::indicatorClear(const QUrl &documentUrl, const int type, const int startLine, const int startCharacter, const int endLine, const int endCharacter) const {
    if (!m_pageHash.contains(documentUrl)) return;
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) luaPage->handler()->indicatorClear(type, startLine, startCharacter, endLine, endCharacter);
}

void DocumentModule::markerAdd(const QUrl &documentUrl, const int type, const int line, const int time) {
    if (!m_pageHash.contains(documentUrl)) documentOpen(documentUrl);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) luaPage->handler()->markerAdd(type, line, time);
}

void DocumentModule::markerDelete(const QUrl &documentUrl, const int type, const int line) const {
    if (!m_pageHash.contains(documentUrl)) return;
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) luaPage->handler()->markerDelete(type, line);
}

QJsonArray DocumentModule::diagnosticsGet(const QUrl &documentUrl) const {
    return m_diagnosticsHash.value(documentUrl);
}

QJsonArray DocumentModule::symbolGet(const QUrl& documentUrl) const {
    return m_symbolHash.value(documentUrl);
}

// public: lsp
void DocumentModule::diagnosticsNotification(const QUrl &documentUrl, const QJsonArray &diagnostics) {
    m_diagnosticsHash.insert(documentUrl, diagnostics);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) luaPage->diagnosticsNotification(diagnostics);
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
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) {
        const auto *scintilla = luaPage->handler();
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
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) {
        const auto *scintilla = luaPage->handler();
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
    m_symbolHash.insert(documentUrl, result);
    if (auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) luaPage->documentSymbolResponse(result);
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
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) luaPage->documentHighlightResponse(result);
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
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) {
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
    } else if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) {
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
    // call hover show
    const QVariantHash hoverSession = {
        {"position", QCursor::pos()}
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
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) {
        const auto *scintilla = luaPage->handler();
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
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) {
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
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) {
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
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) {
        const auto *scintilla = luaPage->handler();
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
    if (auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) {
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
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) {
        const auto *scintilla = luaPage->handler();
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
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) {
        const auto *scintilla = luaPage->handler();
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
    if (auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) {
        luaPage->spellCheckResponse(typos);
    }
}

// private
void DocumentModule::documentFocus(BasePage *basePage, const bool status) {
    if (const auto *luaPage = qobject_cast<LuaPage *>(basePage)) {
        if (status) {
            luaPage->handler()->focusSet(true);
            m_focusedUrl = basePage->documentUrl();
            const QVariantHash session = {
                {"codePage", luaPage->handler()->codePageGet()},
                {"eolMode", luaPage->handler()->eolModeGet()}
            };
            emit focusDocument(m_focusedUrl, session);
        } else {
            luaPage->handler()->indicatorClear(ScintillaIndicator::Highlight);
            luaPage->handler()->indicatorClear(ScintillaIndicator::Read);
            luaPage->handler()->indicatorClear(ScintillaIndicator::Write);
        }
    } else if (const auto *textPage = qobject_cast<TextPage *>(basePage)) {
        if (status) {
            textPage->handler()->focusSet(true);
            m_focusedUrl = basePage->documentUrl();
            const QVariantHash session = {
                {"codePage", textPage->handler()->codePageGet()},
                {"eolMode", textPage->handler()->eolModeGet()}
            };
            emit focusDocument(m_focusedUrl, session);
        }
    }
}

void DocumentModule::documentClose(const QUrl &documentUrl) {
    m_pageHash.remove(documentUrl);
    if (g_terminating) return;
    if (m_pageHash.isEmpty()) {
        m_welcomePage->open();
        m_focusedUrl = nullptr;
    } else {
        const auto begin = m_pageHash.begin();
        m_focusedUrl = begin.key();
    }
    emit appendLog(LogLevel::Info, "document closed", QString("<a href='%1'>%2</a>").arg(documentUrl.toString(), documentUrl.toString()));
}

void DocumentModule::charAdd(const QUrl &documentUrl, const QChar character) const {
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) {
        emit luaPage->handler()->charAdded(character.toLatin1());
    }
}

void DocumentModule::textSetSelected(const QUrl &documentUrl, const QString &text) {
    if (!m_pageHash.contains(documentUrl)) documentOpen(documentUrl);
    if (const auto *luaPage = qobject_cast<LuaPage *>(m_pageHash.value(documentUrl))) {
        luaPage->handler()->textSetSelected(text);
    }
}

void DocumentModule::navigationRecord(const QUrl &documentUrl, const int line, const int character) {
    // not at end, resize
    if (m_navigationHistory["index"].toInt() < m_navigationHistory["list"].toList().size() - 1) {
        QVariantList list = m_navigationHistory["list"].toList();
        list.resize(m_navigationHistory["index"].toInt() + 1);
        m_navigationHistory["list"] = list;
    }
    m_navigationHistory["index"] = m_navigationHistory["index"].toInt() + 1;
    // src index
    if (documentUrl.isEmpty()) {
        const auto index = qobject_cast<LuaPage *>(m_pageHash[m_focusedUrl])->handler()->indexGet();
        const auto navigationSession = QVariantHash{
            {"documentUrl", m_focusedUrl},
            {"line", index["line"]},
            {"character", index["character"]}
        };
        QVariantList list = m_navigationHistory["list"].toList();
        list.append(navigationSession);
        m_navigationHistory["list"] = list;
    }
    // dst index
    else {
        const auto navigationSession = QVariantHash{
            {"documentUrl", documentUrl},
            {"line", line},
            {"character", character}
        };
        QVariantList list = m_navigationHistory["list"].toList();
        list.append(navigationSession);
        m_navigationHistory["list"] = list;
    }
}
