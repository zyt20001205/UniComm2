#include "scriptModule/scriptModule.h"

#include <QFileInfo>
#include <QShortcut>
#include <QTextBrowser>

#include "globals.h"
#include "luaModule/luaControl.h"
#include "portModule/portModule.h"
#include "scriptModule/scriptPage.h"
#include "scriptModule/welcomePage.h"
#include "scriptModule/codeAssistant/codeAssistant.h"
#include "scriptModule/codeEditor/editorWidget.h"

// ScriptModule public
ScriptModule::ScriptModule(QWidget *parent)
    : QObject(parent),
      m_scriptConfig(g_workspaceConfig["scriptConfig"].toObject()),
      m_welcomePage(new WelcomePage()),
      m_codeAssistant(new CodeAssistant(parent)) {
    m_welcomePage->setObjectName("welcomePage");
    for (const auto &value: m_scriptConfig["scriptList"].toArray()) {
        scriptOpen(QUrl(value.toString()));
    }
    connect(m_welcomePage, &WelcomePage::openWorkspace, this, &ScriptModule::openWorkspace);
    connect(this, &ScriptModule::responseCodeAction, m_codeAssistant, &CodeAssistant::dwellShowCodeAction);
    connect(m_codeAssistant, &CodeAssistant::addChar, this, &ScriptModule::charAdd);
    connect(m_codeAssistant, &CodeAssistant::setCursorPosition, this, &ScriptModule::cursorPositionSet);
    connect(m_codeAssistant, &CodeAssistant::getText, this, &ScriptModule::textGet);
    connect(m_codeAssistant, &CodeAssistant::insertText, this, &ScriptModule::textInsert);
    connect(m_codeAssistant, &CodeAssistant::replaceText, this, &ScriptModule::textReplace);
    connect(m_codeAssistant, &CodeAssistant::insertIndicator, this, &ScriptModule::indicatorInsert);
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
    m_welcomePage->propertySet(QVariantMap());
    m_codeAssistant->propertySet(objects);
    m_codeAssistant->fontSet(m_scriptConfig["fontFamily"].toString(), m_scriptConfig["fontSize"].toInt());
    m_editorMenu = qvariant_cast<QObject *>(objects["scriptModuleEditorMenu"]);
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

    g_workspaceConfig["scriptConfig"] = m_scriptConfig;
}

void ScriptModule::scriptFontReload(const QJsonObject &fontConfigScript) const {
    const auto scriptFont = QFont(fontConfigScript["fontFamily"].toString(), fontConfigScript["fontSize"].toInt());
    for (const auto &scriptPage: m_scriptPageHash) {
        scriptPage->m_editorWidget->setFont(scriptFont);
    }
}

void ScriptModule::scriptFontSave(const QJsonObject &fontConfigScript) {
    m_scriptConfig["fontFamily"] = fontConfigScript["fontFamily"].toString();
    m_scriptConfig["fontSize"] = fontConfigScript["fontSize"].toInt();
}

void ScriptModule::scriptIndicatorReload(const QJsonObject &indicatorConfigScript) const {
    for (const auto &scriptPage: m_scriptPageHash) {
        // diagnostic
        scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorErrorStyle"].toInt()), INDICATOR_ERROR);
        scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorErrorColor"].toString()), INDICATOR_ERROR);
        scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_ERROR);
        scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorWarningStyle"].toInt()), INDICATOR_WARNING);
        scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorWarningColor"].toString()), INDICATOR_WARNING);
        scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_WARNING);
        scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorInfoStyle"].toInt()), INDICATOR_INFO);
        scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorInfoColor"].toString()), INDICATOR_INFO);
        scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_INFO);
        scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorHintStyle"].toInt()), INDICATOR_HINT);
        scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorHintColor"].toString()), INDICATOR_HINT);
        scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_HINT);
        // highlight
        scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorHighlightStyle"].toInt()), INDICATOR_HIGHLIGHT);
        scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorHighlightColor"].toString()), INDICATOR_HIGHLIGHT);
        scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_HIGHLIGHT);
        scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorReadStyle"].toInt()), INDICATOR_READ);
        scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorReadColor"].toString()), INDICATOR_READ);
        scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_READ);
        scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorWriteStyle"].toInt()), INDICATOR_WRITE);
        scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorWriteColor"].toString()), INDICATOR_WRITE);
        scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_WRITE);
        // search
        scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorSearchStyle"].toInt()), INDICATOR_SEARCH);
        scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorSearchColor"].toString()), INDICATOR_SEARCH);
        scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_SEARCH);
        scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorSelectionStyle"].toInt()), INDICATOR_SELECTION);
        scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorSelectionColor"].toString()), INDICATOR_SELECTION);
        scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_SELECTION);
        // hyperlink
        scriptPage->m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorHyperlinkStyle"].toInt()), INDICATOR_HYPERLINK);
        scriptPage->m_editorWidget->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorHyperlinkColor"].toString()), INDICATOR_HYPERLINK);
        scriptPage->m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_HYPERLINK);
        // recolor
        scriptPage->m_editorWidget->recolor();
    }
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
    for (const auto &scriptPage: m_scriptPageHash) {
        scriptPage->m_editorWidget->markerDefine(static_cast<QsciScintilla::MarkerSymbol>(markerConfigScript["markerBreakpointStyle"].toInt()), MARKER_BREAKPOINT);
        scriptPage->m_editorWidget->setMarkerBackgroundColor(QColor(markerConfigScript["markerBreakpointBackground"].toString()), MARKER_BREAKPOINT);
        scriptPage->m_editorWidget->setMarkerForegroundColor(QColor(markerConfigScript["markerBreakpointForeground"].toString()), MARKER_BREAKPOINT);
        scriptPage->m_editorWidget->markerDefine(static_cast<QsciScintilla::MarkerSymbol>(markerConfigScript["markerDebugStyle"].toInt()), MARKER_DEBUG);
        scriptPage->m_editorWidget->setMarkerBackgroundColor(QColor(markerConfigScript["markerDebugBackground"].toString()), MARKER_DEBUG);
        scriptPage->m_editorWidget->setMarkerForegroundColor(QColor(markerConfigScript["markerDebugForeground"].toString()), MARKER_DEBUG);
        // recolor
        scriptPage->m_editorWidget->recolor();
    }
}

void ScriptModule::scriptMarkerSave(const QJsonObject &markerConfigScript) {
    m_scriptConfig["markerBreakpointStyle"] = markerConfigScript["markerBreakpointStyle"].toInt();
    m_scriptConfig["markerBreakpointBackground"] = markerConfigScript["markerBreakpointBackground"].toString();
    m_scriptConfig["markerBreakpointForeground"] = markerConfigScript["markerBreakpointForeground"].toString();
    m_scriptConfig["markerDebugStyle"] = markerConfigScript["markerDebugStyle"].toInt();
    m_scriptConfig["markerDebugBackground"] = markerConfigScript["markerDebugBackground"].toString();
    m_scriptConfig["markerDebugForeground"] = markerConfigScript["markerDebugForeground"].toString();
}

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
        connect(scriptPage, &KDDockWidgets::QtWidgets::DockWidget::isFocusedChanged, this, [this, scriptPage](const bool status) {
            scriptFocus(scriptPage, status);
        });
        connect(scriptPage, &ScriptPage::appendLog, this, &ScriptModule::appendLog);
        connect(scriptPage, &ScriptPage::closeScript, this, &ScriptModule::scriptClose);
        connect(scriptPage, &ScriptPage::positionScript, this, &ScriptModule::positionScript);
        connect(scriptPage, &ScriptPage::showMenu,this,&ScriptModule::menuShow);
        connect(scriptPage, &ScriptPage::insertMarker, this, &ScriptModule::markerInsert);
        connect(scriptPage, &ScriptPage::removeMarker, this, &ScriptModule::markerRemove);
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
        connect(scriptPage, &ScriptPage::showDiagnosticDwell, m_codeAssistant, &CodeAssistant::dwellShowDiagnostic);
        connect(scriptPage, &ScriptPage::hideDwell, m_codeAssistant, &CodeAssistant::dwellHide);
        connect(scriptPage, &ScriptPage::leaveDwell, m_codeAssistant, &CodeAssistant::dwellLeave);
        qApp->installEventFilter(m_codeAssistant);
        if (m_focusedPage == nullptr) {
            m_welcomePage->open();
            m_welcomePage->addDockWidgetAsTab(scriptPage);
            m_welcomePage->close();
        } else {
            m_focusedPage->addDockWidgetAsTab(scriptPage);
        }
        scriptFocus(scriptPage, true);
        scriptPage->diagnosticsResponse(m_diagnosticsHash[scriptUrl]);
    } else {
        m_scriptPageHash[scriptUrl]->raise();
    }
}

void ScriptModule::collapseAll(const QUrl &scriptUrl) {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_editorWidget->SendScintilla(QsciScintillaBase::SCI_FOLDALL, QsciScintillaBase::SC_FOLDACTION_CONTRACT); // NOLINT
}

void ScriptModule::expandAll(const QUrl &scriptUrl) {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_editorWidget->SendScintilla(QsciScintillaBase::SCI_FOLDALL, QsciScintillaBase::SC_FOLDACTION_EXPAND); // NOLINT
}

void ScriptModule::cursorPositionSet(const QUrl &scriptUrl, const int startLine, const int startCharacter) {
    scriptOpen(scriptUrl);
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_editorWidget->setCursorPosition(startLine, startCharacter);
    scriptPage->m_editorWidget->setFocus();
}

void ScriptModule::cursorPositionGet() const {
    const QUrl scriptUrl = m_focusedPage->m_scriptUrl;
    int line, index;
    m_focusedPage->m_editorWidget->getCursorPosition(&line, &index);
    g_cursorPosition = {
        {"url", scriptUrl},
        {"line", line + 1},
        {"character", index}
    };
}

QString ScriptModule::textGet(const QUrl &scriptUrl, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
    QString script{};
    // get text from editor
    if (m_scriptPageHash.contains(scriptUrl)) {
        const auto *scriptPage = m_scriptPageHash[scriptUrl];
        script = scriptPage->m_editorWidget->text();
        script.replace("\r\n", "\n");
    }
    // get text from file
    else {
        QFile file(scriptUrl.toLocalFile());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            script = in.readAll();
            file.close();
        }
    }
    // get full script if start line is -1
    if (startLine == -1) {
        return script;
    }
    // split script into lines
    const QStringList lines = script.split("\n");
    // get full line if start character is -1
    if (startCharacter == -1) {
        const QString &line = lines[startLine];
        m_codeAssistant->navigationResponse(line);
        return line;
    }
    // TODO: cover other conditions
    return {};
}

void ScriptModule::indicatorInsert(const QUrl &scriptUrl, const int type, const int lineFrom, const int indexFrom, const int lineTo, const int indexTo, const int time) {
    if (!m_scriptPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_editorWidget->indicatorInsert(type, lineFrom, indexFrom, lineTo, indexTo, time);
}

void ScriptModule::indicatorRemove(const QUrl &scriptUrl, const int type, const int lineFrom, const int indexFrom, const int lineTo, const int indexTo) {
    if (!m_scriptPageHash.contains(scriptUrl)) return;
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_editorWidget->indicatorRemove(type, lineFrom, indexFrom, lineTo, indexTo);
}

void ScriptModule::markerInsert(const QUrl &scriptUrl, const int type, const int line, const int time) {
    if (!m_scriptPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_editorWidget->markerInsert(type, line, time);
}

void ScriptModule::markerRemove(const QUrl &scriptUrl, const int type, const int line) {
    if (!m_scriptPageHash.contains(scriptUrl)) return;
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_editorWidget->markerRemove(type, line);
}

void ScriptModule::annotationInsert(const QUrl &scriptUrl, const int line, const QString &annotation) {
    if (!m_scriptPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_editorWidget->annotate(line - 1, annotation, 0);
}

void ScriptModule::annotationRemove(const QUrl &scriptUrl, const int line) {
    if (!m_scriptPageHash.contains(scriptUrl)) return;
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    if (line == -1) {
        scriptPage->m_editorWidget->clearAnnotations();
    } else {
        scriptPage->m_editorWidget->annotate(line - 1, "", 0);
    }
}

void ScriptModule::diagnosticsNotification(const QUrl &scriptUrl, const QJsonArray &diagnostics) {
    m_diagnosticsHash.insert(scriptUrl, diagnostics);
    if (m_scriptPageHash.contains(scriptUrl)) {
        m_scriptPageHash[scriptUrl]->diagnosticsResponse(diagnostics);
    }
}

void ScriptModule::codeActionRequest(const QUrl &scriptUrl, const int lineFrom, const int indexFrom, const int lineTo, const int indexTo) {
    QJsonArray diagnosticArray{};
    for (const auto &value: m_diagnosticsHash[scriptUrl]) {
        const QJsonObject diagnostic = value.toObject();
        const QJsonObject range = diagnostic["range"].toObject();
        const QJsonObject startPos = range["start"].toObject();
        const QJsonObject endPos = range["end"].toObject();
        const int startLine = startPos["line"].toInt();
        const int startCharacter = startPos["character"].toInt();
        const int endLine = endPos["line"].toInt();
        const int endCharacter = endPos["character"].toInt();
        if (lineFrom == startLine && lineTo == endLine && indexFrom == startCharacter && indexTo == endCharacter) {
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
                        {"line", lineFrom},
                        {"character", indexFrom}
                    }
                },
                {
                    "end", QJsonObject{
                        {"line", lineTo},
                        {"character", indexTo}
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
    const auto *editor = static_cast<QsciScintilla *>(scriptPage->m_editorWidget);
    const long currentPos = editor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    const long wordStartPos = editor->SendScintilla(QsciScintilla::SCI_WORDSTARTPOSITION, currentPos, true);
    const long wordEndPos = editor->SendScintilla(QsciScintilla::SCI_WORDENDPOSITION, currentPos, true);
    // get completion replace range
    const int line = editor->SendScintilla(QsciScintilla::SCI_LINEFROMPOSITION, wordStartPos);
    const int indexFrom = wordStartPos - editor->SendScintilla(QsciScintilla::SCI_POSITIONFROMLINE, line);
    const int indexTo = wordEndPos - editor->SendScintilla(QsciScintilla::SCI_POSITIONFROMLINE, line);
    // get completion display position
    const int x = editor->SendScintilla(QsciScintilla::SCI_POINTXFROMPOSITION, 0, wordStartPos) - 26;
    const int lineHeight = editor->SendScintilla(QsciScintilla::SCI_TEXTHEIGHT, 0);
    const int y = editor->SendScintilla(QsciScintilla::SCI_POINTYFROMPOSITION, 0, wordStartPos) + lineHeight;
    const QPoint position = editor->mapToGlobal(QPoint(x, y));
    // call completion show
    const QVariantMap completionSession = {
        {"scriptUrl", scriptUrl},
        {"position", position},
        {"line", line},
        {"indexFrom", indexFrom},
        {"indexTo", indexTo}
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
    const auto *editor = static_cast<QsciScintilla *>(scriptPage->m_editorWidget);
    const long currentPos = editor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    const long wordStartPos = editor->SendScintilla(QsciScintilla::SCI_WORDSTARTPOSITION, currentPos, true);
    // get navigation display position
    const int x = editor->SendScintilla(QsciScintilla::SCI_POINTXFROMPOSITION, 0, wordStartPos) - 2;
    const int lineHeight = editor->SendScintilla(QsciScintilla::SCI_TEXTHEIGHT, 0);
    const int y = editor->SendScintilla(QsciScintilla::SCI_POINTYFROMPOSITION, 0, wordStartPos) + lineHeight;
    const QPoint position = editor->mapToGlobal(QPoint(x, y));
    // call navigation show
    const QVariantMap navigationSession = {
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

// documentSymbolResponse is sent to structure module

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
    m_codeAssistant->dwellShowHover(message);
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
    const auto *editor = static_cast<QsciScintilla *>(scriptPage->m_editorWidget);
    const long currentPos = editor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    const long wordStartPos = editor->SendScintilla(QsciScintilla::SCI_WORDSTARTPOSITION, currentPos, true);
    // get navigation display position
    const int x = editor->SendScintilla(QsciScintilla::SCI_POINTXFROMPOSITION, 0, wordStartPos) - 2;
    const int lineHeight = editor->SendScintilla(QsciScintilla::SCI_TEXTHEIGHT, 0);
    const int y = editor->SendScintilla(QsciScintilla::SCI_POINTYFROMPOSITION, 0, wordStartPos) + lineHeight;
    const QPoint position = editor->mapToGlobal(QPoint(x, y));
    // call navigation show
    const QVariantMap navigationSession = {
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
    const auto *editor = static_cast<QsciScintilla *>(scriptPage->m_editorWidget);
    const long currentPos = editor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    const long wordStartPos = editor->SendScintilla(QsciScintilla::SCI_WORDSTARTPOSITION, currentPos, true);
    // get navigation display position
    const int x = editor->SendScintilla(QsciScintilla::SCI_POINTXFROMPOSITION, 0, wordStartPos) - 2;
    const int lineHeight = editor->SendScintilla(QsciScintilla::SCI_TEXTHEIGHT, 0);
    const int y = editor->SendScintilla(QsciScintilla::SCI_POINTYFROMPOSITION, 0, wordStartPos) + lineHeight;
    const QPoint position = editor->mapToGlobal(QPoint(x, y));
    // call navigation show
    const QVariantMap navigationSession = {
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

void ScriptModule::signatureHelpResponse(const QUrl &scriptUrl, const QJsonObject &signature) const {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    const auto *editor = static_cast<QsciScintilla *>(scriptPage->m_editorWidget);
    const long currentPos = editor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    const long wordStartPos = editor->SendScintilla(QsciScintilla::SCI_WORDSTARTPOSITION, currentPos, true);
    // get signature display position
    const int x = editor->SendScintilla(QsciScintilla::SCI_POINTXFROMPOSITION, 0, wordStartPos);
    const int y = editor->SendScintilla(QsciScintilla::SCI_POINTYFROMPOSITION, 0, wordStartPos);
    const QPoint position = editor->mapTo(editor->window(), QPoint(x, y));
    // call signature show
    const QVariantMap signatureSession = {
        {"scriptUrl", scriptUrl},
        {"position", position}
    };
    m_codeAssistant->signatureShow(signatureSession, signature);
}

void ScriptModule::spellCheckResponse(const QUrl &scriptUrl, const QVariantList &typos) {
    if (m_scriptPageHash.contains(scriptUrl)) {
        m_scriptPageHash[scriptUrl]->spellCheckResponse(typos);
    }
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
    const auto *editor = static_cast<QsciScintilla *>(scriptPage->m_editorWidget);
    const long currentPos = editor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    const long wordStartPos = editor->SendScintilla(QsciScintilla::SCI_WORDSTARTPOSITION, currentPos, true);
    // get navigation display position
    const int x = editor->SendScintilla(QsciScintilla::SCI_POINTXFROMPOSITION, 0, wordStartPos) - 2;
    const int lineHeight = editor->SendScintilla(QsciScintilla::SCI_TEXTHEIGHT, 0);
    const int y = editor->SendScintilla(QsciScintilla::SCI_POINTYFROMPOSITION, 0, wordStartPos) + lineHeight;
    const QPoint position = editor->mapToGlobal(QPoint(x, y));
    // call navigation show
    const QVariantMap navigationSession = {
        {"type", "typeDefinition"},
        {"scriptUrl", scriptUrl},
        {"position", position}
    };
    m_codeAssistant->navigationShow(navigationSession, typeDefinitions);
}

// ScriptModule private
void ScriptModule::scriptFocus(ScriptPage *scriptPage, const bool status) {
    if (status) {
        m_focusedPage = scriptPage;
        emit focusScript(scriptPage->m_scriptUrl);
        // logging
        // QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        // qDebug() << QString("[%1] %2 %3").arg(timestamp, scriptPage->m_scriptUrl.toString(), "focused");
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

void ScriptModule::menuShow(const QUrl &scriptUrl, const QVariantHash &menuSession) const {
    m_editorMenu->setProperty("scriptUrl", scriptUrl.toString());
    m_editorMenu->setProperty("menuSession", menuSession);
    QMetaObject::invokeMethod(m_editorMenu, "popup");
}

void ScriptModule::textInsert(const QUrl &scriptUrl, const QString &text, const int line, const int index) {
    if (!m_scriptPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_editorWidget->textInsert(text, line, index);
}

void ScriptModule::textReplace(const QUrl &scriptUrl, const QString &text, const int lineFrom, const int indexFrom, const int lineTo, const int indexTo) {
    if (!m_scriptPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_editorWidget->textReplace(text, lineFrom, indexFrom, lineTo, indexTo);
}

void ScriptModule::charAdd(const QUrl &scriptUrl, const QChar character) const {
    m_scriptPageHash[scriptUrl]->charAdded(character.toLatin1());
}
