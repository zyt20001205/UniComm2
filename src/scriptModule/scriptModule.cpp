#include "scriptModule/scriptModule.h"

#include <QFileInfo>
#include <QShortcut>
#include <QTextBrowser>
#include <QTimer>

#include "globals.h"
#include "luaModule/luaControl.h"
#include "portModule/portModule.h"
#include "scriptModule/scriptPage.h"
#include "scriptModule/welcomePage.h"
#include "scriptModule/codeAnalysis/codeAssistant.h"
#include "scriptModule/codeEditor/scintillaWidget.h"

// public
ScriptModule::ScriptModule(QWidget *parent)
    : QObject(parent),
      m_scriptConfig(g_workspaceConfig["scriptConfig"].toObject()),
      m_welcomePage(new WelcomePage()),
      m_codeAssistant(new CodeAssistant(parent)) {
    m_welcomePage->setObjectName("welcomePage");
    connect(m_welcomePage, &WelcomePage::openWorkspace, this, &ScriptModule::openWorkspace);
    connect(this, &ScriptModule::responseCodeAction, m_codeAssistant, &CodeAssistant::codeActionShow);
    connect(m_codeAssistant, &CodeAssistant::addChar, this, &ScriptModule::charAdd);
    connect(m_codeAssistant, &CodeAssistant::setIndex, this, &ScriptModule::indexSet);
    connect(m_codeAssistant, &CodeAssistant::getText, this, &ScriptModule::textGet);
    connect(m_codeAssistant, &CodeAssistant::setText, this, &ScriptModule::textSet);
    connect(m_codeAssistant, &CodeAssistant::insertIndicator, this, &ScriptModule::indicatorFill);
    connect(m_codeAssistant, &CodeAssistant::requestCodeAction, this, &ScriptModule::codeActionRequest);
    connect(m_codeAssistant, &CodeAssistant::insertPort, this, &ScriptModule::insertPort);
    connect(m_codeAssistant, &CodeAssistant::insertDatabase, this, &ScriptModule::insertDatabase);
    connect(m_codeAssistant, &CodeAssistant::insertDatatable, this, &ScriptModule::insertDatatable);
}

ScriptModule::~ScriptModule() {
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] script module destructed").arg(timestamp);
}

void ScriptModule::propertySet(const QVariantMap &objects) {
    for (const auto &value: m_scriptConfig["scriptList"].toArray()) {
        scriptOpen(QUrl(value.toString()));
    }
    const auto focusedUrl = QUrl(m_scriptConfig["scriptFocused"].toString());
    if (!focusedUrl.isEmpty() && m_scriptPageHash.contains(focusedUrl)) {
        QTimer::singleShot(0, this, [this, focusedUrl] {
            m_scriptPageHash[focusedUrl]->setFocus(Qt::FocusReason::MouseFocusReason);
        });
    }

    m_welcomePage->propertySet(QVariantMap());
    m_codeAssistant->propertySet(objects);
    m_codeAssistant->fontSet(m_scriptConfig["fontFamily"].toString(), m_scriptConfig["fontSize"].toInt());
    m_permissionDialog = qvariant_cast<QObject *>(objects["systemModulePermissionDialog"]);
    m_breakpointEditDialog = qvariant_cast<QObject *>(objects["breakpointModuleEditDialog"]);
    m_toolTip = qvariant_cast<QObject *>(objects["scriptModuleToolTip"]);
    m_menu = qvariant_cast<QObject *>(objects["scriptModuleEditorMenu"]);
}

void ScriptModule::scriptConfigSave() {
    // save config
    auto scriptList = QJsonArray();
    for (const auto &url: m_scriptPageHash.keys()) {
        ScriptPage *scriptPage = m_scriptPageHash[url];
        scriptPage->scriptSave();
        scriptList.append(url.toString());
    }
    m_scriptConfig["scriptList"] = scriptList;
    if (m_focusedPage == nullptr) {
        m_scriptConfig["scriptFocused"] = "";
    } else {
        m_scriptConfig["scriptFocused"] = m_focusedPage->m_scriptUrl.toString();
    }
    g_workspaceConfig["scriptConfig"] = m_scriptConfig;
}

void ScriptModule::scriptFontReload(const QJsonObject &fontConfigScript) const {
    // const auto scriptFont = QFont(fontConfigScript["fontFamily"].toString(), fontConfigScript["fontSize"].toInt());
    // for (const auto &scriptPage: m_scriptPageHash) {
    //     scriptPage->m_editorWidget->setFont(scriptFont);
    // }
}

void ScriptModule::scriptFontSave(const QJsonObject &fontConfigScript) {
    m_scriptConfig["fontFamily"] = fontConfigScript["fontFamily"].toString();
    m_scriptConfig["fontSize"] = fontConfigScript["fontSize"].toInt();
}

void ScriptModule::scriptIndicatorReload(const QJsonObject &indicatorConfigScript) const {
    // for (const auto &scriptPage: m_scriptPageHash) {
    //     // diagnostic
    //     scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorErrorStyle"].toInt()), INDICATOR_ERROR);
    //     scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorErrorColor"].toString()), INDICATOR_ERROR);
    //     scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_ERROR);
    //     scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorWarningStyle"].toInt()), INDICATOR_WARNING);
    //     scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorWarningColor"].toString()), INDICATOR_WARNING);
    //     scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_WARNING);
    //     scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorInfoStyle"].toInt()), INDICATOR_INFO);
    //     scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorInfoColor"].toString()), INDICATOR_INFO);
    //     scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_INFO);
    //     scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorHintStyle"].toInt()), INDICATOR_HINT);
    //     scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorHintColor"].toString()), INDICATOR_HINT);
    //     scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_HINT);
    //     // highlight
    //     scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorHighlightStyle"].toInt()), INDICATOR_HIGHLIGHT);
    //     scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorHighlightColor"].toString()), INDICATOR_HIGHLIGHT);
    //     scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_HIGHLIGHT);
    //     scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorReadStyle"].toInt()), INDICATOR_READ);
    //     scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorReadColor"].toString()), INDICATOR_READ);
    //     scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_READ);
    //     scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorWriteStyle"].toInt()), INDICATOR_WRITE);
    //     scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorWriteColor"].toString()), INDICATOR_WRITE);
    //     scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_WRITE);
    //     // search
    //     scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorSearchStyle"].toInt()), INDICATOR_SEARCH);
    //     scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorSearchColor"].toString()), INDICATOR_SEARCH);
    //     scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_SEARCH);
    //     scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorSelectionStyle"].toInt()), INDICATOR_SELECTION);
    //     scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorSelectionColor"].toString()), INDICATOR_SELECTION);
    //     scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_SELECTION);
    //     // hyperlink
    //     scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorHyperlinkStyle"].toInt()), INDICATOR_HYPERLINK);
    //     scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorHyperlinkColor"].toString()), INDICATOR_HYPERLINK);
    //     scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_HYPERLINK);
    //     // recolor
    //     scriptPage->m_editorWidget->recolor();
    // }
}

void ScriptModule::scriptIndicatorSave(const QJsonObject &indicatorConfigScript) {
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

void ScriptModule::scriptMarkerReload(const QJsonObject &markerConfigScript) const {
    // for (const auto &scriptPage: m_scriptPageHash) {
    //     scriptPage->m_editorWidget->markerDefine(static_cast<QsciScintilla::MarkerSymbol>(markerConfigScript["markerBreakpointStyle"].toInt()), MARKER_BREAKPOINT_ENABLED);
    //     scriptPage->m_editorWidget->setMarkerBackgroundColor(QColor(markerConfigScript["markerBreakpointBackground"].toString()), MARKER_BREAKPOINT_ENABLED);
    //     scriptPage->m_editorWidget->setMarkerForegroundColor(QColor(markerConfigScript["markerBreakpointForeground"].toString()), MARKER_BREAKPOINT_ENABLED);
    //     scriptPage->m_editorWidget->markerDefine(static_cast<QsciScintilla::MarkerSymbol>(markerConfigScript["markerDebugStyle"].toInt()), MARKER_DEBUG);
    //     scriptPage->m_editorWidget->setMarkerBackgroundColor(QColor(markerConfigScript["markerDebugBackground"].toString()), MARKER_DEBUG);
    //     scriptPage->m_editorWidget->setMarkerForegroundColor(QColor(markerConfigScript["markerDebugForeground"].toString()), MARKER_DEBUG);
    //     // recolor
    //     scriptPage->m_editorWidget->recolor();
    // }
}

void ScriptModule::scriptMarkerSave(const QJsonObject &markerConfigScript) {
    m_scriptConfig["markerBreakpointStyle"] = markerConfigScript["markerBreakpointStyle"].toInt();
    m_scriptConfig["markerBreakpointBackground"] = markerConfigScript["markerBreakpointBackground"].toString();
    m_scriptConfig["markerBreakpointForeground"] = markerConfigScript["markerBreakpointForeground"].toString();
    m_scriptConfig["markerDebugStyle"] = markerConfigScript["markerDebugStyle"].toInt();
    m_scriptConfig["markerDebugBackground"] = markerConfigScript["markerDebugBackground"].toString();
    m_scriptConfig["markerDebugForeground"] = markerConfigScript["markerDebugForeground"].toString();
}

// public: file
void ScriptModule::scriptOpen(const QUrl &scriptUrl) {
    // open page
    if (!m_scriptPageHash.contains(scriptUrl)) {
        // create script page
        auto *scriptPage = new ScriptPage(m_scriptConfig, scriptUrl);
        scriptPage->setObjectName(scriptUrl.toString());
        // check same file name
        bool conflict = false;
        for (const auto &url: m_scriptPageHash.keys()) {
            if (url.fileName() == scriptUrl.fileName()) {
                conflict = true;
                ScriptPage *conflictPage = m_scriptPageHash[url];
                conflictPage->pathDisambiguation();
            }
        }
        if (conflict) {
            scriptPage->pathDisambiguation();
        }
        // insert url to hash
        m_scriptPageHash[scriptUrl] = scriptPage;
        connect(scriptPage, &ScriptPage::isFocusedChanged, this, [this, scriptPage](const bool status) {scriptFocus(scriptPage, status);});
        connect(scriptPage, &ScriptPage::appendLog, this, &ScriptModule::appendLog);
        connect(scriptPage, &ScriptPage::closeScript, this, &ScriptModule::scriptClose);
        connect(scriptPage, &ScriptPage::startThread, this, &ScriptModule::startThread);
        connect(scriptPage, &ScriptPage::changeSelection, this, &ScriptModule::changeSelection);
        connect(scriptPage, &ScriptPage::setPermission, this, &ScriptModule::permissionSet);
        connect(scriptPage, &ScriptPage::editBreakpoint, this, &ScriptModule::breakpointEdit);
        connect(scriptPage, &ScriptPage::showMenu, this, &ScriptModule::menuShow);
        connect(scriptPage, &ScriptPage::setTooltip, this, &ScriptModule::tooltipSet);
        connect(scriptPage, &ScriptPage::insertBreakpoint, this, &ScriptModule::insertBreakpoint);
        connect(scriptPage, &ScriptPage::removeBreakpoint, this, &ScriptModule::removeBreakpoint);
        connect(scriptPage, &ScriptPage::requestCompletion, this, &ScriptModule::completionRequest);
        connect(scriptPage, &ScriptPage::requestDefinition, this, &ScriptModule::definitionRequest);
        connect(scriptPage, &ScriptPage::requestDocumentHighlight, this, &ScriptModule::documentHighlightRequest);
        connect(scriptPage, &ScriptPage::requestDocumentSymbol, this, &ScriptModule::documentSymbolRequest);
        connect(scriptPage, &ScriptPage::requestFoldingRange, this, &ScriptModule::foldingRangeRequest);
        connect(scriptPage, &ScriptPage::requestFormatting, this, &ScriptModule::formattingRequest);
        connect(scriptPage, &ScriptPage::requestHover, this, &ScriptModule::hoverRequest);
        connect(scriptPage, &ScriptPage::requestImplementation, this, &ScriptModule::implementationRequest);
        connect(scriptPage, &ScriptPage::requestOnTypeFormatting, this, &ScriptModule::onTypeFormattingRequest);
        connect(scriptPage, &ScriptPage::requestReferences, this, &ScriptModule::referencesRequest);
        connect(scriptPage, &ScriptPage::requestSemanticTokens, this, &ScriptModule::semanticTokensRequest);
        connect(scriptPage, &ScriptPage::requestSignatureHelp, this, &ScriptModule::signatureHelpRequest);
        connect(scriptPage, &ScriptPage::requestSpellCheck, this, &ScriptModule::requestSpellCheck);
        connect(scriptPage, &ScriptPage::requestTypeDefinition, this, &ScriptModule::typeDefinitionRequest);
        connect(scriptPage, &ScriptPage::notificationJson, this, &ScriptModule::notificationJson);
        connect(scriptPage, &ScriptPage::showDiagnostic, m_codeAssistant, &CodeAssistant::diagnosticShow);
        connect(scriptPage, &ScriptPage::hideDwell, m_codeAssistant, &CodeAssistant::dwellHide);
        qApp->installEventFilter(m_codeAssistant);
        if (m_focusedPage == nullptr) {
            m_welcomePage->open();
            m_welcomePage->addDockWidgetAsTab(scriptPage);
            m_welcomePage->close();
        } else {
            m_focusedPage->addDockWidgetAsTab(scriptPage);
        }
        scriptPage->diagnosticsResponse(m_diagnosticsHash[scriptUrl]);
    }
    m_scriptPageHash[scriptUrl]->raise();
    m_scriptPageHash[scriptUrl]->setFocus(Qt::FocusReason::MouseFocusReason);
}

int ScriptModule::eolModeGet(const QUrl &scriptUrl) const {
    return m_scriptPageHash[scriptUrl]->m_editorWidget->eolModeGet();
}

void ScriptModule::eolModeSet(const QUrl &scriptUrl, const int eolMode) const {
    m_scriptPageHash[scriptUrl]->m_editorWidget->eolModeSet(eolMode);
}

bool ScriptModule::eolViewGet(const QUrl &scriptUrl) {
    return m_scriptPageHash[scriptUrl]->m_editorWidget->eolViewGet();
}

void ScriptModule::eolViewSet(const QUrl &scriptUrl, const bool status) const {
    m_scriptPageHash[scriptUrl]->m_editorWidget->eolViewSet(status);
}

// public: document
void ScriptModule::foldContractTop(const QUrl &scriptUrl) {
    if (!m_scriptPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    m_scriptPageHash[scriptUrl]->m_editorWidget->foldContractTop();
}

void ScriptModule::foldContractRecursively(const QUrl &scriptUrl) {
    if (!m_scriptPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    m_scriptPageHash[scriptUrl]->m_editorWidget->foldContractRecursively();
}

void ScriptModule::foldExpandRecursively(const QUrl &scriptUrl) {
    if (!m_scriptPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    m_scriptPageHash[scriptUrl]->m_editorWidget->foldExpandRecursively();
}

void ScriptModule::assemblyToggle(const QUrl &scriptUrl, const bool status) {
    m_scriptPageHash[scriptUrl]->assemblyToggle(status);
}

void ScriptModule::indexSet(const QUrl &scriptUrl, const int line, const int character) {
    if (!m_scriptPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    m_scriptPageHash[scriptUrl]->m_editorWidget->indexSet(line, character);
}

void ScriptModule::indexGet() const {
    const auto scriptUrl = m_focusedPage->m_scriptUrl;
    const auto index = m_focusedPage->m_editorWidget->indexGet();
    g_cursorPosition = {
        {"url", scriptUrl},
        {"line", index["line"]},
        {"character", index["character"]}
    };
}

QString ScriptModule::textGet(const QUrl &scriptUrl, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
    if (m_scriptPageHash.contains(scriptUrl)) {
        return m_scriptPageHash[scriptUrl]->m_editorWidget->textGet(startLine, startCharacter, endLine, endCharacter);
    }
    // TODO: rewrite text get from file later
    return {};
    // QString script{};
    // // get text from editor
    // if (m_scriptPageHash.contains(scriptUrl)) {
    //     const auto *scriptPage = m_scriptPageHash[scriptUrl];
    //     script = scriptPage->m_editorWidget->text();
    //     script.replace("\r\n", "\n");
    // }
    // // get text from file
    // else {
    //     QFile file(scriptUrl.toLocalFile());
    //     if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    //         QTextStream in(&file);
    //         script = in.readAll();
    //         file.close();
    //     }
    // }
    // // get full script if start line is -1
    // if (startLine == -1) {
    //     return script;
    // }
    // // split script into lines
    // const QStringList lines = script.split("\n");
    // // get full line if start character is -1
    // if (startCharacter == -1) {
    //     const QString &line = lines[startLine];
    //     m_codeAssistant->navigationResponse(line);
    //     return line;
    // }
    // return {};
}

void ScriptModule::indicatorFill(const QUrl &scriptUrl, const int type, const int startLine, const int startCharacter, const int endLine, const int endCharacter, const int time) {
    if (!m_scriptPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    m_scriptPageHash[scriptUrl]->m_editorWidget->indicatorFill(type, startLine, startCharacter, endLine, endCharacter, time);
}

void ScriptModule::indicatorClear(const QUrl &scriptUrl, const int type, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
    if (!m_scriptPageHash.contains(scriptUrl)) return;
    m_scriptPageHash[scriptUrl]->m_editorWidget->indicatorClear(type, startLine, startCharacter, endLine, endCharacter);
}

void ScriptModule::markerAdd(const QUrl &scriptUrl, const int type, const int line, const int time) {
    if (!m_scriptPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    m_scriptPageHash[scriptUrl]->m_editorWidget->markerAdd(type, line, time);
}

void ScriptModule::markerDelete(const QUrl &scriptUrl, const int type, const int line) {
    if (!m_scriptPageHash.contains(scriptUrl)) return;
    m_scriptPageHash[scriptUrl]->m_editorWidget->markerDelete(type, line);
}

// public: lsp
void ScriptModule::diagnosticsNotification(const QUrl &scriptUrl, const QJsonArray &diagnostics) {
    m_diagnosticsHash.insert(scriptUrl, diagnostics);
    if (m_scriptPageHash.contains(scriptUrl)) {
        m_scriptPageHash[scriptUrl]->diagnosticsResponse(diagnostics);
    }
}

void ScriptModule::codeActionRequest(const QUrl &scriptUrl, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
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

void ScriptModule::completionRequest(const QUrl &scriptUrl, int line, int character) {
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

void ScriptModule::completionResponse(const QUrl &scriptUrl, const QJsonArray &items) const {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    const auto *scintilla = static_cast<ScintillaWidget *>(scriptPage->m_editorWidget);
    const auto wordIndex = scintilla->wordIndexGet();
    const auto startLine = wordIndex["startLine"];
    const auto startCharacter = wordIndex["startCharacter"];
    const auto endLine = wordIndex["endLine"];
    const auto endCharacter = wordIndex["endCharacter"];
    const auto height = scintilla->heightGet();
    const auto point = scintilla->pointGet(startLine, startCharacter);
    const auto x = point["x"];
    const auto y = point["y"] + height;
    const QPoint position = scintilla->mapTo(scintilla->window(), QPoint(x, y));
    // call completion show
    const QVariantHash completionSession = {
        {"scriptUrl", scriptUrl},
        {"position", position},
        {"startLine", startLine},
        {"startCharacter", startCharacter},
        {"endLine", endLine},
        {"endCharacter", endCharacter}
    };
    m_codeAssistant->completionShow(completionSession, items);
}

void ScriptModule::definitionRequest(const QUrl &scriptUrl, const int line, const int character) {
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

void ScriptModule::definitionResponse(const QUrl &scriptUrl, const QJsonArray &definitions) const {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    const auto *scintilla = static_cast<ScintillaWidget *>(scriptPage->m_editorWidget);
    const auto wordIndex = scintilla->wordIndexGet();
    const auto startLine = wordIndex["startLine"];
    const auto startCharacter = wordIndex["startCharacter"];
    const auto height = scintilla->heightGet();
    const auto point = scintilla->pointGet(startLine, startCharacter);
    const auto x = point["x"];
    const auto y = point["y"] + height;
    const QPoint position = scintilla->mapTo(scintilla->window(), QPoint(x, y));
    // call navigation show
    const QVariantHash navigationSession = {
        {"type", "definition"},
        {"scriptUrl", scriptUrl},
        {"position", position}
    };
    m_codeAssistant->navigationShow(navigationSession, definitions);
}

void ScriptModule::documentSymbolRequest(const QUrl &scriptUrl) {
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

void ScriptModule::documentSymbolResponse(const QUrl &scriptUrl, const QJsonArray &result) {
    m_scriptPageHash[scriptUrl]->documentSymbolResponse(result);
}

void ScriptModule::documentHighlightRequest(const QUrl &scriptUrl, const int line, const int character) {
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

void ScriptModule::documentHighlightResponse(const QUrl &scriptUrl, const QJsonArray &result) {
    m_scriptPageHash[scriptUrl]->documentHighlightResponse(result);
}

void ScriptModule::foldingRangeRequest(const QUrl &scriptUrl) {
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

void ScriptModule::foldingRangeResponse(const QUrl &scriptUrl, const QJsonArray &result) const {
    m_scriptPageHash[scriptUrl]->foldingRangeResponse(result);
}

void ScriptModule::formattingRequest(const QUrl &scriptUrl) {
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

void ScriptModule::formattingResponse(const QUrl &scriptUrl, const QString &newText) const {
    m_scriptPageHash[scriptUrl]->formattingResponse(newText);
}

void ScriptModule::hoverRequest(const QUrl &scriptUrl, int line, int character) {
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

void ScriptModule::hoverResponse(const QUrl &scriptUrl, const QString &message) const {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    const auto *scintilla = static_cast<ScintillaWidget *>(scriptPage->m_editorWidget);
    const QPoint position = scintilla->window()->mapFromGlobal(QCursor::pos() + QPoint(10, 10));
    // call hover show
    const QVariantHash hoverSession = {
        {"position", position}
    };
    m_codeAssistant->hoverShow(hoverSession, message);
}

void ScriptModule::implementationRequest(const QUrl &scriptUrl, const int line, const int character) {
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

void ScriptModule::implementationResponse(const QUrl &scriptUrl, const QJsonArray &implementations) const {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    const auto *scintilla = static_cast<ScintillaWidget *>(scriptPage->m_editorWidget);
    const auto wordIndex = scintilla->wordIndexGet();
    const auto startLine = wordIndex["startLine"];
    const auto startCharacter = wordIndex["startCharacter"];
    const auto height = scintilla->heightGet();
    const auto point = scintilla->pointGet(startLine, startCharacter);
    const auto x = point["x"];
    const auto y = point["y"] + height;
    const QPoint position = scintilla->mapTo(scintilla->window(), QPoint(x, y));
    // call navigation show
    const QVariantHash navigationSession = {
        {"type", "implementation"},
        {"scriptUrl", scriptUrl},
        {"position", position}
    };
    m_codeAssistant->navigationShow(navigationSession, implementations);
}

void ScriptModule::onTypeFormattingRequest(const QUrl &scriptUrl, int line, int character) {
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

void ScriptModule::onTypeFormattingResponse(const QUrl &scriptUrl, const QJsonObject &newText) const {
    m_scriptPageHash[scriptUrl]->onTypeFormattingResponse(newText);
}

void ScriptModule::rangeFormattingRequest(const QUrl &scriptUrl, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
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

void ScriptModule::rangeFormattingResponse(const QUrl &scriptUrl, const QString &newText) const {
    auto text = newText;
    if (text.endsWith("\r\n")) text.chop(2);
    m_scriptPageHash[scriptUrl]->rangeFormattingResponse(text);
}

void ScriptModule::referencesRequest(const QUrl &scriptUrl, int line, int character) {
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

void ScriptModule::referencesResponse(const QUrl &scriptUrl, const QJsonArray &references) const {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    const auto *scintilla = static_cast<ScintillaWidget *>(scriptPage->m_editorWidget);
    const auto wordIndex = scintilla->wordIndexGet();
    const auto startLine = wordIndex["startLine"];
    const auto startCharacter = wordIndex["startCharacter"];
    const auto height = scintilla->heightGet();
    const auto point = scintilla->pointGet(startLine, startCharacter);
    const auto x = point["x"];
    const auto y = point["y"] + height;
    const QPoint position = scintilla->mapTo(scintilla->window(), QPoint(x, y));
    // call navigation show
    const QVariantHash navigationSession = {
        {"type", "reference"},
        {"scriptUrl", scriptUrl},
        {"position", position}
    };
    m_codeAssistant->navigationShow(navigationSession, references);
}

void ScriptModule::semanticTokensRequest(const QUrl &scriptUrl) {
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

void ScriptModule::semanticTokensResponse(const QUrl &scriptUrl, const QJsonArray &data) const {
    m_scriptPageHash[scriptUrl]->semanticTokensResponse(data);
}

void ScriptModule::signatureHelpRequest(const QUrl &scriptUrl, int line, int character) {
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

void ScriptModule::signatureHelpResponse(const QUrl &scriptUrl, const QJsonArray &signatures) const {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    const auto *scintilla = static_cast<ScintillaWidget *>(scriptPage->m_editorWidget);
    const auto wordIndex = scintilla->wordIndexGet();
    const auto startLine = wordIndex["startLine"];
    const auto startCharacter = wordIndex["startCharacter"];
    const auto point = scintilla->pointGet(startLine, startCharacter - 1);
    const auto x = point["x"];
    const auto y = point["y"];
    const QPoint position = scintilla->mapTo(scintilla->window(), QPoint(x, y));
    // call signature show
    const QVariantHash signatureSession = {
        {"scriptUrl", scriptUrl},
        {"position", position}
    };
    m_codeAssistant->signatureShow(signatureSession, signatures);
}

void ScriptModule::typeDefinitionRequest(const QUrl &scriptUrl, const int line, const int character) {
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

void ScriptModule::typeDefinitionResponse(const QUrl &scriptUrl, const QJsonArray &typeDefinitions) const {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    const auto *scintilla = static_cast<ScintillaWidget *>(scriptPage->m_editorWidget);
    const auto wordIndex = scintilla->wordIndexGet();
    const auto startLine = wordIndex["startLine"];
    const auto startCharacter = wordIndex["startCharacter"];
    const auto height = scintilla->heightGet();
    const auto point = scintilla->pointGet(startLine, startCharacter);
    const auto x = point["x"];
    const auto y = point["y"] + height;
    const QPoint position = scintilla->mapTo(scintilla->window(), QPoint(x, y));
    // call navigation show
    const QVariantHash navigationSession = {
        {"type", "typeDefinition"},
        {"scriptUrl", scriptUrl},
        {"position", position}
    };
    m_codeAssistant->navigationShow(navigationSession, typeDefinitions);
}

// public: typo
void ScriptModule::spellCheckResponse(const QUrl &scriptUrl, const QVariantList &typos) {
    if (!m_scriptPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    m_scriptPageHash[scriptUrl]->spellCheckResponse(typos);
}

// private
void ScriptModule::scriptFocus(ScriptPage *scriptPage, const bool status) {
    if (status) {
        m_focusedPage = scriptPage;
        const QVariantHash session = {
            {"codePage", scriptPage->m_editorWidget->codePageGet()},
            {"eolMode", scriptPage->m_editorWidget->eolModeGet()}
        };
        emit focusScript(scriptPage->m_scriptUrl, session);
    } else {
        scriptPage->m_editorWidget->indicatorClear(INDICATOR_HIGHLIGHT);
        scriptPage->m_editorWidget->indicatorClear(INDICATOR_READ);
        scriptPage->m_editorWidget->indicatorClear(INDICATOR_WRITE);
    }
}

void ScriptModule::scriptClose(const QUrl &scriptUrl) {
    m_scriptPageHash.remove(scriptUrl);
    if (m_scriptPageHash.isEmpty()) {
        m_welcomePage->open();
        m_focusedPage = nullptr;
    } else {
        const auto begin = m_scriptPageHash.begin();
        m_focusedPage = begin.value();
    }
}

void ScriptModule::charAdd(const QUrl &scriptUrl, const QChar character) const {
    m_scriptPageHash[scriptUrl]->charAdd(character.toLatin1());
}

void ScriptModule::textSet(const QUrl &scriptUrl, const QString &text, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
    if (!m_scriptPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    m_scriptPageHash[scriptUrl]->m_editorWidget->textSet(text, startLine, startCharacter, endLine, endCharacter);
}

void ScriptModule::permissionSet(const QUrl &scriptUrl, const bool readonly) const {
    m_permissionDialog->setProperty("fileUrl", scriptUrl.toString());
    m_permissionDialog->setProperty("readonly", readonly);
    QMetaObject::invokeMethod(m_permissionDialog, "open");
}

void ScriptModule::breakpointEdit(const QUrl &scriptUrl, const int line) const {
    m_breakpointEditDialog->setProperty("scriptUrl", scriptUrl.toString());
    m_breakpointEditDialog->setProperty("line", line);
    QMetaObject::invokeMethod(m_breakpointEditDialog, "open");
}

void ScriptModule::menuShow(const QUrl &scriptUrl, const QVariantHash &menuSession) const {
    m_menu->setProperty("scriptUrl", scriptUrl.toString());
    m_menu->setProperty("menuSession", menuSession);
    QMetaObject::invokeMethod(m_menu, "popup");
}

void ScriptModule::tooltipSet(const QPoint &position, const QString &text) const {
    m_toolTip->setProperty("position", position);
    m_toolTip->setProperty("text", text);
}
