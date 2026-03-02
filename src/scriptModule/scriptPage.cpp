#include "scriptModule/scriptPage.h"

#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QJsonArray>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>
#include <QShortcut>
#include <QVBoxLayout>
#include <kddockwidgets/core/DockWidget.h>

#include "globals.h"
#include "scriptModule/codeEditor/editorWidget.h"
#include "scriptModule/codeEditor/scintillaWidget.h"
#include "scriptModule/codeEditor/searchWidget.h"
#include "utils/cmarkUtils.h"
#include "utils/qtUtils.h"

// public
ScriptPage::ScriptPage(const QJsonObject &scriptConfig, const QUrl &scriptUrl)
    : DockWidget(scriptUrl.toString()),
      m_editorWidget(new EditorWidget(scriptUrl)),
      m_scriptUrl(scriptUrl),
      m_selectionTimer(new QTimer(this)),
      m_contentTimer(new QTimer(this)),
      m_fileWatcher(new QFileSystemWatcher()),
      m_scintillaWidget(new ScintillaWidget(scriptUrl)),
      m_searchWidget(new SearchWidget()),
      m_completionTrigger{'.', ':', '\'', '"', '[', '#', '*', '@', '|', '=', '-', '{', '+', '?'},
      m_signatureHelpTrigger{'(', ','},
      m_onTypeFormattingTrigger{'\n'} {
    setTitle(scriptUrl.fileName());
    auto shortcutSearch = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this); // NOLINT
    connect(shortcutSearch, &QShortcut::activated, m_searchWidget, &SearchWidget::toggle);
    shortcutSearch->setContext(Qt::WidgetWithChildrenShortcut);
    auto shortcutFormatting = new QShortcut(QKeySequence(scriptConfig["formatting"].toString()), this); // NOLINT
    shortcutFormatting->setContext(Qt::WidgetWithChildrenShortcut);
    connect(shortcutFormatting, &QShortcut::activated, this, &ScriptPage::formattingRequest);

    // 100ms debounce for selection change
    m_selectionTimer->setSingleShot(true);
    m_selectionTimer->setInterval(100);
    connect(m_selectionTimer, &QTimer::timeout, this, &ScriptPage::selectionChange);
    // 500ms debounce for content change
    m_contentTimer->setSingleShot(true);
    m_contentTimer->setInterval(500);
    connect(m_contentTimer, &QTimer::timeout, this, &ScriptPage::contentChange);

    auto *widget = new QWidget(); // NOLINT
    setWidget(widget);
    auto *layout = new QVBoxLayout(widget); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_searchWidget);
    layout->addWidget(m_scintillaWidget);
    layout->addWidget(m_editorWidget);
    // font
    m_scintillaWidget->fontSet(QFont(scriptConfig["fontFamily"].toString(), scriptConfig["fontSize"].toInt()));
    // indicator
    {
        m_scintillaWidget->indicatorDefine(
            INDICATOR_ERROR,
            QJsonObject{
                {"style", 8},
                {"fore", 0xe6e6ff},
                {"alpha", 255},
                {"outlineAlpha", 255},
                {"setUnder", true}
            });
        m_scintillaWidget->indicatorDefine(
            INDICATOR_WARNING,
            QJsonObject{
                {"style", 8},
                {"fore", 0xe6f5ff},
                {"alpha", 255},
                {"outlineAlpha", 255},
                {"setUnder", true}
            });
        m_scintillaWidget->indicatorDefine(
            INDICATOR_INFO,
            QJsonObject{
                {"style", 8},
                {"fore", 0xfaf0e6},
                {"alpha", 255},
                {"outlineAlpha", 255},
                {"setUnder", true}
            });
        m_scintillaWidget->indicatorDefine(
            INDICATOR_HINT,
            QJsonObject{
                {"style", 8},
                {"fore", 0xf5f5f5},
                {"alpha", 255},
                {"outlineAlpha", 255},
                {"setUnder", true}
            });
        m_scintillaWidget->indicatorDefine(
            INDICATOR_TYPO,
            QJsonObject{
                {"style", 14},
                {"fore", 0xabd180},
                {"alpha", 255},
                {"outlineAlpha", 255},
                {"setUnder", true}
            });
        m_scintillaWidget->indicatorDefine(
            INDICATOR_HIGHLIGHT,
            QJsonObject{
                {"style", 8},
                {"fore", 0xe0e0e0},
                {"alpha", 255},
                {"outlineAlpha", 255},
                {"setUnder", true}
            });
        m_scintillaWidget->indicatorDefine(
            INDICATOR_READ,
            QJsonObject{
                {"style", 17},
                {"fore", 0xb85f00},
                {"alpha", 255},
                {"outlineAlpha", 255},
                {"setUnder", true}
            });
        m_scintillaWidget->indicatorDefine(
            INDICATOR_WRITE,
            QJsonObject{
                {"style", 17},
                {"fore", 0x2828c6},
                {"alpha", 255},
                {"outlineAlpha", 255},
                {"setUnder", true}
            });
        m_scintillaWidget->indicatorDefine(
            INDICATOR_SEARCH,
            QJsonObject{
                {"style", 8},
                {"fore", 0x7ed4fc},
                {"alpha", 255},
                {"outlineAlpha", 255},
                {"setUnder", true}
            });
        m_scintillaWidget->indicatorDefine(
            INDICATOR_SELECTION,
            QJsonObject{
                {"style", 8},
                {"fore", 0x3372c4},
                {"alpha", 255},
                {"outlineAlpha", 255},
                {"setUnder", true}
            });
        m_scintillaWidget->indicatorDefine(
            INDICATOR_HYPERLINK,
            QJsonObject{
                {"style", 17},
                {"fore", 0xcc6d00},
                {"setUnder", true}
            });
    }
    // margin
    {
        m_scintillaWidget->marginDefine(
            MARGIN_NUMBER,
            QJsonObject{
                {"type", 1},
                {"width", 32}
            });
        m_scintillaWidget->marginDefine(
            MARGIN_BREAKPOINT,
            QJsonObject{
                {"type", 0},
                {"width", 16},
                {"mask", static_cast<int>(~SC_MASK_FOLDERS & ~SC_MASK_HISTORY)},
                {"sensitive", true}
            });
        m_scintillaWidget->marginDefine(
            MARGIN_FOLDERS,
            QJsonObject{
                {"type", 0},
                {"width", 16},
                {"mask", static_cast<int>(SC_MASK_FOLDERS)},
                {"sensitive", true}
            });
        m_scintillaWidget->marginDefine(
            MARGIN_HISTORY,
            QJsonObject{
                {"type", 0},
                {"width", 4},
                {"mask", SC_MASK_HISTORY},
            });
    }
    // marker
    {
        m_scintillaWidget->markerDefine(
            MARKER_REGION,
            QJsonObject{
                {"symbol", 2},
                {"fore", 0x00ffff},
                {"back", 0x00ffff}
            });
        m_scintillaWidget->markerDefine(
            MARKER_BREAKPOINT,
            QJsonObject{
                {"symbol", 0},
                {"fore", 0x0000ff},
                {"back", 0x0000ff}
            });
        m_scintillaWidget->markerDefine(
            MARKER_DEBUG,
            QJsonObject{
                {"symbol", 2},
                {"fore", 0x000000},
                {"back", 0xa500ff}
            });
        m_scintillaWidget->markerDefine(
            MARKER_ERROR,
            QJsonObject{
                {"symbol", 22},
                {"fore", 0xffe6e6},
                {"back", 0xffe6e6}
            });
        m_scintillaWidget->markerDefine(
            MARKER_HINT,
            QJsonObject{
                {"symbol", 22},
                {"fore", 0xffe6e6},
                {"back", 0xffe6e6}
            });
    }
    // style
    {
        m_scintillaWidget->styleDefine(
            LUA_TOKEN_NAMESPACE,
            QJsonObject{
                {"fore", 0x808000}
            });
        m_scintillaWidget->styleDefine(
            LUA_TOKEN_CLASS,
            QJsonObject{
                {"fore", 0x808000}
            });
        m_scintillaWidget->styleDefine(
            LUA_TOKEN_TYPE,
            QJsonObject{
                {"fore", 0xb33300}
            });
        m_scintillaWidget->styleDefine(
            LUA_TOKEN_PARAMETER,
            QJsonObject{
                {"fore", 0x000000}
            });
        m_scintillaWidget->styleDefine(
            LUA_TOKEN_VARIABLE,
            QJsonObject{
                {"fore", 0x000000}
            });
        m_scintillaWidget->styleDefine(
            LUA_TOKEN_PROPERTY,
            QJsonObject{
                {"fore", 0x7a0e66}
            });
        m_scintillaWidget->styleDefine(
            LUA_TOKEN_ENUMMEMBAER,
            QJsonObject{
                {"fore", 0x941087}
            });
        m_scintillaWidget->styleDefine(
            LUA_TOKEN_FUNCTION_DECLARATION,
            QJsonObject{
                {"fore", 0x7a6200}
            });
        m_scintillaWidget->styleDefine(
            LUA_TOKEN_FUNCTION_CALL,
            QJsonObject{
                {"fore", 0x000000}
            });
        m_scintillaWidget->styleDefine(
            LUA_TOKEN_METHOD,
            QJsonObject{
                {"fore", 0x000000}
            });
        m_scintillaWidget->styleDefine(
            LUA_TOKEN_MACRO,
            QJsonObject{
                {"fore", 0x2e541f}
            });
        m_scintillaWidget->styleDefine(
            LUA_TOKEN_KEYWORD,
            QJsonObject{
                {"fore", 0xb33300}
            });
        m_scintillaWidget->styleDefine(
            LUA_TOKEN_COMMENT,
            QJsonObject{
                {"fore", 0x8c8c8c}
            });
        m_scintillaWidget->styleDefine(
            LUA_TOKEN_STRING,
            QJsonObject{
                {"fore", 0x177d06}
            });
        m_scintillaWidget->styleDefine(
            LUA_TOKEN_NUMBER,
            QJsonObject{
                {"fore", 0xeb5017}
            });
        m_scintillaWidget->styleDefine(
            LUA_TOKEN_OPERATOR,
            QJsonObject{
                {"fore", 0x000000}
            });
    }
    // script
    const QUrl &url(m_scriptUrl);
    const QString scriptPath = url.toLocalFile();
    QFile file(scriptPath);
    if (!file.open(QIODevice::ReadOnly)) return;
    QTextStream in(&file);
    const QString script = in.readAll();
    file.close();
    m_scintillaWidget->textSet(script);
    m_scintillaWidget->savepointSet();
    // signals
    connect(m_scintillaWidget, &ScintillaEdit::charAdded, this, &ScriptPage::charAdd);
    connect(m_scintillaWidget, &ScintillaEdit::marginClicked, this, &ScriptPage::marginClick);
    connect(m_scintillaWidget, &ScintillaEdit::updateUi, this, &ScriptPage::uiUpdate);
    // TODO: delete later
    {
        // font
        m_editorWidget->setFont(QFont(scriptConfig["fontFamily"].toString(), scriptConfig["fontSize"].toInt()));
        // indicator diagnostic
        m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorErrorStyle"].toInt()), INDICATOR_ERROR);
        m_editorWidget->setIndicatorForegroundColor(QColor(scriptConfig["indicatorErrorColor"].toString()), INDICATOR_ERROR);
        m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_ERROR);
        m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorWarningStyle"].toInt()), INDICATOR_WARNING);
        m_editorWidget->setIndicatorForegroundColor(QColor(scriptConfig["indicatorWarningColor"].toString()), INDICATOR_WARNING);
        m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_WARNING);
        m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorInfoStyle"].toInt()), INDICATOR_INFO);
        m_editorWidget->setIndicatorForegroundColor(QColor(scriptConfig["indicatorInfoColor"].toString()), INDICATOR_INFO);
        m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_INFO);
        m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorHintStyle"].toInt()), INDICATOR_HINT);
        m_editorWidget->setIndicatorForegroundColor(QColor(scriptConfig["indicatorHintColor"].toString()), INDICATOR_HINT);
        m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_HINT);
        m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorTypoStyle"].toInt()), INDICATOR_TYPO);
        m_editorWidget->setIndicatorForegroundColor(QColor(scriptConfig["indicatorTypoColor"].toString()), INDICATOR_TYPO);
        m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_TYPO);
        // indicator highlight
        m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorHighlightStyle"].toInt()), INDICATOR_HIGHLIGHT);
        m_editorWidget->setIndicatorForegroundColor(QColor(scriptConfig["indicatorHighlightColor"].toString()), INDICATOR_HIGHLIGHT);
        m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_HIGHLIGHT);
        // TODO: looking for a better solution
        m_editorWidget->SendScintilla(SCI_INDICSETALPHA, INDICATOR_HIGHLIGHT, 127);
        m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorReadStyle"].toInt()), INDICATOR_READ);
        m_editorWidget->setIndicatorForegroundColor(QColor(scriptConfig["indicatorReadColor"].toString()), INDICATOR_READ);
        m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_READ);
        m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorWriteStyle"].toInt()), INDICATOR_WRITE);
        m_editorWidget->setIndicatorForegroundColor(QColor(scriptConfig["indicatorWriteColor"].toString()), INDICATOR_WRITE);
        m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_WRITE);
        // indicator search
        m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorSearchStyle"].toInt()), INDICATOR_SEARCH);
        m_editorWidget->setIndicatorForegroundColor(QColor(scriptConfig["indicatorSearchColor"].toString()), INDICATOR_SEARCH);
        m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_SEARCH);
        m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorSelectionStyle"].toInt()), INDICATOR_SELECTION);
        m_editorWidget->setIndicatorForegroundColor(QColor(scriptConfig["indicatorSelectionColor"].toString()), INDICATOR_SELECTION);
        m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_SELECTION);
        // indicator hyperlink
        m_editorWidget->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorHyperlinkStyle"].toInt()), INDICATOR_HYPERLINK);
        m_editorWidget->setIndicatorForegroundColor(QColor(scriptConfig["indicatorHyperlinkColor"].toString()), INDICATOR_HYPERLINK);
        m_editorWidget->setIndicatorDrawUnder(true, INDICATOR_HYPERLINK);
        // marker
        m_editorWidget->markerDefine(static_cast<QsciScintilla::MarkerSymbol>(scriptConfig["markerBreakpointStyle"].toInt()), MARKER_BREAKPOINT);
        m_editorWidget->setMarkerBackgroundColor(QColor(scriptConfig["markerBreakpointBackground"].toString()), MARKER_BREAKPOINT);
        m_editorWidget->setMarkerForegroundColor(QColor(scriptConfig["markerBreakpointForeground"].toString()), MARKER_BREAKPOINT);
        m_editorWidget->markerDefine(static_cast<QsciScintilla::MarkerSymbol>(scriptConfig["markerDebugStyle"].toInt()), MARKER_DEBUG);
        m_editorWidget->setMarkerBackgroundColor(QColor(scriptConfig["markerDebugBackground"].toString()), MARKER_DEBUG);
        m_editorWidget->setMarkerForegroundColor(QColor(scriptConfig["markerDebugForeground"].toString()), MARKER_DEBUG);

        const QUrl &url(m_scriptUrl);
        const QString scriptPath = url.toLocalFile();
        // read-only check
        if (const QFileInfo fileInfo(scriptPath); !fileInfo.isWritable()) {
            scriptReadonly(true);
        }
        // load script
        QFile file(scriptPath);
        file.open(QIODevice::ReadOnly);
        QTextStream in(&file);
        const QString content = in.readAll();
        file.close();
        m_fileWatcher->addPath(scriptPath);
        m_editorWidget->setText(content);
        // connect signals
        connect(m_editorWidget, &EditorWidget::showMenu, this, [this](const QVariantHash &menuSession) { emit showMenu(m_scriptUrl, menuSession); });
        connect(m_editorWidget, &EditorWidget::requestPermission, this, &ScriptPage::permissionRequest);
        connect(m_editorWidget, &EditorWidget::hideDwellWidget, this, &ScriptPage::hideDwell);
        connect(m_editorWidget, &EditorWidget::requestDefinition, this, &ScriptPage::definitionRequest);
        connect(m_editorWidget, &EditorWidget::requestDocumentHighlight, this, &ScriptPage::documentHighlightRequest);
        connect(m_editorWidget, &EditorWidget::requestHover, this, &ScriptPage::hoverRequest);
        connect(m_editorWidget, &EditorWidget::requestImplementation, this, &ScriptPage::implementationRequest);
        connect(m_editorWidget, &EditorWidget::requestOnTypeFormatting, this, &ScriptPage::onTypeFormattingRequest);
        connect(m_editorWidget, &EditorWidget::requestReferences, this, &ScriptPage::referencesRequest);
        connect(m_editorWidget, &EditorWidget::requestTypeDefinition, this, &ScriptPage::typeDefinitionRequest);
        connect(m_editorWidget, &EditorWidget::setStat, m_searchWidget, &SearchWidget::statSet);
        connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &ScriptPage::scriptReload);
        connect(m_searchWidget, &SearchWidget::searchText, m_editorWidget, &EditorWidget::textSearch);
        connect(m_searchWidget, &SearchWidget::searchPrev, m_editorWidget, &EditorWidget::prevSearch);
        connect(m_searchWidget, &SearchWidget::searchNext, m_editorWidget, &EditorWidget::nextSearch);
        connect(m_searchWidget, &SearchWidget::replaceText, m_editorWidget, qOverload<const QString &>(&EditorWidget::textReplace));
        connect(m_searchWidget, &SearchWidget::replaceAllText, m_editorWidget, &EditorWidget::textReplaceAll);
    }
    QTimer::singleShot(0, this, [this] {
        // lsp
        didOpenNotification();
        // logging
        emit appendLog(QString("<a href='%1'>%2</a> opened").arg(m_scriptUrl.toString(), m_scriptUrl.toString()), "info");
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 opened").arg(timestamp, m_scriptUrl.toString());
    });
}

// public: file
void ScriptPage::pathDisambiguation() {
    const QString scriptPath = m_scriptUrl.toLocalFile();
    const QString workspacePath = g_workspaceUrl.toLocalFile();
    const QString relatedPath = QDir(workspacePath).relativeFilePath(scriptPath);
    setTitle(relatedPath);
}

void ScriptPage::scriptReload() {
    const QMessageBox::StandardButton reply = QMessageBox::question(
        nullptr,
        tr("Reload"),
        QString(tr("%1\n\n"
            "This file has been modified by another program.\n"
            "Do you want to reload it?")).arg(m_scriptUrl.toString()),
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }
    // reload new script
    const QUrl &url(m_scriptUrl);
    const QString scriptPath = url.toLocalFile();
    QFile file(scriptPath);
    file.open(QIODevice::ReadOnly);
    QTextStream in(&file);
    const QString content = in.readAll();
    file.close();
    m_editorWidget->setText(content);
    // logging
    emit appendLog(QString("<a href='%1'>%2</a> reloaded").arg(m_scriptUrl.toString(), m_scriptUrl.fileName()), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 reloaded").arg(timestamp, m_scriptUrl.fileName());
}

void ScriptPage::scriptSave() {
    if (!m_scintillaWidget->modifyGet()) return;
    // update status
    m_scintillaWidget->savepointSet();
    scriptModify();
    didSaveNotification();
    // block file watcher signals
    m_fileWatcher->blockSignals(true);
    // save file
    const QString scriptPath = m_scriptUrl.toLocalFile();
    QFile file(scriptPath);
    if (!file.open(QIODevice::WriteOnly)) return;
    QTextStream out(&file);
    out << m_scintillaWidget->textGet();
    file.close();
    // logging
    emit appendLog(QString("<a href='%1'>%2</a> saved").arg(m_scriptUrl.toString(), m_scriptUrl.toString()), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 saved").arg(timestamp, m_scriptUrl.toString());
    // restore file watcher signals 1 sec later
    QTimer::singleShot(1000, this, [this] { m_fileWatcher->blockSignals(false); });
}

void ScriptPage::scriptClose() {
    // ask for saving
    if (m_scintillaWidget->modifyGet()) {
        const QMessageBox::StandardButton reply = QMessageBox::question(
            nullptr,
            tr("Close Script"),
            tr("The script has been edited. Save changes?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            scriptSave();
        }
    }
    didCloseNotification();
    emit closeScript(m_scriptUrl);
    deleteLater();
    // logging
    emit appendLog(QString("<a href='%1'>%2</a> closed").arg(m_scriptUrl.toString(), m_scriptUrl.toString()), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 closed").arg(timestamp, m_scriptUrl.toString());
}

// public: lsp
void ScriptPage::diagnosticsResponse(const QJsonArray &diagnostics) {
    if (!m_scriptUrl.toString().endsWith(".lua")) return;
    m_scriptDiagnostic = diagnostics;
    // clear
    m_scintillaWidget->indicatorClear(INDICATOR_ERROR);
    m_scintillaWidget->indicatorClear(INDICATOR_WARNING);
    m_scintillaWidget->indicatorClear(INDICATOR_INFO);
    m_scintillaWidget->indicatorClear(INDICATOR_HINT);
    // publish
    for (const auto &value: diagnostics) {
        const QJsonObject diagnostic = value.toObject();
        const int severity = diagnostic["severity"].toInt();
        const QJsonObject range = diagnostic["range"].toObject();
        const QJsonObject start = range["start"].toObject();
        const QJsonObject end = range["end"].toObject();
        const int startLine = start["line"].toInt();
        const int startCharacter = start["character"].toInt();
        const int endLine = end["line"].toInt();
        const int endCharacter = end["character"].toInt();
        m_scintillaWidget->indicatorFill(severity, startLine, startCharacter, endLine, endCharacter);
    }
}

void ScriptPage::documentHighlightResponse(const QJsonArray &result) const {
    // clear previous highlight
    m_scintillaWidget->indicatorClear(INDICATOR_HIGHLIGHT);
    m_scintillaWidget->indicatorClear(INDICATOR_READ);
    m_scintillaWidget->indicatorClear(INDICATOR_WRITE);
    // highlight
    for (const auto &highlight: result) {
        const QJsonObject highlightObject = highlight.toObject();
        const int kind = highlightObject["kind"].toInt();
        const QJsonObject highlightRange = highlightObject["range"].toObject();
        const QJsonObject highlightStartPos = highlightRange["start"].toObject();
        const QJsonObject highlightEndPos = highlightRange["end"].toObject();
        const int startLine = highlightStartPos["line"].toInt();
        const int startCharacter = highlightStartPos["character"].toInt();
        const int endLine = highlightEndPos["line"].toInt();
        const int endCharacter = highlightEndPos["character"].toInt();
        m_scintillaWidget->indicatorFill(INDICATOR_HIGHLIGHT, startLine, startCharacter, endLine, endCharacter);
        if (kind == 2) m_scintillaWidget->indicatorFill(INDICATOR_READ, startLine, startCharacter, endLine, endCharacter);
        else if (kind == 3) m_scintillaWidget->indicatorFill(INDICATOR_WRITE, startLine, startCharacter, endLine, endCharacter);
    }
}

void ScriptPage::foldingRangeResponse(const QJsonArray &result) const {
    QHash<int, int> deltaDepthHash{};
    for (const auto &value: result) {
        const QJsonObject valueObject = value.toObject();
        const int startLine = valueObject["startLine"].toInt();
        const int endLine = valueObject["endLine"].toInt();
        deltaDepthHash.insert(startLine + 1, deltaDepthHash.value(startLine + 1, 0) + 1);
        deltaDepthHash.insert(endLine + 2, deltaDepthHash.value(endLine + 2, 0) - 1);
    }
    int depth = 0;
    for (int line = 0; line < m_scintillaWidget->lineCountGet(); line++) {
        const int deltaDepth = deltaDepthHash.value(line, 0);
        depth += deltaDepth;
        int level = SC_FOLDLEVELBASE + depth;
        if (deltaDepthHash.value(line + 1, 0) > 0) level |= SC_FOLDLEVELHEADERFLAG;
        m_scintillaWidget->foldLevelSet(line, level);
    }
}

void ScriptPage::formattingResponse(const QString &newText) const {
    m_scintillaWidget->textSet(newText);
}

void ScriptPage::onTypeFormattingResponse(const QJsonObject &newText) const {
    const QString text = newText["newText"].toString();
    const QJsonObject range = newText["range"].toObject();
    const QJsonObject start = range["start"].toObject();
    const QJsonObject end = range["end"].toObject();
    const int startLine = start["line"].toInt();
    const int startCharacter = start["character"].toInt();
    const int endLine = end["line"].toInt();
    const int endCharacter = end["character"].toInt();
    m_scintillaWidget->textSet(text, startLine, startCharacter, endLine, endCharacter);
}

void ScriptPage::semanticTokensResponse(const QJsonArray &data) const {
    // clear
    m_scintillaWidget->styleSet(LUA_TOKEN_UNUSED);
    // publish
    int line = 0;
    int character = 0;
    for (int i = 0; i < data.size(); i += 5) {
        const int deltaLine = data[i].toInt();
        const int deltaCharacter = data[i + 1].toInt();
        const int length = data[i + 2].toInt();
        const int tokenType = data[i + 3].toInt();
        const int tokenModifiers = data[i + 4].toInt();
        line += deltaLine;
        character = deltaLine > 0 ? deltaCharacter : character + deltaCharacter;
        int type{};
        switch (tokenType) {
            case TOKENTYPE_NAMESPACE:
                type = LUA_TOKEN_NAMESPACE;
                break;
            case TOKENTYPE_CLASS:
                type = LUA_TOKEN_CLASS;
                break;
            case TOKENTYPE_TYPE:
                type = LUA_TOKEN_TYPE;
                break;
            case TOKENTYPE_PARAMETER:
                type = LUA_TOKEN_PARAMETER;
                break;
            case TOKENTYPE_VARIABLE:
                type = LUA_TOKEN_VARIABLE;
                break;
            case TOKENTYPE_PROPERTY:
                type = LUA_TOKEN_PROPERTY;
                break;
            case TOKENTYPE_ENUMMEMBAER:
                type = LUA_TOKEN_ENUMMEMBAER;
                break;
            case TOKENTYPE_FUNCTION:
                if (tokenModifiers == TOKENMODIFIERS_DECLARATION || tokenModifiers == TOKENMODIFIERS_GLOBAL) type = LUA_TOKEN_FUNCTION_DECLARATION;
                else type = LUA_TOKEN_FUNCTION_CALL;
                break;
            case TOKENTYPE_METHOD:
                type = LUA_TOKEN_METHOD;
                break;
            case TOKENTYPE_MACRO:
                type = LUA_TOKEN_MACRO;
                break;
            case TOKENTYPE_KEYWORD:
                type = LUA_TOKEN_KEYWORD;
                break;
            case TOKENTYPE_COMMENT:
                type = LUA_TOKEN_COMMENT;
                break;
            case TOKENTYPE_STRING:
                type = LUA_TOKEN_STRING;
                break;
            case TOKENTYPE_NUMBER:
                type = LUA_TOKEN_NUMBER;
                break;
            case TOKENTYPE_OPERATOR:
                type = LUA_TOKEN_OPERATOR;
                break;
            default:
                qDebug() << "skip token" << line << character << length << tokenType;
                break;
        }
        m_scintillaWidget->styleSet(type, line, character, length);
    }
}

// public: typo
void ScriptPage::spellCheckResponse(const QVariantList &typos) {
    m_scriptTypo = typos;
    // clear
    m_scintillaWidget->indicatorClear(INDICATOR_TYPO);
    // publish
    for (const auto &value: typos) {
        auto typo = value.toMap();
        const int startLine = typo["line"].toInt();
        const int endLine = typo["line"].toInt();
        const int startCharacter = typo["startCharacter"].toInt();
        const int endCharacter = typo["endCharacter"].toInt();
        m_scintillaWidget->indicatorFill(INDICATOR_TYPO, startLine, startCharacter, endLine, endCharacter);
    }
}

// public: slot
void ScriptPage::charAdd(const int ch) {
    m_selection = m_scintillaWidget->selectionGet();
    const QChar character(ch);
    if (character.isLetter() || m_completionTrigger.contains(character)) {
        didChangeNotification();
        completionRequest();
    } else if (m_signatureHelpTrigger.contains(character)) {
        didChangeNotification();
        completionRequest();
        signatureHelpRequest();
    } else if (m_onTypeFormattingTrigger.contains(character)) {
        didChangeNotification();
        onTypeFormattingRequest();
    }
}

// protected
void ScriptPage::closeEvent(QCloseEvent *event) {
    scriptClose();
    event->accept();
}

// private: slot
void ScriptPage::marginClick(const Scintilla::Position position, Scintilla::KeyMod modifiers, const int margin) {
    const int line = m_scintillaWidget->lineGet(position);
    if (margin == MARGIN_BREAKPOINT) {
        if (m_scintillaWidget->markerGet(line) & 1 << MARKER_REGION) {
            const int startPos = m_editorWidget->positionFromLineIndex(line + 1, 0);
            for (int current = line; current < m_editorWidget->lines(); ++current) {
                const QString lineText = m_editorWidget->text(current);
                if (lineText.contains("--#endregion")) {
                    const int endPos = m_editorWidget->positionFromLineIndex(current, 0);
                    qDebug() << m_editorWidget->text(startPos, endPos);
                    return;
                }
            }
            qDebug() << "error: --#endregion not found";
        } else if (m_scintillaWidget->markerGet(line) & 1 << MARKER_BREAKPOINT) {
            emit removeBreakpoint(m_scriptUrl, line + 1);
            m_scintillaWidget->markerDelete(MARKER_BREAKPOINT, line);
        } else {
            emit insertBreakpoint(m_scriptUrl, line + 1, QVariantHash());
            m_scintillaWidget->markerAdd(MARKER_BREAKPOINT, line);
        }
    }
}

void ScriptPage::uiUpdate(const Scintilla::Update updated) const {
    if (updated == Scintilla::Update::Content) {
        m_contentTimer->start();
    } else if (updated == Scintilla::Update::Selection) {
        m_selectionTimer->start();
    }
}

void ScriptPage::selectionChange() {
    m_selection = m_scintillaWidget->selectionGet();
    emit changeSelection(m_selection);
    if (m_selection["lines"] == 0 && m_selection["characters"] == 0) {
        documentHighlightRequest();
    }
}

void ScriptPage::contentChange() {
    m_selection = m_scintillaWidget->selectionGet();
    // lsp request
    didChangeNotification();
    documentHighlightRequest();
    documentSymbolRequest();
    foldingRangeRequest();
    semanticTokensRequest();
    // nuspell request
    spellCheckRequest();
    // modification check
    scriptModify();
}

// private: file
void ScriptPage::scriptReadonly(const bool status) {
    m_readonly = status;
    m_editorWidget->setReadOnly(status);
    if (status) {
        setIcon(QIcon(":/icon/lockClosed.svg"));
    } else {
        setIcon(QIcon());
    }
}

void ScriptPage::scriptModify() {
    const QString pageName = title();
    if (m_scintillaWidget->modifyGet() && !pageName.endsWith('*')) {
        setTitle(pageName + "*");
    } else if (!m_scintillaWidget->modifyGet() && pageName.endsWith('*')) {
        setTitle(pageName.chopped(1));
    }
}

void ScriptPage::permissionRequest() {
    const QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("Warning"),
        tr("This file is read-only. Would you like to make it writable?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }
    // update status
    scriptReadonly(false);
    // block file watcher signals
    m_fileWatcher->blockSignals(true);
    const QString scriptPath = m_scriptUrl.toLocalFile();
    QFile::setPermissions(
        scriptPath,
        QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    // logging
    emit appendLog(QString("<a href='%1'>%2</a> permitted").arg(m_scriptUrl.toString(), m_scriptUrl.fileName()), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 permitted").arg(timestamp, m_scriptUrl.fileName());
    // restore file watcher signals 1 sec later
    QTimer::singleShot(1000, this, [this] { m_fileWatcher->blockSignals(false); });
}

// private: lsp
void ScriptPage::didOpenNotification() {
    // did open notification to lua language server
    const QJsonObject didOpenParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()},
                {"languageId", "lua"},
                {"version", m_version++},
                {"text", m_editorWidget->text()}
            }
        }
    };
    emit notificationJson("textDocument/didOpen", didOpenParams);
}

void ScriptPage::didChangeNotification() {
    // did change notification to lua language server
    const auto content = m_scintillaWidget->textGet();
    const QJsonObject didChangeParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()},
                {"version", m_version++}
            }
        },
        {
            "contentChanges", QJsonArray{
                QJsonObject{
                    {"text", content}
                }
            }
        }
    };
    emit notificationJson("textDocument/didChange", didChangeParams);
}

void ScriptPage::didSaveNotification() {
    // did save notification to lua language server
    const QJsonObject didSaveParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()}
            }
        }
    };
    emit notificationJson("textDocument/didSave", didSaveParams);
}

void ScriptPage::didCloseNotification() {
    // did close notification to lua language server
    const QJsonObject didCloseParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()}
            }
        }
    };
    emit notificationJson("textDocument/didClose", didCloseParams);
}

void ScriptPage::completionRequest() {
    // completion request to script module
    emit requestCompletion(m_scriptUrl, m_selection["line"], m_selection["character"]);
}

void ScriptPage::definitionRequest() {
    // get cursor position
    int line, character;
    m_editorWidget->getCursorPosition(&line, &character);
    // definition request to script module
    emit requestDefinition(m_scriptUrl, line, character);
}

void ScriptPage::documentHighlightRequest() {
    // document highlight request to script module
    emit requestDocumentHighlight(m_scriptUrl, m_selection["line"], m_selection["character"]);
}

void ScriptPage::documentSymbolRequest() {
    // document symbol request to script module
    emit requestDocumentSymbol(m_scriptUrl);
}

void ScriptPage::foldingRangeRequest() {
    // folding range request to script module
    emit requestFoldingRange(m_scriptUrl);
}

void ScriptPage::formattingRequest() {
    // formatting request to script module
    emit requestFormatting(m_scriptUrl);
}

void ScriptPage::hoverRequest() {
    // get mouse position
    const QPoint globalPos = QCursor::pos();
    const QPoint localPos = mapFromGlobal(globalPos);
    if (!rect().contains(localPos)) return;
    // get cursor position
    const long charPos = m_editorWidget->SendScintilla(SCI_POSITIONFROMPOINTCLOSE, localPos.x(), localPos.y());
    if (charPos == -1) return;
    int line, character;
    m_editorWidget->lineIndexFromPosition(charPos, &line, &character);
    if (line == 0 && character == 0) return;
    // show diagnostic if exists
    QString diagnosticText = "<table width='100%'>";
    for (const auto &value: m_scriptDiagnostic) {
        const QJsonObject diagnostic = value.toObject();
        const QJsonObject range = diagnostic["range"].toObject();
        const QJsonObject start = range["start"].toObject();
        const QJsonObject end = range["end"].toObject();
        const int startLine = start["line"].toInt();
        const int startCharacter = start["character"].toInt();
        const int endLine = end["line"].toInt();
        const int endCharacter = end["character"].toInt();
        if (line >= startLine && line <= endLine && character >= startCharacter && character <= endCharacter) {
            const int severity = diagnostic["severity"].toInt();
            QString severityString{};
            switch (severity) {
                case 1: {
                    severityString = "Error";
                }
                break;
                case 2: {
                    severityString = "Warning";
                }
                break;
                case 3: {
                    severityString = "Info";
                }
                break;
                case 4: {
                    severityString = "Hint";
                }
                break;
                default: break;
            }
            const QString message = diagnostic["message"].toString();
            // qDebug() << message << parsed;
            const QString commandLine = QString("requestcodeaction://codeAction/%2/%3/%4/%5").arg(
                QString::number(startLine), QString::number(startCharacter), QString::number(endLine), QString::number(endCharacter));
            diagnosticText += QString("<tr><td><b>%1</b>: %2</td><td align='right'><a href='%3'>Code Action</a></td></tr>").arg(severityString, md2html(message), commandLine);
        }
    }
    // show typo if exists
    for (const auto &value: m_scriptTypo) {
        auto typo = value.toMap();
        const int lineFrom = typo["line"].toInt();
        const int lineTo = typo["line"].toInt();
        const int indexFrom = typo["indexFrom"].toInt();
        const int indexTo = typo["indexTo"].toInt();
        if (line >= lineFrom && line <= lineTo && character >= indexFrom && character <= indexTo) {
            const int startPos = m_editorWidget->positionFromLineIndex(lineFrom, indexFrom);
            const int endPos = m_editorWidget->positionFromLineIndex(lineTo, indexTo);
            const QString word = m_editorWidget->text(startPos, endPos);
            const QString commandLine = QString("requestspellsuggest://%1/%2/%3/%4/%5").arg(
                word, QString::number(lineFrom), QString::number(indexFrom), QString::number(lineTo), QString::number(indexTo));
            diagnosticText += QString("<tr><td><b>Typo</b>: In word '%1'</td><td align='right'><a href='%2'>Show Suggestions</a></td></tr>").arg(word, commandLine);
        }
    }
    // call diagnostic show
    const QPoint position = m_editorWidget->window()->mapFromGlobal(QCursor::pos() + QPoint(10, 10));
    const QVariantHash diagnosticSession = {
        {"scriptUrl", m_scriptUrl},
        {"position", position}
    };
    if (diagnosticText == "<table width='100%'>") {
        emit showDiagnostic(diagnosticSession, "");
    } else {
        diagnosticText += "</table>";
        emit showDiagnostic(diagnosticSession, diagnosticText);
    }
    // hover request to script module
    emit requestHover(m_scriptUrl, line, character);
}

void ScriptPage::implementationRequest() {
    // get cursor position
    int line, character;
    m_editorWidget->getCursorPosition(&line, &character);
    // implementation request to script module
    emit requestImplementation(m_scriptUrl, line, character);
}

void ScriptPage::referencesRequest() {
    // get cursor position
    int line, character;
    m_editorWidget->getCursorPosition(&line, &character);
    // references request to script module
    emit requestReferences(m_scriptUrl, line, character);
}

void ScriptPage::onTypeFormattingRequest() {
    // on type formatting request to script module
    emit requestOnTypeFormatting(m_scriptUrl, m_selection["line"], m_selection["character"]);
}

void ScriptPage::semanticTokensRequest() {
    // semantic tokens request to script module
    emit requestSemanticTokens(m_scriptUrl);
}

void ScriptPage::signatureHelpRequest() {
    // signature help request to script module
    emit requestSignatureHelp(m_scriptUrl, m_selection["line"], m_selection["character"]);
}

void ScriptPage::typeDefinitionRequest() {
    // get cursor position
    int line, character;
    m_editorWidget->getCursorPosition(&line, &character);
    // type definition request to script module
    emit requestTypeDefinition(m_scriptUrl, line, character);
}

// private: typo
void ScriptPage::spellCheckRequest() {
    if (!m_scriptUrl.toString().endsWith(".lua")) return;
    // spell check request to script module
    emit requestSpellCheck(m_scriptUrl, m_scintillaWidget->textGet());
}

// private:
void ScriptPage::positionFill(const int x, const int y) const {
    const QString text = QString("%1, %2").arg(QString::number(x), QString::number(y));
    m_editorWidget->insert(text);
    const long currentPos = m_editorWidget->SendScintilla(SCI_GETCURRENTPOS);
    const long cursorPos = currentPos + text.length();
    m_editorWidget->SendScintilla(SCI_SETCURRENTPOS, cursorPos); // NOLINT
    m_editorWidget->SendScintilla(SCI_SETSELECTIONSTART, cursorPos); // NOLINT
    m_editorWidget->SendScintilla(SCI_SETSELECTIONEND, cursorPos); // NOLINT
}
