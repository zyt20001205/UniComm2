#include "scriptModule/scriptPage.h"

#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QJsonArray>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>
#include <kddockwidgets/core/DockWidget.h>
#include <kddockwidgets/core/Group.h>

#include "globals.h"
#include "utils/qtUtils.h"

// ScriptPage public
ScriptPage::ScriptPage(const QJsonObject &scriptConfig, const QUrl &scriptUrl)
    : DockWidget(scriptUrl.fileName()),
      m_scriptEditor(new ScriptEditor()),
      m_scriptUrl(scriptUrl),
      m_fileWatcher(new QFileSystemWatcher()),
      m_searchWidget(new SearchWidget()),
      m_editTimer(new QTimer(this)) {
    auto shortcutSearch = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this); // NOLINT
    connect(shortcutSearch, &QShortcut::activated, m_searchWidget, &SearchWidget::toggle);
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
    m_scriptEditor->setFont(QFont(scriptConfig["fontFamily"].toString(), scriptConfig["fontSize"].toInt()));
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
    m_editTimer->setInterval(300);
    m_editTimer->setSingleShot(true);
    connect(m_editTimer, &QTimer::timeout, [this] {
        scriptEditFinish();
    });
    // connect signals
    connect(m_scriptEditor, SIGNAL(textChanged()), this, SLOT(scriptEdit()));
    connect(m_scriptEditor, SIGNAL(SCN_CHARADDED(int)), this, SLOT(charAdded(int)));
    connect(m_scriptEditor, SIGNAL(SCN_DWELLSTART(int,int,int)), this, SLOT(dwellStart(int,int,int)));
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
    connect(m_scriptEditor, &ScriptEditor::requestPermission, this, &ScriptPage::permissionRequest);
    connect(m_scriptEditor, &ScriptEditor::requestDefinition, this, &ScriptPage::definitionRequest);
    connect(m_scriptEditor, &ScriptEditor::requestFormatting, this, &ScriptPage::formattingRequest);
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &ScriptPage::scriptReload);
    connect(m_searchWidget, &SearchWidget::searchText, m_scriptEditor, &ScriptEditor::textSearch);
    QTimer::singleShot(0, this, [this] {
        // lsp
        didOpenNotification();
        documentSymbolRequest();
        foldingRangeRequest();
        semanticTokensRequest();
        // logging
        emit appendLog(QString("<a href='%1'>%2</a> opened").arg(m_scriptUrl.toString(), m_scriptUrl.fileName()), "info");
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 opened").arg(timestamp, m_scriptUrl.fileName());
    });
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
    emit appendLog(QString("<a href='%1'>%2</a> saved").arg(m_scriptUrl.toString(), m_scriptUrl.fileName()), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 saved").arg(timestamp, m_scriptUrl.fileName());
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
    emit appendLog(QString("<a href='%1'>%2</a> closed").arg(m_scriptUrl.toString(), m_scriptUrl.fileName()), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 closed").arg(timestamp, m_scriptUrl.fileName());
}

void ScriptPage::diagnosticsResponse(const QJsonArray &diagnosticsArray) const {
    // clear previous diagnostics
    const int lastLine = m_scriptEditor->lines() - 1;
    const int lastIndex = m_scriptEditor->lineLength(lastLine);
    m_scriptEditor->clearIndicatorRange(0, 0, lastLine, lastIndex, INDICATOR_ERROR);
    m_scriptEditor->clearIndicatorRange(0, 0, lastLine, lastIndex, INDICATOR_WARNING);
    m_scriptEditor->clearIndicatorRange(0, 0, lastLine, lastIndex, INDICATOR_INFO);
    m_scriptEditor->clearIndicatorRange(0, 0, lastLine, lastIndex, INDICATOR_HINT);
    // publish diagnostics
    int row = 0;
    for (const auto &diagnostic: diagnosticsArray) {
        const QJsonObject diagnosticObject = diagnostic.toObject();
        const int severity = diagnosticObject["severity"].toInt();
        const QJsonObject diagnosticRange = diagnosticObject["range"].toObject();
        const QJsonObject diagnosticStartPos = diagnosticRange["start"].toObject();
        const QJsonObject diagnosticEndPos = diagnosticRange["end"].toObject();
        const int startLine = diagnosticStartPos["line"].toInt();
        const int startCharacter = diagnosticStartPos["character"].toInt();
        const int endLine = diagnosticEndPos["line"].toInt();
        const int endCharacter = diagnosticEndPos["character"].toInt();
        m_scriptEditor->fillIndicatorRange(startLine, startCharacter, endLine, endCharacter, severity);
        row++;
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
            qDebug() << "skip token" << currentLine << currentChar << length << tokenType;
            continue;
        }
        // start styling
        m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STARTSTYLING, startPos); // NOLINT
        switch (tokenType) {
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

void ScriptPage::textReplace(QString &text, const QString &kind) {
    if (kind == "Function") {
        text += "()";
    } else if (kind == "Field") {
        text += ".";
    } else if (kind == "EnumMember") {
        if (text == "\"Add New Port\"") {
            emit insertPort(-1, QJsonObject());
            return;
        }
        if (text == "\"Add New Database Key\"") {
            emit insertDatabase(-1, QString());
            return;
        }
        if (text == "\"Add New Datatable Key\"") {
            emit insertDatatable(-1, QString());
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
        emit setFullCompletion(false);
        completionRequest();
        signatureHelpRequest();
    } else if (kind == "Field") {
        didChangeNotification();
        emit setFullCompletion(true);
        completionRequest();
    }
}

// ScriptPage protected
void ScriptPage::closeEvent(QCloseEvent *event) {
    scriptClose();
    event->accept();
}

// ScriptPage private slots
void ScriptPage::scriptEdit() const {
    m_editTimer->stop();
    m_editTimer->start();
}

void ScriptPage::charAdded(const int ch) {
    const QChar character(ch);
    if (character.isLetter() || character == '.' || character == ':') {
        didChangeNotification();
        emit setFullCompletion(true);
        completionRequest();
    } else if (character == "(" || character == ",") {
        didChangeNotification();
        emit setFullCompletion(false);
        completionRequest();
        signatureHelpRequest();
    }
}

void ScriptPage::dwellStart(const int pos, const int x, const int y) {
    const QPoint globalPos = QCursor::pos();
    QPoint localPos = m_scriptEditor->mapFromGlobal(globalPos);
    if (!m_scriptEditor->rect().contains(localPos)) return;
    int line, character;
    m_scriptEditor->lineIndexFromPosition(pos, &line, &character);
    if (line == 0 && character == 0) return;
    hoverRequest(line, character);
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
void ScriptPage::scriptEditFinish() {
    // lsp request
    didChangeNotification();
    documentSymbolRequest();
    foldingRangeRequest();
    semanticTokensRequest();
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
    // completion request to lua language server
    int line, character;
    m_scriptEditor->getCursorPosition(&line, &character);
    const QJsonObject completionParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()}
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

void ScriptPage::definitionRequest(const int line, const int character) {
    // definition request to lua language server
    const QJsonObject definitionParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()}
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

void ScriptPage::documentSymbolRequest() {
    // document symbol request to lua language server
    const QJsonObject documentSymbolParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()}
            }
        }
    };
    emit requestJson("textDocument/documentSymbol", documentSymbolParams);
}

void ScriptPage::foldingRangeRequest() {
    // folding range request to lua language server
    const QJsonObject foldingRangeParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()}
            }
        }
    };
    emit requestJson("textDocument/foldingRange", foldingRangeParams);
}

void ScriptPage::formattingRequest() {
    // formatting request to lua language server
    const QJsonObject formattingParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()}
            }
        },
        {
            "options", QJsonObject{
                {"tabSize", m_scriptEditor->tabWidth()},
                {"insertSpaces", true},
                {"trimTrailingWhitespace", true},
                {"insertFinalNewline", true}
            }
        }
    };
    emit requestJson("textDocument/formatting", formattingParams);
}

void ScriptPage::semanticTokensRequest() {
    // semantic tokens request to lua language server
    const QJsonObject semanticTokensParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()}
            }
        }
    };
    emit requestJson("textDocument/semanticTokens/full", semanticTokensParams);
}

void ScriptPage::signatureHelpRequest() {
    // signature help request to lua language server
    int line, character;
    m_scriptEditor->getCursorPosition(&line, &character);
    const QJsonObject signatureHelpParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()}
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

void ScriptPage::hoverRequest(const int line, const int character) {
    // hover request to lua language server
    const QJsonObject hoverParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()}
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
      m_regExpButton(new QPushButton()) {
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

    hide();
}

void SearchWidget::toggle() {
    if (isVisible()) hide();
    else {
        m_searchLineEdit->setFocus();
        show();
    }
}

// ScriptEditor public
ScriptEditor::ScriptEditor(QWidget *parent)
    : QsciScintilla(parent) {
    // set markers
    markerDefine(Circle, MARKER_BREAKPOINT);
    setMarkerBackgroundColor(Qt::red, MARKER_BREAKPOINT);
    setMarkerForegroundColor(Qt::red, MARKER_BREAKPOINT);

    markerDefine(RightTriangle, MARKER_ARROW);
    setMarkerBackgroundColor(QColor(255, 165, 0), MARKER_ARROW);
    setMarkerForegroundColor(QColor(255, 165, 0), MARKER_ARROW);

    markerDefine(Background, MARKER_ERROR);
    setMarkerBackgroundColor(QColor(255, 230, 230), MARKER_ERROR);

    markerDefine(Background, MARKER_HINT);
    setMarkerBackgroundColor(Qt::cyan, MARKER_HINT);

    markerDefine(Background, MARKER_HEATMAP0);
    setMarkerBackgroundColor(QColor(235, 245, 235), MARKER_HEATMAP0);

    markerDefine(Background, MARKER_HEATMAP25);
    setMarkerBackgroundColor(QColor(175, 225, 175), MARKER_HEATMAP25);

    markerDefine(Background, MARKER_HEATMAP50);
    setMarkerBackgroundColor(QColor(110, 200, 110), MARKER_HEATMAP50);

    markerDefine(Background, MARKER_HEATMAP75);
    setMarkerBackgroundColor(QColor(40, 160, 40), MARKER_HEATMAP75);

    markerDefine(Background, MARKER_HEATMAP100);
    setMarkerBackgroundColor(QColor(20, 100, 20), MARKER_HEATMAP100);

    // set indicators
    indicatorDefine(StraightBoxIndicator, INDICATOR_ERROR);
    setIndicatorForegroundColor(QColor(255, 230, 230), INDICATOR_ERROR);
    setIndicatorDrawUnder(true, INDICATOR_ERROR);

    indicatorDefine(StraightBoxIndicator, INDICATOR_WARNING);
    setIndicatorForegroundColor(QColor(255, 245, 230), INDICATOR_WARNING);
    setIndicatorDrawUnder(true, INDICATOR_WARNING);

    indicatorDefine(StraightBoxIndicator, INDICATOR_INFO);
    setIndicatorForegroundColor(QColor(230, 240, 250), INDICATOR_INFO);
    setIndicatorDrawUnder(true, INDICATOR_INFO);

    indicatorDefine(StraightBoxIndicator, INDICATOR_HINT);
    setIndicatorForegroundColor(QColor(245, 245, 245), INDICATOR_HINT);
    setIndicatorDrawUnder(true, INDICATOR_HINT);

    indicatorDefine(StraightBoxIndicator, INDICATOR_HIGHLIGHT);
    setIndicatorForegroundColor(QColor(252, 212, 126), INDICATOR_HIGHLIGHT);
    setIndicatorDrawUnder(true, INDICATOR_HIGHLIGHT);

    indicatorDefine(PlainIndicator, INDICATOR_HYPERLINK);
    setIndicatorForegroundColor(QColor(0, 0, 255), INDICATOR_HYPERLINK);
    setIndicatorDrawUnder(true, INDICATOR_HYPERLINK);

    // set margins
    setMarginType(0, NumberMargin);
    QsciScintilla::setMarginWidth(0, 32);

    setMarginType(1, SymbolMargin);
    QsciScintilla::setMarginSensitivity(1, true);
    QsciScintilla::setMarginWidth(1, 16);

    QsciScintilla::setFolding(CircledTreeFoldStyle);
    setMarginType(2, SymbolMargin);
    QsciScintilla::setMarginSensitivity(2, true);
    QsciScintilla::setMarginWidth(2, 16);

    // color format is BGR!!! DO NOT FORGET!!!
    SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_CLASS, static_cast<long>(0x808000)); // NOLINT
    SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_TYPE, static_cast<long>(0xB33300)); // NOLINT
    SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_PARAMETER, static_cast<long>(0x000000)); // NOLINT
    SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_VARIABLE, static_cast<long>(0x000000)); // NOLINT
    SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_PROPERTY, static_cast<long>(0x7A0E66)); // NOLINT
    SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_FUNCTION_DECLARATION, static_cast<long>(0x7A6200)); // NOLINT
    SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_FUNCTION_CALL, static_cast<long>(0x000000)); // NOLINT
    SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_METHOD, static_cast<long>(0x000000)); // NOLINT
    SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_MACRO, static_cast<long>(0x2E541F)); // NOLINT
    SendScintilla(QsciScintillaBase::SCI_STYLESETBOLD, LUATOKEN_MACRO, 1); // NOLINT
    SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_KEYWORD, static_cast<long>(0xB33300)); // NOLINT
    SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_COMMENT, static_cast<long>(0x8C8C8C)); // NOLINT
    SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_STRING, static_cast<long>(0x177D06)); // NOLINT
    SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_NUMBER, static_cast<long>(0xEB5017)); // NOLINT
    SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_OPERATOR, static_cast<long>(0x000000)); // NOLINT
    // script scintilla settings
    setScrollWidth(1);
    QsciScintilla::setBraceMatching(SloppyBraceMatch);
    QsciScintilla::setAutoIndent(true);
    QsciScintilla::setBackspaceUnindents(true);
    QsciScintilla::setIndentationGuides(true);
    QsciScintilla::setTabWidth(4);
    SendScintilla(QsciScintillaBase::SCI_SETMOUSEDWELLTIME, 1000); // NOLINT
    // connect auto pair
    connect(this, SIGNAL(SCN_CHARADDED(int)), this, SLOT(pairHandle(int)));
    m_autoPairHash['('] = ')';
    m_autoPairHash['['] = ']';
    m_autoPairHash['{'] = '}';
    m_autoPairHash['"'] = '"';
    m_autoPairHash['\''] = '\'';
}

void ScriptEditor::textSearch(const QString &text, const int flag) const {
    // clear previous search result
    const int docLength = SendScintilla(SCI_GETLENGTH);
    SendScintilla(SCI_SETINDICATORCURRENT, INDICATOR_HIGHLIGHT); // NOLINT
    SendScintilla(SCI_INDICATORCLEARRANGE, 0, docLength); // NOLINT
    // start searching
    if (text.isEmpty()) return;
    const int currentPos = SendScintilla(SCI_GETCURRENTPOS);
    const int anchorPos = SendScintilla(SCI_GETANCHOR);
    SendScintilla(SCI_SETCURRENTPOS, 0); // NOLINT
    SendScintilla(SCI_SEARCHANCHOR); // NOLINT
    while (SendScintilla(SCI_SEARCHNEXT, flag, text.toUtf8().constData()) != -1) {
        const int start = SendScintilla(SCI_GETSELECTIONSTART);
        const int end = SendScintilla(SCI_GETSELECTIONEND);
        const int length = end - start;
        SendScintilla(SCI_INDICATORFILLRANGE, start, length); // NOLINT
        SendScintilla(SCI_SETCURRENTPOS, end); // NOLINT
        SendScintilla(SCI_SEARCHANCHOR); // NOLINT
    }
    SendScintilla(SCI_SETSEL, anchorPos, currentPos); // NOLINT
}

// ScriptEditor protected
void ScriptEditor::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);
    menu.addAction(QIcon(":/icon/textCollapse.svg"), tr("Collapse All"), this, [this] { SendScintilla(SCI_FOLDALL, SC_FOLDACTION_CONTRACT); }); // NOLINT
    menu.addAction(QIcon(":/icon/textExpand.svg"), tr("Expand All"), this, [this] { SendScintilla(SCI_FOLDALL, SC_FOLDACTION_EXPAND); }); // NOLINT
    QMenu *dockMenu = menu.addMenu(QIcon(":/icon/dock.svg"), tr("Dock Position"));
    dockMenu->addAction(QIcon(":/icon/splitRight.svg"), tr("Dock Right"), this, [this] { emit dockRight(); }); // NOLINT
    dockMenu->addAction(QIcon(":/icon/splitLeft.svg"), tr("Dock Left"), this, [this] { emit dockLeft(); }); // NOLINT
    dockMenu->addAction(QIcon(":/icon/splitUp.svg"), tr("Dock Top"), this, [this] { emit dockTop(); }); // NOLINT
    dockMenu->addAction(QIcon(":/icon/splitDown.svg"), tr("Dock Bottom"), this, [this] { emit dockBottom(); }); // NOLINT
    menu.addAction(tr("Formatting"), this, &ScriptEditor::requestFormatting);
    menu.exec(event->globalPos());
}

void ScriptEditor::keyPressEvent(QKeyEvent *event) {
    if (isReadOnly()) {
        emit requestPermission();
        event->accept();
        return;
    }
    switch (event->key()) {
        case Qt::Key_Slash: {
            if (event->modifiers() == Qt::ControlModifier) {
                commentHandle();
                event->accept();
                return;
            }
        }
        break;
        case Qt::Key_D: {
            if (event->modifiers() == Qt::ControlModifier) {
                duplicateHandle();
                event->accept();
                return;
            }
        }
        break;
        case Qt::Key_Control: {
            m_ctrlPressed = true;
        }
        break;
        default:
            break;
    }
    QsciScintilla::keyPressEvent(event);
}

void ScriptEditor::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Control) {
        m_ctrlPressed = false;
        definitionHandle();
        event->accept();
    }
    QsciScintilla::keyReleaseEvent(event);
}

void ScriptEditor::mouseMoveEvent(QMouseEvent *event) {
    if (m_ctrlPressed) {
        definitionHandle();
        event->accept();
        return;
    }
    QsciScintilla::mouseMoveEvent(event);
}

void ScriptEditor::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_jumpValid) {
        const QPoint mousePos = event->pos();
        const long charPos = SendScintilla(SCI_POSITIONFROMPOINTCLOSE, mousePos.x(), mousePos.y());
        if (charPos != -1) {
            const int line = SendScintilla(SCI_LINEFROMPOSITION, charPos);
            const int character = charPos - SendScintilla(SCI_POSITIONFROMLINE, line);
            emit requestDefinition(line, character);
        }
        event->accept();
        return;
    }
    QsciScintilla::mousePressEvent(event);
}

// ScriptEditor private
void ScriptEditor::pairHandle(const int ascii) {
    const auto input = QChar(ascii);
    if (!m_autoPairHash.contains(input)) return;
    insert(m_autoPairHash[input]);
}

void ScriptEditor::commentHandle() {
    int startLine, startCharacter, endLine, endCharacter;
    getSelection(&startLine, &startCharacter, &endLine, &endCharacter);
    if (startLine == -1) {
        getCursorPosition(&startLine, &startCharacter);
        endLine = startLine;
    }
    beginUndoAction();
    for (int line = startLine; line <= endLine; ++line) {
        QString lineText = text(line);
        if (lineText.startsWith("-- ")) {
            setSelection(line, 0, line, 3);
            removeSelectedText();
        } else {
            insertAt("-- ", line, 0);
        }
    }
    endUndoAction();
}

void ScriptEditor::duplicateHandle() {
    int currentLine, currentCharacter;
    getCursorPosition(&currentLine, &currentCharacter);
    const QString lineText = text(currentLine);
    beginUndoAction();
    insertAt(lineText, currentLine + 1, 0);
    setCursorPosition(currentLine + 1, currentCharacter);
    endUndoAction();
}

void ScriptEditor::definitionHandle() {
    const int lastLine = lines() - 1;
    const int lastIndex = lineLength(lastLine);
    clearIndicatorRange(0, 0, lastLine, lastIndex, INDICATOR_HYPERLINK);
    if (m_ctrlPressed) {
        const QPoint mousePos = mapFromGlobal(QCursor::pos());
        const int x = mousePos.x();
        const int y = mousePos.y();
        if (const long charPos = SendScintilla(SCI_POSITIONFROMPOINTCLOSE, x, y); charPos != -1) {
            const long wordStart = SendScintilla(SCI_WORDSTARTPOSITION, charPos, true);
            const long wordEnd = SendScintilla(SCI_WORDENDPOSITION, charPos, true);
            if (wordStart < wordEnd) {
                m_jumpValid = true;
                viewport()->setCursor(Qt::PointingHandCursor);
                const int startLine = SendScintilla(SCI_LINEFROMPOSITION, wordStart);
                const int startIndex = wordStart - SendScintilla(SCI_POSITIONFROMLINE, startLine);
                const int endLine = SendScintilla(SCI_LINEFROMPOSITION, wordEnd);
                const int endIndex = wordEnd - SendScintilla(SCI_POSITIONFROMLINE, endLine);
                fillIndicatorRange(startLine, startIndex, endLine, endIndex, INDICATOR_HYPERLINK);
            } else {
                m_jumpValid = false;
                viewport()->setCursor(Qt::IBeamCursor);
            }
        } else {
            m_jumpValid = false;
            viewport()->setCursor(Qt::IBeamCursor);
        }
    } else {
        m_jumpValid = false;
        viewport()->setCursor(Qt::IBeamCursor);
    }
}
