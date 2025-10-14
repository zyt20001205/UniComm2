#include "scriptModule/scriptPage.h"

#include <QJsonArray>
#include <QShortcut>

#include "globals.h"
#include "utils.h"

// ScriptPage public
ScriptPage::ScriptPage(const QJsonObject &scriptConfig, const QUrl &scriptUrl)
    : DockWidget(scriptUrl.fileName()),
      m_scriptEditor(new ScriptEditor()),
      m_scriptUrl(scriptUrl),
      m_editTimer(new QTimer(this)) {
    auto shortcutFormatting = new QShortcut(QKeySequence(scriptConfig["formatting"].toString()), this); // NOLINT
    shortcutFormatting->setContext(Qt::WidgetWithChildrenShortcut);
    connect(shortcutFormatting, &QShortcut::activated, this, [this] { formattingRequest(); });
    setWidget(m_scriptEditor);
    m_scriptEditor->setFont(QFont(scriptConfig["fontFamily"].toString(), scriptConfig["fontSize"].toInt()));
    const QUrl &url(scriptUrl);
    const QString scriptPath = url.toLocalFile();
    QFile file(scriptPath);
    file.open(QIODevice::ReadOnly);
    QTextStream in(&file);
    const QString content = in.readAll();
    file.close();
    m_scriptEditor->setText(content);
    m_scriptHash = stringHashCalc(m_scriptEditor->text());
    dwellSwitch(true);
    m_editTimer->setInterval(300);
    m_editTimer->setSingleShot(true);
    connect(m_editTimer, &QTimer::timeout, [this] {
        scriptEditFinish();
    });
    // m_scriptEditor->installEventFilter(m_tooltipCompletion);
    // m_scriptEditor->installEventFilter(m_tooltipSignatureHelp);
    // connect signals
    connect(m_scriptEditor, SIGNAL(textChanged()), this, SLOT(scriptEdit()));
    connect(m_scriptEditor, SIGNAL(SCN_CHARADDED(int)), this, SLOT(charAdded(int)));
    connect(m_scriptEditor, SIGNAL(SCN_DWELLSTART(int,int,int)), this, SLOT(dwellStart(int,int,int)));
    connect(m_scriptEditor, SIGNAL(marginClicked(int,int,Qt::KeyboardModifiers)), this, SLOT(marginClick(int,int,Qt::KeyboardModifiers)));
    // connect(m_tooltipCompletion, &TooltipCompletion::replaceText, this, &ScriptPage::textReplace);
    // connect(m_tooltipCompletion, &TooltipCompletion::insertText, this, &ScriptPage::textInsert);
    // connect(m_tooltipHover, &TooltipHover::switchDwell, this, &ScriptPage::dwellSwitch);
    // connect(m_tooltipPosition, &TooltipPosition::fillPosition, this, &ScriptPage::positionFill);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, scriptPath, "opened");
    // didOpen notification to lua language server
    QTimer::singleShot(0, this, [this] {
        didOpenNotification();
        documentSymbolRequest();
        foldingRangeRequest();
        semanticTokensRequest();
    });
}

void ScriptPage::scriptSave() {
    if (!m_modified) return;
    // save file
    const QString scriptPath = m_scriptUrl.toLocalFile();
    QFile file(scriptPath);
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);
    out << m_scriptEditor->text();
    file.close();
    // update status
    m_modified = false;
    m_scriptHash = fileHashCalc(m_scriptEditor->text());
    emit modifyScript(false);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, m_scriptUrl.toString(), "saved");
}

void ScriptPage::diagnosticsReturn(const QJsonArray &diagnosticsArray) const {
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

void ScriptPage::foldingRangeReturn(const QJsonArray &result) const {
    QMap<int, int> deltaDepthMap;
    for (const QJsonValue &value: result) {
        const int startLine = value["startLine"].toInt();
        const int endLine = value["endLine"].toInt();
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

void ScriptPage::formattingReturn(const QString &newText) const {
    m_scriptEditor->setText(newText);
}

void ScriptPage::hoverReturn(const QString &message) const {
    // m_tooltipHover->showTooltip(message);
}

void ScriptPage::semanticTokensReturn(const QJsonArray &data) const {
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

void ScriptPage::signatureHelpReturn(const QJsonObject &signature) const {
    // m_tooltipSignatureHelp->showTooltip(signature);
    // const auto *editor = static_cast<QsciScintilla *>(m_scriptEditor);
    // long currentPos = editor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    // while (true) {
    //     const int prevChar = editor->SendScintilla(QsciScintilla::SCI_GETCHARAT, currentPos - 1);
    //     if (prevChar == '(') break;
    //     currentPos--;
    // }
    // const int x = editor->SendScintilla(QsciScintilla::SCI_POINTXFROMPOSITION, 0, currentPos);
    // const int y = editor->SendScintilla(QsciScintilla::SCI_POINTYFROMPOSITION, 0, currentPos);
    // const QPoint cursorGlobalPos = editor->mapToGlobal(QPoint(x, y));
    // const int lineHeight = editor->SendScintilla(QsciScintilla::SCI_TEXTHEIGHT, 0);
    // m_tooltipSignatureHelp->move(cursorGlobalPos.x() - 2, cursorGlobalPos.y() - lineHeight);
}

// ScriptPage private slots
void ScriptPage::scriptEdit() const {
    m_editTimer->stop();
    m_editTimer->start();
}

void ScriptPage::charAdded(const int ch) {
    if (const QChar character(ch); character.isLetter()) {
        // completionRequest();
    }
}

void ScriptPage::dwellStart(const int pos, const int x, const int y) {
    // int line, character;
    // m_scriptEditor->lineIndexFromPosition(pos, &line, &character);
    // if (line == 0 && character == 0) return;
    // hoverRequest(line, character);
    // // logging
    // QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    // qDebug() << QString("[%1] %2 %3").arg(timestamp, m_scriptUrl.toString(), "hovered");
}

void ScriptPage::marginClick(const int margin, const int line, Qt::KeyboardModifiers state) {
    if (margin == 1 && line >= 0) {
        if (m_scriptEditor->markersAtLine(line) & 1 << MARKER_BREAKPOINT) {
            m_scriptEditor->markerDelete(line, MARKER_BREAKPOINT);
            g_breakpoints[m_scriptUrl].remove(line + 1);
            if (g_breakpoints[m_scriptUrl].isEmpty()) g_breakpoints.remove(m_scriptUrl);
            emit removeBreakpoint(m_scriptUrl, line + 1);
        } else {
            g_breakpoints[m_scriptUrl][line + 1]["expr"] = "";
            m_scriptEditor->markerAdd(line, MARKER_BREAKPOINT);
            emit insertBreakpoint(m_scriptUrl, line + 1);
        }
    }
}


// ScriptPage private
void ScriptPage::didOpenNotification() {
    // didOpen notification to lua language server
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
    // didChange notification to lua language server
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
    // semanticTokens request to lua language server
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
    // signatureHelp request to lua language server
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
        m_modified = modified;
        emit modifyScript(modified);
    }
}

void ScriptPage::dwellSwitch(const bool status) const {
    if (status) m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETMOUSEDWELLTIME, 1000); // NOLINT
    else m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETMOUSEDWELLTIME, QsciScintilla::SC_TIME_FOREVER); // NOLINT
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

void ScriptPage::textReplace(QString &text, const QString &kind) const {
    if (kind == "Function") {
        text += "()";
    } else if (kind == "Field") {
        text += ".";
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
}

void ScriptPage::textInsert(QString &text, const QString &kind) const {
    if (kind == "Function") {
        text += "()";
    } else if (kind == "Field") {
        text += ".";
    }
    m_scriptEditor->insert(text);
    const long currentPos = m_scriptEditor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    long cursorPos;
    if (kind == "Function") {
        cursorPos = currentPos + text.length() - 1;
    } else {
        cursorPos = currentPos + text.length();
    }
    m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETCURRENTPOS, cursorPos); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETSELECTIONSTART, cursorPos); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETSELECTIONEND, cursorPos); // NOLINT
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

// ScriptEditor public
ScriptEditor::ScriptEditor(QWidget *parent)
    : QsciScintilla(parent) {
    // define markers
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
    // define indicators
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

    indicatorDefine(BoxIndicator, INDICATOR_HIGHLIGHT);
    setIndicatorForegroundColor(Qt::red, INDICATOR_HIGHLIGHT);
    setIndicatorDrawUnder(true, INDICATOR_HIGHLIGHT);
    // set margins
    setMarginType(0, NumberMargin);
    QsciScintilla::setMarginWidth(0, "000");

    setMarginType(1, SymbolMargin);
    QsciScintilla::setMarginSensitivity(1, true);
    QsciScintilla::setMarginWidth(1, "16");

    QsciScintilla::setFolding(BoxedTreeFoldStyle);
    setMarginType(2, SymbolMargin);
    QsciScintilla::setMarginSensitivity(2, true);
    QsciScintilla::setMarginWidth(2, "16");
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
    // connect auto pair
    connect(this, SIGNAL(SCN_CHARADDED(int)), this, SLOT(autoPairHandle(int)));
    m_autoPairHash['('] = ')';
    m_autoPairHash['['] = ']';
    m_autoPairHash['{'] = '}';
    m_autoPairHash['"'] = '"';
    m_autoPairHash['\''] = '\'';
}

// ScriptEditor protected
void ScriptEditor::keyPressEvent(QKeyEvent *event) {
    if (event->modifiers() == Qt::ControlModifier) {
        switch (event->key()) {
            case Qt::Key_Slash:
                commentHandle();
                event->accept();
                return;
            case Qt::Key_D:
                duplicateHandle();
                event->accept();
                return;
            default: break;
        }
    }
    QsciScintilla::keyPressEvent(event);
}

// ScriptEditor private
void ScriptEditor::autoPairHandle(const int ascii) {
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
