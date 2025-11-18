#include "scriptModule/scriptModule.h"

#include <QFileInfo>
#include <QHBoxLayout>
#include <QShortcut>
#include <QTableWidget>
#include <QTextBrowser>

#include "configModule.h"
#include "globals.h"
#include "luaModule/luaControl.h"
#include "portModule/portModule.h"
#include "scriptModule/completionTooltip.h"
#include "scriptModule/hoverTooltip.h"
#include "scriptModule/positionTooltip.h"
#include "scriptModule/scriptPage.h"
#include "scriptModule/signatureHelpTooltip.h"
#include "scriptModule/welcomePage.h"

// ScriptModule public
ScriptModule::ScriptModule()
    : m_scriptConfig(g_workspaceConfig["scriptConfig"].toObject()),
      m_welcomePage(new WelcomePage()),
      m_completionTooltip(new CompletionTooltip(g_mainWindow)),
      m_hoverTooltip(new HoverTooltip(g_mainWindow)),
      m_positionTooltip(new PositionTooltip(g_mainWindow)),
      m_signatureHelpTooltip(new SignatureHelpTooltip(g_mainWindow)) {
    m_welcomePage->setObjectName("welcomePage");
    const auto breakpointHash = m_scriptConfig["breakpointHash"].toObject();
    for (const auto &key: breakpointHash.keys()) {
        const QUrl url(key);
        const auto breakpointLineHash = breakpointHash[key].toObject();
        for (auto it = breakpointLineHash.begin(); it != breakpointLineHash.end(); ++it) {
            const int line = it.key().toInt();
            const QVariantHash breakpointInfo = it.value().toObject().toVariantHash();
            g_breakpoints[url].insert(line, breakpointInfo);
            emit insertBreakpoint(url, line);
        }
    }
    for (const auto &value: m_scriptConfig["scriptList"].toArray()) {
        scriptOpen(QUrl(value.toString()));
    }
    connect(m_welcomePage, &WelcomePage::openWorkspace, this, &ScriptModule::openWorkspace);
    connect(m_completionTooltip, &CompletionTooltip::replaceText, this, &ScriptModule::textReplace);
    connect(m_positionTooltip, &PositionTooltip::replaceText, this, &ScriptModule::textReplace);
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

    auto breakpointHash = QJsonObject();
    for (const auto &url: g_breakpoints.keys()) {
        auto breakpointLineHash = QJsonObject();
        for (auto it = g_breakpoints[url].begin(); it != g_breakpoints[url].end(); ++it) {
            const int line = it.key();
            const QVariantHash &info = it.value();
            breakpointLineHash.insert(QString::number(line), QJsonObject::fromVariantHash(info));
        }
        breakpointHash.insert(url.toString(), breakpointLineHash);
    }
    m_scriptConfig["breakpointHash"] = breakpointHash;

    g_workspaceConfig["scriptConfig"] = m_scriptConfig;
}

void ScriptModule::scriptFontReload(const QJsonObject &fontConfigScript) const {
    const auto scriptFont = QFont(fontConfigScript["fontFamily"].toString(), fontConfigScript["fontSize"].toInt());
    for (const auto &scriptPage: m_scriptPageHash) {
        scriptPage->m_scriptEditor->setFont(scriptFont);
    }
}

void ScriptModule::scriptFontSave(const QJsonObject &fontConfigScript) {
    m_scriptConfig["fontFamily"] = fontConfigScript["fontFamily"].toString();
    m_scriptConfig["fontSize"] = fontConfigScript["fontSize"].toInt();
}

void ScriptModule::scriptIndicatorReload(const QJsonObject &indicatorConfigScript) const {
    for (const auto &scriptPage: m_scriptPageHash) {
        // diagnostic
        scriptPage->m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorErrorStyle"].toInt()), INDICATOR_ERROR);
        scriptPage->m_scriptEditor->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorErrorColor"].toString()), INDICATOR_ERROR);
        scriptPage->m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_ERROR);
        scriptPage->m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorWarningStyle"].toInt()), INDICATOR_WARNING);
        scriptPage->m_scriptEditor->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorWarningColor"].toString()), INDICATOR_WARNING);
        scriptPage->m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_WARNING);
        scriptPage->m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorInfoStyle"].toInt()), INDICATOR_INFO);
        scriptPage->m_scriptEditor->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorInfoColor"].toString()), INDICATOR_INFO);
        scriptPage->m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_INFO);
        scriptPage->m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorHintStyle"].toInt()), INDICATOR_HINT);
        scriptPage->m_scriptEditor->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorHintColor"].toString()), INDICATOR_HINT);
        scriptPage->m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_HINT);
        // highlight
        scriptPage->m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorHighlightStyle"].toInt()), INDICATOR_HIGHLIGHT);
        scriptPage->m_scriptEditor->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorHighlightColor"].toString()), INDICATOR_HIGHLIGHT);
        scriptPage->m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_HIGHLIGHT);
        scriptPage->m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorReadStyle"].toInt()), INDICATOR_READ);
        scriptPage->m_scriptEditor->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorReadColor"].toString()), INDICATOR_READ);
        scriptPage->m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_READ);
        scriptPage->m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorWriteStyle"].toInt()), INDICATOR_WRITE);
        scriptPage->m_scriptEditor->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorWriteColor"].toString()), INDICATOR_WRITE);
        scriptPage->m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_WRITE);
        // search
        scriptPage->m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorSearchStyle"].toInt()), INDICATOR_SEARCH);
        scriptPage->m_scriptEditor->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorSearchColor"].toString()), INDICATOR_SEARCH);
        scriptPage->m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_SEARCH);
        scriptPage->m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorSelectionStyle"].toInt()), INDICATOR_SELECTION);
        scriptPage->m_scriptEditor->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorSelectionColor"].toString()), INDICATOR_SELECTION);
        scriptPage->m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_SELECTION);
        // hyperlink
        scriptPage->m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorConfigScript["indicatorHyperlinkStyle"].toInt()), INDICATOR_HYPERLINK);
        scriptPage->m_scriptEditor->setIndicatorForegroundColor(QColor(indicatorConfigScript["indicatorHyperlinkColor"].toString()), INDICATOR_HYPERLINK);
        scriptPage->m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_HYPERLINK);
        // recolor
        scriptPage->m_scriptEditor->recolor();
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
        scriptPage->m_scriptEditor->markerDefine(static_cast<QsciScintilla::MarkerSymbol>(markerConfigScript["markerBreakpointStyle"].toInt()), MARKER_BREAKPOINT);
        scriptPage->m_scriptEditor->setMarkerBackgroundColor(QColor(markerConfigScript["markerBreakpointBackground"].toString()), MARKER_BREAKPOINT);
        scriptPage->m_scriptEditor->setMarkerForegroundColor(QColor(markerConfigScript["markerBreakpointForeground"].toString()), MARKER_BREAKPOINT);
        scriptPage->m_scriptEditor->markerDefine(static_cast<QsciScintilla::MarkerSymbol>(markerConfigScript["markerDebugStyle"].toInt()), MARKER_DEBUG);
        scriptPage->m_scriptEditor->setMarkerBackgroundColor(QColor(markerConfigScript["markerDebugBackground"].toString()), MARKER_DEBUG);
        scriptPage->m_scriptEditor->setMarkerForegroundColor(QColor(markerConfigScript["markerDebugForeground"].toString()), MARKER_DEBUG);
        // recolor
        scriptPage->m_scriptEditor->recolor();
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
        connect(scriptPage, &ScriptPage::insertPort, this, &ScriptModule::insertPort);
        connect(scriptPage, &ScriptPage::insertDatabase, this, &ScriptModule::insertDatabase);
        connect(scriptPage, &ScriptPage::insertDatatable, this, &ScriptModule::insertDatatable);
        connect(scriptPage, &ScriptPage::insertMarker, this, &ScriptModule::markerInsert);
        connect(scriptPage, &ScriptPage::removeMarker, this, &ScriptModule::markerRemove);
        connect(scriptPage, &ScriptPage::insertBreakpoint, this, &ScriptModule::insertBreakpoint);
        connect(scriptPage, &ScriptPage::removeBreakpoint, this, &ScriptModule::removeBreakpoint);
        connect(scriptPage, &ScriptPage::requestJson, this, &ScriptModule::requestJson);
        connect(scriptPage, &ScriptPage::notificationJson, this, &ScriptModule::notificationJson);
        connect(scriptPage, &ScriptPage::setFullCompletion, m_completionTooltip, &CompletionTooltip::fullCompleteSet);
        connect(scriptPage, &ScriptPage::showHoverTooltip, m_hoverTooltip, &HoverTooltip::showTooltip);
        connect(scriptPage, &ScriptPage::showPositionTooltip, m_positionTooltip, &PositionTooltip::showTooltip);
        scriptPage->m_scriptEditor->installEventFilter(m_completionTooltip);
        scriptPage->m_scriptEditor->installEventFilter(m_signatureHelpTooltip);
        if (m_focusedPage == nullptr) {
            m_welcomePage->open();
            m_welcomePage->addDockWidgetAsTab(scriptPage);
            m_welcomePage->close();
        } else {
            m_focusedPage->addDockWidgetAsTab(scriptPage);
        }
        scriptFocus(scriptPage, true);
        scriptPage->diagnosticsResponse(m_diagnosticsHash[scriptUrl]);
        emit openScript(scriptUrl);
        // load breakpoint
        if (g_breakpoints.contains(scriptUrl)) {
            for (const auto &line: g_breakpoints[scriptUrl].keys()) {
                markerInsert(scriptUrl, MARKER_BREAKPOINT, line);
            }
        }
    } else {
        m_scriptPageHash[scriptUrl]->raise();
    }
}

void ScriptModule::cursorPositionSet(const QUrl &scriptUrl, const int startLine, const int startCharacter) {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_scriptEditor->setCursorPosition(startLine, startCharacter);
    scriptPage->m_scriptEditor->setFocus();
}

void ScriptModule::cursorPositionGet() const {
    const QUrl scriptUrl = m_focusedPage->m_scriptUrl;
    int line, index;
    m_focusedPage->m_scriptEditor->getCursorPosition(&line, &index);
    g_cursorPosition = {
        {"url", scriptUrl},
        {"line", line + 1},
        {"character", index}
    };
}

void ScriptModule::indicatorInsert(const QUrl &scriptUrl, const int type, const int lineFrom, const int indexFrom, const int lineTo, const int indexTo, const int time) {
    if (!m_scriptPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_scriptEditor->indicatorInsert(type, lineFrom, indexFrom, lineTo, indexTo, time);
}

void ScriptModule::indicatorRemove(const QUrl &scriptUrl, const int type, const int lineFrom, const int indexFrom, const int lineTo, const int indexTo) {
    if (!m_scriptPageHash.contains(scriptUrl)) return;
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_scriptEditor->indicatorRemove(type, lineFrom, indexFrom, lineTo, indexTo);
}

void ScriptModule::markerInsert(const QUrl &scriptUrl, const int type, const int line, const int time) {
    if (!m_scriptPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_scriptEditor->markerInsert(type, line, time);
}

void ScriptModule::markerRemove(const QUrl &scriptUrl, const int type, const int line) {
    if (!m_scriptPageHash.contains(scriptUrl)) return;
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_scriptEditor->markerRemove(type, line);
}

void ScriptModule::annotationInsert(const QUrl &scriptUrl, const int line, const QString &annotation) {
    if (!m_scriptPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_scriptEditor->annotate(line - 1, annotation, 0);
}

void ScriptModule::annotationRemove(const QUrl &scriptUrl, const int line) {
    if (!m_scriptPageHash.contains(scriptUrl)) return;
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    if (line == -1) {
        scriptPage->m_scriptEditor->clearAnnotations();
    } else {
        scriptPage->m_scriptEditor->annotate(line - 1, "", 0);
    }
}

void ScriptModule::diagnosticsNotification(const QUrl &scriptUrl, const QJsonArray &diagnosticsArray) {
    m_diagnosticsHash.insert(scriptUrl, diagnosticsArray);
    if (m_scriptPageHash.contains(scriptUrl)) {
        m_scriptPageHash[scriptUrl]->diagnosticsResponse(diagnosticsArray);
    }
}

void ScriptModule::completionResponse(const QUrl &scriptUrl, const QJsonArray &items) const {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    const auto *editor = static_cast<QsciScintilla *>(scriptPage->m_scriptEditor);
    const long currentPos = editor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    const long wordStartPos = editor->SendScintilla(QsciScintilla::SCI_WORDSTARTPOSITION, currentPos, true);
    const int x = editor->SendScintilla(QsciScintilla::SCI_POINTXFROMPOSITION, 0, wordStartPos);
    const int y = editor->SendScintilla(QsciScintilla::SCI_POINTYFROMPOSITION, 0, wordStartPos);
    const QPoint cursorGlobalPos = editor->mapToGlobal(QPoint(x, y));
    const int lineHeight = editor->SendScintilla(QsciScintilla::SCI_TEXTHEIGHT, 0);
    m_completionTooltip->showTooltip(items);
    m_completionTooltip->move(cursorGlobalPos.x() - 2, cursorGlobalPos.y() + lineHeight);
}

void ScriptModule::definitionResponse(const QUrl &scriptUrl, const QJsonArray &definitions) {
    if (definitions.size() != 1) {
        qDebug() << "multiple definitions WIP";
        return;
    }
    for (const auto &value: definitions) {
        const QJsonObject definition = value.toObject();
        // open definition script
        QString uri = definition["uri"].toString();
        uri = QUrl::fromPercentEncoding(uri.toUtf8());
        if (QChar &drive = uri[8]; drive.isLetter() && drive.isLower()) { drive = drive.toUpper(); }
        const QUrl definitionUrl(uri);
        scriptOpen(definitionUrl);
        // show indicator
        const QJsonObject rangeObject = definition["range"].toObject();
        const QJsonObject startObject = rangeObject["start"].toObject();
        const QJsonObject endObject = rangeObject["end"].toObject();
        const int startLine = startObject["line"].toInt();
        const int startCharacter = startObject["character"].toInt();
        const int endLine = endObject["line"].toInt();
        const int endCharacter = endObject["character"].toInt();
        indicatorInsert(definitionUrl, INDICATOR_SELECTION, startLine, startCharacter, endLine, endCharacter, 1000);
        // set cursor
        cursorPositionSet(definitionUrl, startLine, startCharacter);
    }
}

void ScriptModule::documentHighlightResponse(const QUrl &scriptUrl, const QJsonArray &result) {
    m_scriptPageHash[scriptUrl]->documentHighlightResponse(result);
}

void ScriptModule::foldingRangeResponse(const QUrl &scriptUrl, const QJsonArray &result) const {
    m_scriptPageHash[scriptUrl]->foldingRangeResponse(result);
}

void ScriptModule::formattingResponse(const QUrl &scriptUrl, const QString &newText) const {
    m_scriptPageHash[scriptUrl]->formattingResponse(newText);
}

void ScriptModule::hoverResponse(const QUrl &scriptUrl, const QString &message) const {
    m_hoverTooltip->showTooltip(message);
}

void ScriptModule::semanticTokensResponse(const QUrl &scriptUrl, const QJsonArray &data) const {
    m_scriptPageHash[scriptUrl]->semanticTokensResponse(data);
}

void ScriptModule::signatureHelpResponse(const QUrl &scriptUrl, const QJsonObject &signature) const {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    const auto *editor = static_cast<QsciScintilla *>(scriptPage->m_scriptEditor);
    long currentPos = editor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    while (true) {
        const int prevChar = editor->SendScintilla(QsciScintilla::SCI_GETCHARAT, currentPos - 1);
        if (prevChar == '(') break;
        currentPos--;
    }
    const int x = editor->SendScintilla(QsciScintilla::SCI_POINTXFROMPOSITION, 0, currentPos);
    const int y = editor->SendScintilla(QsciScintilla::SCI_POINTYFROMPOSITION, 0, currentPos);
    const QPoint cursorGlobalPos = editor->mapToGlobal(QPoint(x, y));
    const int lineHeight = editor->SendScintilla(QsciScintilla::SCI_TEXTHEIGHT, 0);
    m_signatureHelpTooltip->showTooltip(signature);
    m_signatureHelpTooltip->move(cursorGlobalPos.x() - 2, cursorGlobalPos.y() - lineHeight);
}

// ScriptModule private
void ScriptModule::scriptFocus(ScriptPage *scriptPage, const bool status) {
    m_completionTooltip->hideTooltip();
    m_signatureHelpTooltip->hideTooltip();
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
    emit closeScript(scriptUrl);
}

void ScriptModule::textReplace(QString &text, const QString &kind) const {
    m_focusedPage->textReplace(text, kind);
}
