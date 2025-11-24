#include "scriptModule/scriptPage.h"

#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>
#include <kddockwidgets/core/DockWidget.h>
#include <kddockwidgets/core/Group.h>

#include "globals.h"
#include "scriptModule/scriptEditor.h"
#include "utils/qtUtils.h"

// ScriptPage public
ScriptPage::ScriptPage(const QJsonObject &scriptConfig, const QUrl &scriptUrl)
    : DockWidget(scriptUrl.toString()),
      m_scriptEditor(new ScriptEditor()),
      m_scriptUrl(scriptUrl),
      m_fileWatcher(new QFileSystemWatcher()),
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

    auto *widget = new QWidget(); // NOLINT
    setWidget(widget);
    auto *layout = new QVBoxLayout(widget); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_searchWidget);
    layout->addWidget(m_scriptEditor);
    // font
    m_scriptEditor->setFont(QFont(scriptConfig["fontFamily"].toString(), scriptConfig["fontSize"].toInt()));
    // indicator diagnostic
    m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorErrorStyle"].toInt()), INDICATOR_ERROR);
    m_scriptEditor->setIndicatorForegroundColor(QColor(scriptConfig["indicatorErrorColor"].toString()), INDICATOR_ERROR);
    m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_ERROR);
    m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorWarningStyle"].toInt()), INDICATOR_WARNING);
    m_scriptEditor->setIndicatorForegroundColor(QColor(scriptConfig["indicatorWarningColor"].toString()), INDICATOR_WARNING);
    m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_WARNING);
    m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorInfoStyle"].toInt()), INDICATOR_INFO);
    m_scriptEditor->setIndicatorForegroundColor(QColor(scriptConfig["indicatorInfoColor"].toString()), INDICATOR_INFO);
    m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_INFO);
    m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorHintStyle"].toInt()), INDICATOR_HINT);
    m_scriptEditor->setIndicatorForegroundColor(QColor(scriptConfig["indicatorHintColor"].toString()), INDICATOR_HINT);
    m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_HINT);
    m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorTypoStyle"].toInt()), INDICATOR_TYPO);
    m_scriptEditor->setIndicatorForegroundColor(QColor(scriptConfig["indicatorTypoColor"].toString()), INDICATOR_TYPO);
    m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_TYPO);
    // indicator highlight
    m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorHighlightStyle"].toInt()), INDICATOR_HIGHLIGHT);
    m_scriptEditor->setIndicatorForegroundColor(QColor(scriptConfig["indicatorHighlightColor"].toString()), INDICATOR_HIGHLIGHT);
    m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_HIGHLIGHT);
    m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorReadStyle"].toInt()), INDICATOR_READ);
    m_scriptEditor->setIndicatorForegroundColor(QColor(scriptConfig["indicatorReadColor"].toString()), INDICATOR_READ);
    m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_READ);
    m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorWriteStyle"].toInt()), INDICATOR_WRITE);
    m_scriptEditor->setIndicatorForegroundColor(QColor(scriptConfig["indicatorWriteColor"].toString()), INDICATOR_WRITE);
    m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_WRITE);
    // indicator search
    m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorSearchStyle"].toInt()), INDICATOR_SEARCH);
    m_scriptEditor->setIndicatorForegroundColor(QColor(scriptConfig["indicatorSearchColor"].toString()), INDICATOR_SEARCH);
    m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_SEARCH);
    m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorSelectionStyle"].toInt()), INDICATOR_SELECTION);
    m_scriptEditor->setIndicatorForegroundColor(QColor(scriptConfig["indicatorSelectionColor"].toString()), INDICATOR_SELECTION);
    m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_SELECTION);
    // indicator hyperlink
    m_scriptEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(scriptConfig["indicatorHyperlinkStyle"].toInt()), INDICATOR_HYPERLINK);
    m_scriptEditor->setIndicatorForegroundColor(QColor(scriptConfig["indicatorHyperlinkColor"].toString()), INDICATOR_HYPERLINK);
    m_scriptEditor->setIndicatorDrawUnder(true, INDICATOR_HYPERLINK);
    // marker
    m_scriptEditor->markerDefine(static_cast<QsciScintilla::MarkerSymbol>(scriptConfig["markerBreakpointStyle"].toInt()), MARKER_BREAKPOINT);
    m_scriptEditor->setMarkerBackgroundColor(QColor(scriptConfig["markerBreakpointBackground"].toString()), MARKER_BREAKPOINT);
    m_scriptEditor->setMarkerForegroundColor(QColor(scriptConfig["markerBreakpointForeground"].toString()), MARKER_BREAKPOINT);
    m_scriptEditor->markerDefine(static_cast<QsciScintilla::MarkerSymbol>(scriptConfig["markerDebugStyle"].toInt()), MARKER_DEBUG);
    m_scriptEditor->setMarkerBackgroundColor(QColor(scriptConfig["markerDebugBackground"].toString()), MARKER_DEBUG);
    m_scriptEditor->setMarkerForegroundColor(QColor(scriptConfig["markerDebugForeground"].toString()), MARKER_DEBUG);

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
    m_scriptEditor->setText(content);
    m_scriptHash = stringHashCalc(m_scriptEditor->text());
    // connect signals
    connect(m_scriptEditor, SIGNAL(SCN_CHARADDED(int)), this, SLOT(charAdded(int)));
    connect(m_scriptEditor, SIGNAL(marginClicked(int,int,Qt::KeyboardModifiers)), this, SLOT(marginClick(int,int,Qt::KeyboardModifiers)));
    connect(m_scriptEditor, &ScriptEditor::dockRight, this, [this] {
        const auto controller = dockWidget();
        if (const auto tabGroup = group(); tabGroup->dockWidgetCount() > 1) {
            for (const auto &dock: tabGroup->dockWidgets()) {
                if (dock != controller) {
                    controller->addDockWidgetToContainingWindow(controller, KDDockWidgets::Location_OnRight, dock);
                }
            }
        }
    });
    connect(m_scriptEditor, &ScriptEditor::dockLeft, this, [this] {
        const auto controller = dockWidget();
        if (const auto tabGroup = group(); tabGroup->dockWidgetCount() > 1) {
            for (const auto &dock: tabGroup->dockWidgets()) {
                if (dock != controller) {
                    controller->addDockWidgetToContainingWindow(controller, KDDockWidgets::Location_OnLeft, dock);
                }
            }
        }
    });
    connect(m_scriptEditor, &ScriptEditor::dockTop, this, [this] {
        const auto controller = dockWidget();
        if (const auto tabGroup = group(); tabGroup->dockWidgetCount() > 1) {
            for (const auto &dock: tabGroup->dockWidgets()) {
                if (dock != controller) {
                    controller->addDockWidgetToContainingWindow(controller, KDDockWidgets::Location_OnTop, dock);
                }
            }
        }
    });
    connect(m_scriptEditor, &ScriptEditor::dockBottom, this, [this] {
        const auto controller = dockWidget();
        if (const auto tabGroup = group(); tabGroup->dockWidgetCount() > 1) {
            for (const auto &dock: tabGroup->dockWidgets()) {
                if (dock != controller) {
                    controller->addDockWidgetToContainingWindow(controller, KDDockWidgets::Location_OnBottom, dock);
                }
            }
        }
    });
    connect(m_scriptEditor, &ScriptEditor::openInExplorer, this, [this] {
        const QString folderPath = QFileInfo(m_scriptUrl.toLocalFile()).absolutePath();
#ifdef Q_OS_WIN
        const QString command = "explorer.exe";
        QStringList args;
        args << QDir::toNativeSeparators(folderPath);
        QProcess::startDetached(command, args);
#endif
    });
    connect(m_scriptEditor, &ScriptEditor::openInApplication, this, [this] {
#ifdef Q_OS_WIN
        const QString command = "explorer.exe";
        QStringList args;
        args << QDir::toNativeSeparators(m_scriptUrl.toLocalFile());
        QProcess::startDetached(command, args);
#endif
    });
    connect(m_scriptEditor, &ScriptEditor::requestPermission, this, &ScriptPage::permissionRequest);
    connect(m_scriptEditor, &ScriptEditor::requestIdle, this, &ScriptPage::idleRequest);
    connect(m_scriptEditor, &ScriptEditor::hideHoverTooltip, this, &ScriptPage::hideHoverTooltip);
    connect(m_scriptEditor, &ScriptEditor::leaveHoverTooltip, this, &ScriptPage::leaveHoverTooltip);
    connect(m_scriptEditor, &ScriptEditor::requestDefinition, this, &ScriptPage::definitionRequest);
    connect(m_scriptEditor, &ScriptEditor::requestDocumentHighlight, this, &ScriptPage::documentHighlightRequest);
    connect(m_scriptEditor, &ScriptEditor::requestFormatting, this, &ScriptPage::formattingRequest);
    connect(m_scriptEditor, &ScriptEditor::requestHover, this, &ScriptPage::hoverRequest);
    connect(m_scriptEditor, &ScriptEditor::requestImplementation, this, &ScriptPage::implementationRequest);
    connect(m_scriptEditor, &ScriptEditor::requestOnTypeFormatting, this, &ScriptPage::onTypeFormattingRequest);
    connect(m_scriptEditor, &ScriptEditor::requestReferences, this, &ScriptPage::referencesRequest);
    connect(m_scriptEditor, &ScriptEditor::requestTypeDefinition, this, &ScriptPage::typeDefinitionRequest);
    connect(m_scriptEditor, &ScriptEditor::setStat, m_searchWidget, &SearchWidget::statSet);
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &ScriptPage::scriptReload);
    connect(m_searchWidget, &SearchWidget::searchText, m_scriptEditor, &ScriptEditor::textSearch);
    connect(m_searchWidget, &SearchWidget::searchPrev, m_scriptEditor, &ScriptEditor::prevSearch);
    connect(m_searchWidget, &SearchWidget::searchNext, m_scriptEditor, &ScriptEditor::nextSearch);
    connect(m_searchWidget, &SearchWidget::replaceText, m_scriptEditor, qOverload<const QString &>(&ScriptEditor::textReplace));
    connect(m_searchWidget, &SearchWidget::replaceAllText, m_scriptEditor, &ScriptEditor::textReplaceAll);
    QTimer::singleShot(0, this, [this] {
        // lsp
        didOpenNotification();
        // logging
        emit appendLog(QString("<a href='%1'>%2</a> opened").arg(m_scriptUrl.toString(), m_scriptUrl.toString()), "info");
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 opened").arg(timestamp, m_scriptUrl.toString());
    });
}

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
    m_scriptEditor->setText(content);
    // logging
    emit appendLog(QString("<a href='%1'>%2</a> reloaded").arg(m_scriptUrl.toString(), m_scriptUrl.fileName()), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 reloaded").arg(timestamp, m_scriptUrl.fileName());
}

void ScriptPage::scriptSave() {
    if (!m_modified) return;
    // update status
    scriptModify(false);
    m_scriptHash = fileHashCalc(m_scriptEditor->text());
    didSaveNotification();
    // block file watcher signals
    m_fileWatcher->blockSignals(true);
    // save file
    const QString scriptPath = m_scriptUrl.toLocalFile();
    QFile file(scriptPath);
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);
    out << m_scriptEditor->text();
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
    if (m_modified) {
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

void ScriptPage::diagnosticsResponse(const QJsonArray &diagnostics) {
    m_scriptDiagnostic = diagnostics;
    // clear previous diagnostics
    m_scriptEditor->indicatorRemove(INDICATOR_ERROR);
    m_scriptEditor->indicatorRemove(INDICATOR_WARNING);
    m_scriptEditor->indicatorRemove(INDICATOR_INFO);
    m_scriptEditor->indicatorRemove(INDICATOR_HINT);
    // publish diagnostics
    for (const auto &value: diagnostics) {
        const QJsonObject diagnostic = value.toObject();
        const int severity = diagnostic["severity"].toInt();
        const QJsonObject range = diagnostic["range"].toObject();
        const QJsonObject startPos = range["start"].toObject();
        const QJsonObject endPos = range["end"].toObject();
        const int startLine = startPos["line"].toInt();
        const int startCharacter = startPos["character"].toInt();
        const int endLine = endPos["line"].toInt();
        const int endCharacter = endPos["character"].toInt();
        m_scriptEditor->indicatorInsert(severity, startLine, startCharacter, endLine, endCharacter);
    }
}

void ScriptPage::documentHighlightResponse(const QJsonArray &result) const {
    // clear previous highlight
    m_scriptEditor->indicatorRemove(INDICATOR_HIGHLIGHT);
    m_scriptEditor->indicatorRemove(INDICATOR_READ);
    m_scriptEditor->indicatorRemove(INDICATOR_WRITE);
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
        m_scriptEditor->indicatorInsert(INDICATOR_HIGHLIGHT, startLine, startCharacter, endLine, endCharacter);
        if (kind == 2) m_scriptEditor->indicatorInsert(INDICATOR_READ, startLine, startCharacter, endLine, endCharacter);
        else if (kind == 3) m_scriptEditor->indicatorInsert(INDICATOR_WRITE, startLine, startCharacter, endLine, endCharacter);
    }
}

void ScriptPage::foldingRangeResponse(const QJsonArray &result) const {
    QMap<int, int> deltaDepthMap;
    for (const auto &value: result) {
        const QJsonObject valueObject = value.toObject();
        const int startLine = valueObject["startLine"].toInt();
        const int endLine = valueObject["endLine"].toInt();
        deltaDepthMap.insert(startLine + 1, deltaDepthMap.value(startLine + 1, 0) + 1);
        deltaDepthMap.insert(endLine + 1, deltaDepthMap.value(endLine + 1, 0) - 1);
    }
    int currentDepth = 0;
    for (int line = 0; line < m_scriptEditor->lines(); line++) {
        const int deltaDepth = deltaDepthMap.value(line, 0);
        currentDepth += deltaDepth;
        int level = QsciScintilla::SC_FOLDLEVELBASE + currentDepth;
        if (deltaDepthMap.value(line + 1, 0) > 0) level |= QsciScintilla::SC_FOLDLEVELHEADERFLAG;
        m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETFOLDLEVEL, line, level); // NOLINT
    }
}

void ScriptPage::formattingResponse(const QString &newText) const {
    m_scriptEditor->setText(newText);
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
    m_scriptEditor->textReplace(text, startLine, startCharacter, endLine, endCharacter);
}

void ScriptPage::semanticTokensResponse(const QJsonArray &data) const {
    // clear
    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STARTSTYLING, 0); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, m_scriptEditor->length(), static_cast<long>(0));
    int currentLine = 0;
    int currentChar = 0;
    for (int i = 0; i < data.size(); i += 5) {
        // semantic tokens response extract
        const int deltaLine = data[i].toInt();
        const int deltaStartChar = data[i + 1].toInt();
        const int length = data[i + 2].toInt();
        const int tokenType = data[i + 3].toInt();
        const int tokenModifiers = data[i + 4].toInt();
        // calculate start position
        currentLine += deltaLine;
        currentChar = deltaLine > 0 ? deltaStartChar : currentChar + deltaStartChar;
        const int startPos = m_scriptEditor->positionFromLineIndex(currentLine, currentChar);
        const int endPos = startPos + length;
        if (startPos < 0 || endPos > m_scriptEditor->length() || length <= 0) {
            qDebug() << "long string skipped" << currentLine << currentChar;
            continue;
        }
        // start styling
        m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STARTSTYLING, startPos); // NOLINT
        switch (tokenType) {
            case TOKENTYPE_NAMESPACE:
                m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_NAMESPACE); // NOLINT
                break;
            case TOKENTYPE_CLASS:
                m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_CLASS); // NOLINT
                break;
            case TOKENTYPE_TYPE:
                m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_TYPE); // NOLINT
                break;
            case TOKENTYPE_PARAMETER:
                m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_PARAMETER); // NOLINT
                break;
            case TOKENTYPE_VARIABLE:
                m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_VARIABLE); // NOLINT
                break;
            case TOKENTYPE_PROPERTY:
                m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_PROPERTY); // NOLINT
                break;
            case TOKENTYPE_ENUMMEMBAER:
                m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_ENUMMEMBAER); // NOLINT
                break;
            case TOKENTYPE_FUNCTION:
                if (tokenModifiers == TOKENMODIFIERS_DECLARATION || tokenModifiers == TOKENMODIFIERS_GLOBAL) {
                    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_FUNCTION_DECLARATION); // NOLINT
                } else {
                    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_FUNCTION_CALL); // NOLINT
                }
                break;
            case TOKENTYPE_METHOD:
                m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_METHOD); // NOLINT
                break;
            case TOKENTYPE_MACRO:
                m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_MACRO); // NOLINT
                break;
            case TOKENTYPE_KEYWORD:
                m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_KEYWORD); // NOLINT
                break;
            case TOKENTYPE_COMMENT:
                m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_COMMENT); // NOLINT
                break;
            case TOKENTYPE_STRING:
                m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_STRING); // NOLINT
                break;
            case TOKENTYPE_NUMBER:
                m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_NUMBER); // NOLINT
                break;
            case TOKENTYPE_OPERATOR:
                m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_OPERATOR); // NOLINT
                break;
            default:
                qDebug() << "skip token" << currentLine << currentChar << length << tokenType;
                break;
        }
    }
}

void ScriptPage::spellCheckResponse(const QVariantList &typos) {
    m_scriptTypo = typos;
    // clear previous typo
    m_scriptEditor->indicatorRemove(INDICATOR_TYPO);
    // publish typo
    for (const auto &value: typos) {
        auto typo = value.toMap();
        const int lineFrom = typo["line"].toInt();
        const int lineTo = typo["line"].toInt();
        const int indexFrom = typo["indexFrom"].toInt();
        const int indexTo = typo["indexTo"].toInt();
        m_scriptEditor->indicatorInsert(INDICATOR_TYPO, lineFrom, indexFrom, lineTo, indexTo);
    }
}

void ScriptPage::textReplace(QString &text, const QString &kind) {
    if (kind == "Function") {
        text += "()";
    } else if (kind == "Field") {
        text += ".";
    } else if (kind == "EnumMember") {
        if (text == "\"Add New Port\"") {
            emit insertPort();
            return;
        }
        if (text == "\"Add New Database Key\"") {
            emit insertDatabase();
            return;
        }
        if (text == "\"Add New Datatable Key\"") {
            emit insertDatatable();
            return;
        }
        if (text == "\"Position Hint\"") {
            emit showPositionTooltip();
            return;
        }
        text.replace("\\", "\\\\");
    }
    const long currentPos = m_scriptEditor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    const long startPos = m_scriptEditor->SendScintilla(QsciScintilla::SCI_WORDSTARTPOSITION, currentPos, true); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETTARGETRANGE, startPos, currentPos); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintilla::SCI_REPLACETARGET, text.length(), text.toUtf8().constData()); // NOLINT
    long cursorPos;
    if (kind == "Function") {
        cursorPos = startPos + text.length() - 1;
    } else {
        cursorPos = startPos + text.length();
    }
    m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETCURRENTPOS, cursorPos); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETSELECTIONSTART, cursorPos); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETSELECTIONEND, cursorPos); // NOLINT
    if (kind == "Function") {
        didChangeNotification();
        emit fullCompletionTooltip(false);
        completionRequest();
        signatureHelpRequest();
    } else if (kind == "Field") {
        didChangeNotification();
        emit fullCompletionTooltip(true);
        completionRequest();
    }
}

// ScriptPage protected
void ScriptPage::closeEvent(QCloseEvent *event) {
    scriptClose();
    event->accept();
}

// ScriptPage private slots
void ScriptPage::charAdded(const int ch) {
    const QChar character(ch);
    if (character.isLetter() || m_completionTrigger.contains(character)) {
        didChangeNotification();
        emit fullCompletionTooltip(true);
        completionRequest();
    } else if (m_signatureHelpTrigger.contains(character)) {
        didChangeNotification();
        emit fullCompletionTooltip(false);
        completionRequest();
        signatureHelpRequest();
    } else if (m_onTypeFormattingTrigger.contains(character)) {
        didChangeNotification();
        onTypeFormattingRequest();
    }
}

void ScriptPage::marginClick(const int margin, const int line, Qt::KeyboardModifiers state) {
    if (margin == 1 && line >= 0) {
        if (m_scriptEditor->markersAtLine(line) & 1 << MARKER_BREAKPOINT) {
            g_breakpoints[m_scriptUrl].remove(line + 1);
            if (g_breakpoints[m_scriptUrl].isEmpty()) g_breakpoints.remove(m_scriptUrl);
            emit removeBreakpoint(m_scriptUrl, line + 1);
            emit removeMarker(m_scriptUrl, MARKER_BREAKPOINT, line + 1);
        } else {
            g_breakpoints[m_scriptUrl][line + 1]["expr"] = "";
            emit insertBreakpoint(m_scriptUrl, line + 1);
            emit insertMarker(m_scriptUrl, MARKER_BREAKPOINT, line + 1, -1);
        }
    }
}

// ScriptPage private
void ScriptPage::scriptReadonly(const bool status) {
    m_readonly = status;
    m_scriptEditor->setReadOnly(status);
    if (status) {
        setIcon(QIcon(":/icon/lockClosed.svg"));
    } else {
        setIcon(QIcon());
    }
}

void ScriptPage::scriptModify(const bool status) {
    m_modified = status;
    const QString pageName = title();
    if (status) {
        setTitle(pageName + "*");
    } else {
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

void ScriptPage::idleRequest() {
    int line, character;
    m_scriptEditor->getCursorPosition(&line, &character);
    // lsp request
    didChangeNotification();
    documentHighlightRequest();
    documentSymbolRequest();
    foldingRangeRequest();
    semanticTokensRequest();
    // nuspell request
    spellCheckRequest();
    // modification check
    bool modified{};
    if (const QString script = m_scriptEditor->text(); stringHashCalc(script) != m_scriptHash) {
        modified = true;
    } else {
        modified = false;
    }
    if (modified != m_modified) {
        scriptModify(modified);
    }
}

void ScriptPage::didOpenNotification() {
    // did open notification to lua language server
    const QJsonObject didOpenParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()},
                {"languageId", "lua"},
                {"version", m_version++},
                {"text", m_scriptEditor->text()}
            }
        }
    };
    emit notificationJson("textDocument/didOpen", didOpenParams);
}

void ScriptPage::didChangeNotification() {
    // did change notification to lua language server
    const QString content = m_scriptEditor->text();
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
    // get cursor position
    int line, character;
    m_scriptEditor->getCursorPosition(&line, &character);
    // completion request to script module
    emit requestCompletion(m_scriptUrl, line, character);
}

void ScriptPage::definitionRequest() {
    // get cursor position
    int line, character;
    m_scriptEditor->getCursorPosition(&line, &character);
    // definition request to script module
    emit requestDefinition(m_scriptUrl, line, character);
}

void ScriptPage::documentHighlightRequest() {
    // get cursor position
    int line, character;
    m_scriptEditor->getCursorPosition(&line, &character);
    // document highlight request to script module
    emit requestDocumentHighlight(m_scriptUrl, line, character);
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
    const long charPos = m_scriptEditor->SendScintilla(QsciScintilla::SCI_POSITIONFROMPOINTCLOSE, localPos.x(), localPos.y());
    if (charPos == -1) return;
    int line, character;
    m_scriptEditor->lineIndexFromPosition(charPos, &line, &character);
    if (line == 0 && character == 0) return;
    // show diagnostic if exists
    QString diagnosticText = "<table width='100%'>";
    for (const auto &value: m_scriptDiagnostic) {
        const QJsonObject diagnostic = value.toObject();
        const QJsonObject range = diagnostic["range"].toObject();
        const QJsonObject startPos = range["start"].toObject();
        const QJsonObject endPos = range["end"].toObject();
        const int startLine = startPos["line"].toInt();
        const int startCharacter = startPos["character"].toInt();
        const int endLine = endPos["line"].toInt();
        const int endCharacter = endPos["character"].toInt();
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
            diagnosticText += QString("<tr><td><b>%1</b>: %2</td><td></td></tr>").arg(severityString, message.toHtmlEscaped());
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
            const int startPos = m_scriptEditor->positionFromLineIndex(lineFrom, indexFrom);
            const int endPos = m_scriptEditor->positionFromLineIndex(lineTo, indexTo);
            const QString word = m_scriptEditor->text(startPos, endPos);
            const QString commandLine = QString("requestspellsuggest://%1/%2/%3/%4/%5").arg(
                word, QString::number(lineFrom), QString::number(indexFrom), QString::number(lineTo), QString::number(indexTo));
            diagnosticText += QString("<tr><td><b>Typo</b>: In word '%1'</td><td align='right'><a href='%2'>Show Suggestions</a></td></tr>").arg(word, commandLine);
        }
    }
    if (diagnosticText != "<table width='100%'>") {
        diagnosticText += "</table>";
        emit showDiagnosticTooltip(m_scriptUrl, diagnosticText);
    }
    // hover request to script module
    emit requestHover(m_scriptUrl, line, character);
}

void ScriptPage::implementationRequest() {
    // get cursor position
    int line, character;
    m_scriptEditor->getCursorPosition(&line, &character);
    // implementation request to script module
    emit requestImplementation(m_scriptUrl, line, character);
}

void ScriptPage::referencesRequest() {
    // get cursor position
    int line, character;
    m_scriptEditor->getCursorPosition(&line, &character);
    // references request to script module
    emit requestReferences(m_scriptUrl, line, character);
}

void ScriptPage::onTypeFormattingRequest() {
    // get cursor position
    int line, character;
    m_scriptEditor->getCursorPosition(&line, &character);
    // on type formatting request to script module
    emit requestOnTypeFormatting(m_scriptUrl, line, character);
}

void ScriptPage::semanticTokensRequest() {
    // semantic tokens request to script module
    emit requestSemanticTokens(m_scriptUrl);
}

void ScriptPage::signatureHelpRequest() {
    // get cursor position
    int line, character;
    m_scriptEditor->getCursorPosition(&line, &character);
    // signature help request to script module
    emit requestSignatureHelp(m_scriptUrl, line, character);
}

void ScriptPage::spellCheckRequest() {
    // spell check request to script module
    emit requestSpellCheck(m_scriptUrl, m_scriptEditor->text());
}

void ScriptPage::typeDefinitionRequest() {
    // get cursor position
    int line, character;
    m_scriptEditor->getCursorPosition(&line, &character);
    // type definition request to script module
    emit requestTypeDefinition(m_scriptUrl, line, character);
}

void ScriptPage::positionFill(const int x, const int y) const {
    const QString text = QString("%1, %2").arg(QString::number(x), QString::number(y));
    m_scriptEditor->insert(text);
    const long currentPos = m_scriptEditor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    const long cursorPos = currentPos + text.length();
    m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETCURRENTPOS, cursorPos); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETSELECTIONSTART, cursorPos); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETSELECTIONEND, cursorPos); // NOLINT
}

// SearchWidget public
SearchWidget::SearchWidget(QWidget *parent)
    : QWidget(parent),
      m_searchLineEdit(new QLineEdit()),
      m_wholeWordButton(new QPushButton()),
      m_matchCaseButton(new QPushButton()),
      m_wordStartButton(new QPushButton()),
      m_regExpButton(new QPushButton()),
      m_statLabel(new QLabel("0/0")),
      m_prevButton(new QPushButton()),
      m_nextButton(new QPushButton()),
      m_replaceLineEdit(new QLineEdit()),
      m_replaceButton(new QPushButton(tr("Replace"))),
      m_replaceAllButton(new QPushButton(tr("Replace All"))) {
    auto *layout = new QVBoxLayout(this); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);

    auto *searchBar = new QWidget(); // NOLINT
    layout->addWidget(searchBar);
    auto *searchLayout = new QHBoxLayout(searchBar); // NOLINT
    searchLayout->setContentsMargins(0, 0, 0, 0);
    searchLayout->addWidget(m_searchLineEdit);
    m_searchLineEdit->setClearButtonEnabled(true);
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, [this] { emit searchText(m_searchLineEdit->text(), m_searchFlag); });
    searchLayout->addWidget(m_wholeWordButton);
    m_wholeWordButton->setCheckable(true);
    m_wholeWordButton->setFixedSize(24, 24);
    m_wholeWordButton->setIcon(QIcon(":/icon/wholeWord.svg"));
    m_wholeWordButton->setToolTip(tr("Whole Word"));
    connect(m_wholeWordButton, &QPushButton::clicked, this, [this](const bool status) {
        if (status) {
            m_searchFlag |= QsciScintilla::SCFIND_WHOLEWORD;
        } else {
            m_searchFlag &= ~QsciScintilla::SCFIND_WHOLEWORD;
        }
        emit searchText(m_searchLineEdit->text(), m_searchFlag);
    });
    searchLayout->addWidget(m_matchCaseButton);
    m_matchCaseButton->setCheckable(true);
    m_matchCaseButton->setFixedSize(24, 24);
    m_matchCaseButton->setIcon(QIcon(":/icon/matchCase.svg"));
    m_matchCaseButton->setToolTip(tr("Match Case"));
    connect(m_matchCaseButton, &QPushButton::clicked, this, [this](const bool status) {
        if (status) {
            m_searchFlag |= QsciScintilla::SCFIND_MATCHCASE;
        } else {
            m_searchFlag &= ~QsciScintilla::SCFIND_MATCHCASE;
        }
        emit searchText(m_searchLineEdit->text(), m_searchFlag);
    });
    searchLayout->addWidget(m_wordStartButton);
    m_wordStartButton->setCheckable(true);
    m_wordStartButton->setFixedSize(24, 24);
    m_wordStartButton->setIcon(QIcon(":/icon/wordStart.svg"));
    m_wordStartButton->setToolTip(tr("Word Start"));
    connect(m_wordStartButton, &QPushButton::clicked, this, [this](const bool status) {
        if (status) {
            m_searchFlag |= QsciScintilla::SCFIND_WORDSTART;
        } else {
            m_searchFlag &= ~QsciScintilla::SCFIND_WORDSTART;
        }
        emit searchText(m_searchLineEdit->text(), m_searchFlag);
    });
    searchLayout->addWidget(m_regExpButton);
    m_regExpButton->setCheckable(true);
    m_regExpButton->setFixedSize(24, 24);
    m_regExpButton->setIcon(QIcon(":/icon/regExp.svg"));
    m_regExpButton->setToolTip(tr("Regular Expression"));
    connect(m_regExpButton, &QPushButton::clicked, this, [this](const bool status) {
        if (status) {
            m_searchFlag |= QsciScintilla::SCFIND_REGEXP;
        } else {
            m_searchFlag &= ~QsciScintilla::SCFIND_REGEXP;
        }
        emit searchText(m_searchLineEdit->text(), m_searchFlag);
    });
    searchLayout->addStretch();
    searchLayout->addWidget(m_statLabel);
    searchLayout->addWidget(m_prevButton);
    m_prevButton->setEnabled(false);
    m_prevButton->setFixedSize(24, 24);
    m_prevButton->setIcon(QIcon(":/icon/arrowUp.svg"));
    m_prevButton->setToolTip(tr("Search Previous"));
    connect(m_prevButton, &QPushButton::clicked, this, &SearchWidget::searchPrev);
    searchLayout->addWidget(m_nextButton);
    m_nextButton->setEnabled(false);
    m_nextButton->setFixedSize(24, 24);
    m_nextButton->setIcon(QIcon(":/icon/arrowDown.svg"));
    m_nextButton->setToolTip(tr("Search Next"));
    connect(m_nextButton, &QPushButton::clicked, this, &SearchWidget::searchNext);

    auto *replaceBar = new QWidget(); // NOLINT
    layout->addWidget(replaceBar);
    auto *replaceLayout = new QHBoxLayout(replaceBar); // NOLINT
    replaceLayout->setContentsMargins(0, 0, 0, 0);
    replaceLayout->addWidget(m_replaceLineEdit);
    m_replaceLineEdit->setClearButtonEnabled(true);
    replaceLayout->addStretch();
    replaceLayout->addWidget(m_replaceButton);
    m_replaceButton->setEnabled(false);
    connect(m_replaceButton, &QPushButton::clicked, this, [this] { emit replaceText(m_replaceLineEdit->text()); });
    replaceLayout->addWidget(m_replaceAllButton);
    m_replaceAllButton->setEnabled(false);
    connect(m_replaceAllButton, &QPushButton::clicked, this, [this] { emit replaceAllText(m_replaceLineEdit->text()); });

    setTabOrder(m_searchLineEdit, m_replaceLineEdit);

    hide();
}

void SearchWidget::toggle() {
    if (isVisible()) hide();
    else {
        m_searchLineEdit->setFocus();
        show();
    }
}

void SearchWidget::statSet(int current, const int total) const {
    if (current == 0 && total == 0) {
        m_prevButton->setEnabled(false);
        m_nextButton->setEnabled(false);
        m_replaceButton->setEnabled(false);
        m_replaceAllButton->setEnabled(false);
    } else {
        m_prevButton->setEnabled(true);
        m_nextButton->setEnabled(true);
        m_replaceButton->setEnabled(true);
        m_replaceAllButton->setEnabled(true);
        current++;
    }
    m_statLabel->setText(QString("%1/%2").arg(QString::number(current), QString::number(total)));
}
