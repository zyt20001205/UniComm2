#include "scriptModule/scriptModule.h"

#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QSplitter>
#include <QTableWidget>
#include <QTextBrowser>

#include "configModule.h"
#include "globals.h"
#include "luaModule/luaControl.h"
#include "utils.h"
#include "portModule/portModule.h"

// ScriptModule public
ScriptModule::ScriptModule(QWidget *parent)
    : QWidget(parent),
      m_scriptConfig(g_config["scriptConfig"].toObject()),
      m_scriptTabWidget(new QTabWidget()),
      m_scriptTabOverlay(new QWidget(m_scriptTabWidget)) {
    // script module init
    auto *layout = new QHBoxLayout(this); // NOLINT
    layout->addWidget(m_scriptTabWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    m_scriptTabWidget->setMovable(true);
    m_scriptTabWidget->setTabsClosable(true);
    // clear invalid script url
    QJsonArray validScriptList;
    for (const auto &value: m_scriptConfig["scriptList"].toArray()) {
        if (const auto scriptUrl = QUrl(value.toString()); QFileInfo::exists(scriptUrl.toLocalFile())) {
            scriptOpen(scriptUrl);
            validScriptList.append(value);
        }
    }
    m_scriptConfig["scriptList"] = validScriptList;
    m_scriptTabWidget->setCurrentIndex(m_scriptConfig["scriptFocused"].toInt());
    connect(m_scriptTabWidget, &QTabWidget::tabCloseRequested, this, &ScriptModule::scriptClose);
    connect(m_scriptTabWidget, &QTabWidget::currentChanged, this, [this](const int index) {
        scriptSwitch(index);
    });
    connect(m_scriptTabWidget->tabBar(), &QTabBar::tabMoved, this, &ScriptModule::scriptSwap);

    auto *ctrlWidget = new QWidget(); // NOLINT
    m_scriptTabWidget->setCornerWidget(ctrlWidget);
    auto *ctrlLayout = new QHBoxLayout(ctrlWidget); // NOLINT
    ctrlLayout->setContentsMargins(0, 0, 0, 0);
    ctrlLayout->setAlignment(Qt::AlignRight);
    auto *runButton = new QPushButton(); // NOLINT
    ctrlLayout->addWidget(runButton);
    runButton->setFixedSize(24, 24);
    runButton->setIcon(QIcon(":/icon/play.svg"));
    connect(runButton, &QPushButton::clicked, this, [this] {
        if (const auto scriptPage = static_cast<ScriptPage *>(m_scriptTabWidget->currentWidget())) {
            const QUrl scriptUrl = scriptPage->m_scriptUrl;
            const QString script = scriptPage->m_scriptEditor->text();
            emit runThread(scriptUrl, script);
        }
    });
    auto *debugButton = new QPushButton(); // NOLINT
    ctrlLayout->addWidget(debugButton);
    debugButton->setFixedSize(24, 24);
    debugButton->setIcon(QIcon(":/icon/bug.svg"));
    connect(debugButton, &QPushButton::clicked, this, [this] {
        if (const auto scriptPage = static_cast<ScriptPage *>(m_scriptTabWidget->currentWidget())) {
            const QUrl scriptUrl = scriptPage->m_scriptUrl;
            const QString script = scriptPage->m_scriptEditor->text();
            emit debugThread(scriptUrl, script);
        }
    });

    m_scriptTabOverlay->installEventFilter(this);
    m_scriptTabOverlay->setStyleSheet("background-color: rgba(0, 0, 0, 96);");
    auto *overlayLayout = new QVBoxLayout(m_scriptTabOverlay); // NOLINT
    overlayLayout->setAlignment(Qt::AlignCenter);
    overlayLayout->setContentsMargins(0, 0, 0, 0);
    auto *overlayLabel = new QLabel(tr("Open Workspace")); // NOLINT
    overlayLayout->addWidget(overlayLabel);
    overlayLabel->setFont(QFont("Consolas", 12, QFont::Bold));
    overlayLabel->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: white;");
    const QJsonObject mainConfig = g_config["mainConfig"].toObject();
    overlayShow();
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "script module initialized");

    // emit switchScript to ensure receive structure analysis(currentChanged won't trigger if index is not changed)
    if (m_scriptTabWidget->count() > 0) {
        QTimer::singleShot(0, this, [this] { scriptSwitch(m_scriptTabWidget->currentIndex()); });
    }
}

void ScriptModule::workspaceOpen(const QUrl &rootUrl) {
    m_rootUrl = rootUrl;
    m_diagnosticsHash.clear();
    overlayHide();
}

void ScriptModule::scriptConfigSave() {
    // save script
    for (const ScriptPage *scriptPage: m_scriptPageHash) {
        if (scriptPage) {
            if (scriptPage->m_scriptModify) {
                // update tab name
                if (const int index = m_scriptTabWidget->indexOf(scriptPage); index != -1) {
                    QString tabName = m_scriptTabWidget->tabText(index);
                    tabName.chop(1);
                    m_scriptTabWidget->setTabText(index, tabName);
                }
            }
            // save script
            scriptPage->scriptSave();
        }
    }
    // save config
    auto scriptList = QJsonArray();
    for (const QUrl &url: m_scriptList) {
        scriptList.append(url.toString());
    }
    m_scriptConfig["scriptList"] = scriptList;
    m_scriptConfig["scriptFocused"] = m_scriptTabWidget->currentIndex();
    g_config["scriptConfig"] = m_scriptConfig;
}

void ScriptModule::scriptOpen(const QUrl &scriptUrl) {
    // check if tab exists
    auto *scriptPage = m_scriptPageHash[scriptUrl];
    if (scriptPage == nullptr) {
        // create script page
        scriptPage = new ScriptPage(m_scriptConfig, scriptUrl);
        m_scriptPageHash[scriptUrl] = scriptPage;
        connect(scriptPage, &ScriptPage::modifyScript, this, [this, scriptPage] { scriptModify(m_scriptTabWidget->indexOf(scriptPage)); });
        connect(scriptPage, &ScriptPage::insertBreakpoint, this, &ScriptModule::insertBreakpoint);
        connect(scriptPage, &ScriptPage::removeBreakpoint, this, &ScriptModule::removeBreakpoint);
        connect(scriptPage, &ScriptPage::requestJson, this, &ScriptModule::requestJson);
        connect(scriptPage, &ScriptPage::notificationJson, this, &ScriptModule::notificationJson);
        m_scriptTabWidget->addTab(scriptPage, scriptUrl.fileName());
        scriptPage->diagnosticsReturn(m_diagnosticsHash[scriptUrl]);
        // append to list
        m_scriptList.append(scriptUrl);
        // qDebug() << m_scriptList;
    }
    m_scriptTabWidget->setCurrentWidget(scriptPage);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, scriptUrl.toString(), "opened");
}

void ScriptModule::cursorPositionSet(const QUrl &scriptUrl, const int startLine, const int startCharacter) {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_scriptEditor->setCursorPosition(startLine, startCharacter);
}

void ScriptModule::cursorPositionGet() const {
    if (const auto scriptPage = static_cast<ScriptPage *>(m_scriptTabWidget->currentWidget())) {
        const QUrl scriptUrl = scriptPage->m_scriptUrl;
        int line, index;
        scriptPage->m_scriptEditor->getCursorPosition(&line, &index);
        g_cursorPosition = {
            {"url", scriptUrl},
            {"line", line + 1},
            {"character", index}
        };
    }
}

void ScriptModule::indicatorShow(const QUrl &scriptUrl, const int startLine, const int startCharacter, const int endLine, const int endCharacter, const int time) {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_scriptEditor->fillIndicatorRange(startLine, startCharacter, endLine, endCharacter, INDICATOR_HIGHLIGHT);
    QTimer::singleShot(time, [scriptPage, startLine, startCharacter, endLine, endCharacter] {
        scriptPage->m_scriptEditor->clearIndicatorRange(startLine, startCharacter, endLine, endCharacter, INDICATOR_HIGHLIGHT);
    });
}

void ScriptModule::markerShow(const QUrl &scriptUrl, const int type, const int line, const int time) const {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    if (line == -1) {
        scriptPage->m_scriptEditor->markerDeleteAll(type);
        return;
    }
    scriptPage->m_scriptEditor->markerAdd(line - 1, type);
    if (time == -1) return;
    QTimer::singleShot(time, [scriptPage, line, type] {
        scriptPage->m_scriptEditor->markerDelete(line - 1, type);
    });
}

void ScriptModule::diagnosticsReturn(const QUrl &scriptUrl, const QJsonArray &diagnosticsArray) {
    m_diagnosticsHash.insert(scriptUrl, diagnosticsArray);
    if (m_scriptPageHash.contains(scriptUrl)) {
        m_scriptPageHash[scriptUrl]->diagnosticsReturn(diagnosticsArray);
    }
}

void ScriptModule::completionReturn(const QUrl &scriptUrl, const QJsonArray &items) const {
    m_scriptPageHash[scriptUrl]->completionReturn(items);
}

void ScriptModule::foldingRangeReturn(const QUrl &scriptUrl, const QJsonArray &result) const {
    m_scriptPageHash[scriptUrl]->foldingRangeReturn(result);
}

void ScriptModule::formattingReturn(const QUrl &scriptUrl, const QString &newText) const {
    m_scriptPageHash[scriptUrl]->formattingReturn(newText);
}

void ScriptModule::hoverReturn(const QUrl &scriptUrl, const QString &message) const {
    m_scriptPageHash[scriptUrl]->hoverReturn(message);
}

void ScriptModule::semanticTokensReturn(const QUrl &scriptUrl, const QJsonArray &data) const {
    m_scriptPageHash[scriptUrl]->semanticTokensReturn(data);
}

void ScriptModule::signatureHelpReturn(const QUrl &scriptUrl, const QJsonObject &signature) const {
    m_scriptPageHash[scriptUrl]->signatureHelpReturn(signature);
}

// ScriptModule protected
bool ScriptModule::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_scriptTabOverlay && event->type() == QEvent::MouseButtonPress) {
        emit openWorkspace();
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void ScriptModule::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (!m_scriptTabOverlay->isHidden()) overlayResize();
}


// ScriptModule private
void ScriptModule::scriptSwitch(const int index) {
    const auto scriptPage = static_cast<ScriptPage *>(m_scriptTabWidget->widget(index));
    emit switchScript(scriptPage->m_scriptUrl);
}

void ScriptModule::scriptModify(const int index) const {
    QString tabName = m_scriptTabWidget->tabText(index);
    if (!tabName.endsWith("*")) {
        m_scriptTabWidget->setTabText(index, tabName + "*");
    }
}

void ScriptModule::scriptClose(const int index) {
    // find page
    if (auto *scriptPage = static_cast<ScriptPage *>(m_scriptTabWidget->widget(index))) {
        // remove hash & list
        const QUrl scriptUrl = scriptPage->m_scriptUrl;
        m_scriptPageHash.remove(scriptUrl);
        m_scriptList.removeAt(index);
        // qDebug() << m_scriptList;
        // ask for saving
        if (scriptPage->m_scriptModify) {
            const QMessageBox::StandardButton reply =
                    QMessageBox::question(nullptr, tr("Close Script"), tr("The script has been edited. Save changes?"), QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                scriptPage->scriptSave();
            }
        }
        // delete page
        scriptPage->deleteLater();
    }
    // remove tab
    m_scriptTabWidget->removeTab(index);
}

void ScriptModule::scriptSwap(const int srcIndex, const int dstIndex) {
    const QUrl tmp = m_scriptList.takeAt(srcIndex);
    m_scriptList.insert(dstIndex, tmp);
    // qDebug() << m_scriptList;
}

void ScriptModule::overlayShow() const {
    overlayResize();
    m_scriptTabOverlay->raise();
    m_scriptTabOverlay->show();
}

void ScriptModule::overlayHide() const {
    m_scriptTabOverlay->hide();
}

void ScriptModule::overlayResize() const {
    m_scriptTabOverlay->resize(m_scriptTabWidget->size());
    m_scriptTabOverlay->move(0, 0);
}

// ScriptPage public
ScriptPage::ScriptPage(const QJsonObject &scriptConfig, const QUrl &scriptUrl, QWidget *parent)
    : QWidget(parent),
      m_scriptEditor(new ScriptEditor()),
      m_tooltipCompletion(new TooltipCompletion(this)),
      m_tooltipHover(new TooltipHover(this)),
      m_tooltipPosition(new TooltipPosition(this)),
      m_tooltipSignatureHelp(new TooltipSignatureHelp(this)) {
    auto shortcutFormatting = new QShortcut(QKeySequence(scriptConfig["formatting"].toString()), this); // NOLINT
    connect(shortcutFormatting, &QShortcut::activated, this, [this] { formattingRequest(); });
    auto *layout = new QVBoxLayout(this); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    m_editTimer = new QTimer(this);
    m_editTimer->setInterval(300);
    m_editTimer->setSingleShot(true);
    connect(m_editTimer, &QTimer::timeout, [this] {
        scriptEditFinish();
    });
    layout->addWidget(m_scriptEditor);
    m_scriptEditor->setFont(QFont(scriptConfig["fontFamily"].toString(), scriptConfig["fontSize"].toInt()));
    m_scriptUrl = scriptUrl;
    const QUrl &url(scriptUrl);
    const QString scriptPath = url.toLocalFile();
    QFile file(scriptPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    const QString content = in.readAll();
    file.close();
    m_scriptEditor->setText(content);
    dwellSwitch(true);
    m_scriptEditor->installEventFilter(m_tooltipCompletion);
    m_scriptEditor->installEventFilter(m_tooltipSignatureHelp);
    // connect signals
    connect(m_scriptEditor, SIGNAL(modificationChanged(bool)), this, SLOT(scriptModify(bool)));
    connect(m_scriptEditor, SIGNAL(textChanged()), this, SLOT(scriptEdit()));
    connect(m_scriptEditor, SIGNAL(SCN_DWELLSTART(int,int,int)), this, SLOT(dwellStart(int,int,int)));
    connect(m_scriptEditor, SIGNAL(marginClicked(int,int,Qt::KeyboardModifiers)), this, SLOT(marginClick(int,int,Qt::KeyboardModifiers)));
    connect(m_tooltipCompletion, &TooltipCompletion::replaceText, this, &ScriptPage::textReplace);
    connect(m_tooltipCompletion, &TooltipCompletion::insertText, this, &ScriptPage::textInsert);
    connect(m_tooltipHover, &TooltipHover::switchDwell, this, &ScriptPage::dwellSwitch);
    connect(m_tooltipPosition, &TooltipPosition::fillPosition, this, &ScriptPage::positionFill);
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

void ScriptPage::scriptSave() const {
    if (!m_scriptModify) return;
    // save file
    const QString scriptPath = m_scriptUrl.toLocalFile();
    QFile file(scriptPath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << m_scriptEditor->text();
    file.close();
    // update status
    m_scriptEditor->setModified(false);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, m_scriptUrl.toString(), "saved");
}

void ScriptPage::completionReturn(const QJsonArray &items) const {
    m_tooltipCompletion->showTooltip(items);
    const auto *editor = static_cast<QsciScintilla *>(m_scriptEditor);
    const long currentPos = editor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    const long wordStartPos = editor->SendScintilla(QsciScintilla::SCI_WORDSTARTPOSITION, currentPos, true);
    const int x = editor->SendScintilla(QsciScintilla::SCI_POINTXFROMPOSITION, 0, wordStartPos);
    const int y = editor->SendScintilla(QsciScintilla::SCI_POINTYFROMPOSITION, 0, wordStartPos);
    const QPoint cursorGlobalPos = editor->mapToGlobal(QPoint(x, y));
    const int lineHeight = editor->SendScintilla(QsciScintilla::SCI_TEXTHEIGHT, 0);
    m_tooltipCompletion->move(cursorGlobalPos.x() - 2, cursorGlobalPos.y() + lineHeight);
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
    m_tooltipHover->showTooltip(message);
}

void ScriptPage::semanticTokensReturn(const QJsonArray &data) const {
    // clear
    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STARTSTYLING, 0, 0xFF); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, m_scriptEditor->length(), static_cast<long>(0));
    // color format is BGR!!! DO NOT FORGET!!!
    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_TYPE, static_cast<long>(0xB33300)); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_PARAMETER, static_cast<long>(0x000000)); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_VARIABLE, static_cast<long>(0x000000)); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_PROPERTY, static_cast<long>(0x7A0E66)); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_FUNCTION_DECLARATION, static_cast<long>(0x7A6200)); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_FUNCTION_CALL, static_cast<long>(0x000000)); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_METHOD, static_cast<long>(0x000000)); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_MACRO, static_cast<long>(0x2E541F)); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETBOLD, LUATOKEN_MACRO, 1); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_KEYWORD, static_cast<long>(0xB33300)); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_COMMENT, static_cast<long>(0x8C8C8C)); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_STRING, static_cast<long>(0x177D06)); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_NUMBER, static_cast<long>(0xEB5017)); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_OPERATOR, static_cast<long>(0x000000)); // NOLINT

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
        m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STARTSTYLING, startPos, 0xFF); // NOLINT
        switch (tokenType) {
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
    m_tooltipSignatureHelp->showTooltip(signature);
    const auto *editor = static_cast<QsciScintilla *>(m_scriptEditor);
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
    m_tooltipSignatureHelp->move(cursorGlobalPos.x() - 2, cursorGlobalPos.y() - lineHeight);
}

// ScriptPage private slots
void ScriptPage::scriptModify(const bool status) {
    m_scriptModify = status;
    if (m_scriptModify) {
        emit modifyScript();
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3").arg(timestamp, m_scriptUrl.toString(), "modified");
    }
}

void ScriptPage::scriptEdit() const {
    m_editTimer->stop();
    m_editTimer->start();
}

void ScriptPage::dwellStart(const int pos, const int x, const int y) {
    int line, character;
    m_scriptEditor->lineIndexFromPosition(pos, &line, &character);
    if (line == 0 && character == 0) return;
    hoverRequest(line, character);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, m_scriptUrl.toString(), "hovered");
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
    const long currentPos = m_scriptEditor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    const QChar currentChar = static_cast<char>(m_scriptEditor->SendScintilla(QsciScintilla::SCI_GETCHARAT, currentPos - 1));
    const QChar prevChar = static_cast<char>(m_scriptEditor->SendScintilla(QsciScintilla::SCI_GETCHARAT, currentPos - 2));
    didChangeNotification();
    if (currentChar.isLetter() || currentChar == '.' || currentChar == ':' || currentChar == '"') {
        completionRequest();
        m_tooltipSignatureHelp->hideTooltip();
    } else if (currentChar == '(' || currentChar == ',' || prevChar == ',') {
        QByteArray prevChars(5, 0);
        m_scriptEditor->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, qMax(0L, currentPos - 6), currentPos - 1, prevChars.data());
        if (prevChars == "Click") {
            m_tooltipPosition->showTooltip();
        } else {
            completionRequest();
            signatureHelpRequest();
            // m_tooltipCompletion->hideTooltip();
        }
    } else {
        m_tooltipCompletion->hideTooltip();
        m_tooltipSignatureHelp->hideTooltip();
    }
    documentSymbolRequest();
    foldingRangeRequest();
    semanticTokensRequest();
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, m_scriptUrl.toString(), "edited");
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

// TooltipCompletion public
TooltipCompletion::TooltipCompletion(QWidget *parent) : QWidget(parent), m_tableWidget(new QTableWidget(this)) {
    setWindowFlags(Qt::ToolTip);
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_tableWidget);
    m_tableWidget->setFixedWidth(600);
    m_tableWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_tableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_tableWidget->setFont(QFont("Consolas", 12));
    m_tableWidget->setShowGrid(false);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setColumnCount(3);
    m_tableWidget->horizontalHeader()->setVisible(false);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_tableWidget->verticalHeader()->setVisible(false);
    m_kindList = {
        "0", "Text", "Method", "Function", "Constructor", "Field", "Variable", "Class", "Interface", "Module", "Property", "Unit", "Value", "Enum", "Keyword", "Snippet", "Color",
        "File", "Reference", "Folder", "EnumMember", "Constant", "Struct", "Event", "Operator", "TypeParameter"
    };
}

void TooltipCompletion::showTooltip(const QJsonArray &items) {
    m_tableWidget->setRowCount(0);
    int row = 0;
    for (const QJsonValue &value: items) {
        QJsonObject item = value.toObject();
        const QString kind = m_kindList[item["kind"].toInt()];
        const QString label = item["label"].toString();
        const QString insertText = item["insertText"].toString(label);
        m_tableWidget->insertRow(row);
        auto *insertTextItem = new QTableWidgetItem(insertText); // NOLINT
        auto *kindItem = new QTableWidgetItem(kind); // NOLINT
        auto *labelItem = new QTableWidgetItem(label); // NOLINT
        m_tableWidget->setItem(row, 0, insertTextItem);
        m_tableWidget->setItem(row, 1, kindItem);
        m_tableWidget->setItem(row, 2, labelItem);
        row++;
    }
    if (m_tableWidget->rowCount() > 0) {
        m_currentRow = 0;
        m_tableWidget->selectRow(m_currentRow);
        m_insertText = m_tableWidget->item(m_currentRow, 0)->text();
        m_kind = m_tableWidget->item(m_currentRow, 1)->text();
    } else {
        m_currentRow = -1;
        m_kind.clear();
        m_insertText.clear();
    }
    m_tableWidget->resizeRowsToContents();
    this->adjustSize();
    this->show();
}

void TooltipCompletion::hideTooltip() {
    this->hide();
}

// TooltipCompletion protected
bool TooltipCompletion::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress && this->isVisible()) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
            case Qt::Key_Tab:
                if (!m_insertText.isEmpty()) emit replaceText(m_insertText, m_kind);
                return true;
            case Qt::Key_Return:
                if (!m_insertText.isEmpty()) emit insertText(m_insertText, m_kind);
                return true;
            case Qt::Key_Escape:
                hideTooltip();
                return true;
            case Qt::Key_Up:
                moveUp();
                return true;
            case Qt::Key_Down:
                moveDown();
                return true;
            case Qt::Key_Left:
                return true;
            case Qt::Key_Right:
                return true;
            default:
                return false;
        }
    }
    return QWidget::eventFilter(obj, event);
}

// TooltipCompletion private
void TooltipCompletion::moveUp() {
    if (m_currentRow == -1) return;
    if (m_currentRow > 0) {
        m_currentRow--;
        m_tableWidget->selectRow(m_currentRow);
        m_insertText = m_tableWidget->item(m_currentRow, 0)->text();
        m_kind = m_tableWidget->item(m_currentRow, 1)->text();
    }
}

void TooltipCompletion::moveDown() {
    if (m_currentRow == -1) return;
    if (m_currentRow < m_tableWidget->rowCount() - 1) {
        m_currentRow++;
        m_tableWidget->selectRow(m_currentRow);
        m_insertText = m_tableWidget->item(m_currentRow, 0)->text();
        m_kind = m_tableWidget->item(m_currentRow, 1)->text();
    }
}

// TooltipHover public
TooltipHover::TooltipHover(QWidget *parent)
    : QWidget(parent),
      m_textBrowser(new QTextBrowser(this)) {
    setWindowFlags(Qt::ToolTip);
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_textBrowser);
    m_textBrowser->setFixedWidth(600);
    m_textBrowser->setFont(QFont("Consolas", 10));
    m_textBrowser->setOpenExternalLinks(true);
    m_textBrowser->installEventFilter(this);
}

// TooltipHover protected
bool TooltipHover::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Leave) {
        hideTooltip();
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

// TooltipHover private
void TooltipHover::showTooltip(const QString &message) {
    emit switchDwell(false);
    m_textBrowser->setMarkdown(message);
    this->adjustSize();
    this->move(QCursor::pos() + QPoint(15, 15));
    this->show();
}

void TooltipHover::hideTooltip() {
    emit switchDwell(true);
    this->hide();
}

// TooltipPosition public
TooltipPosition::TooltipPosition(QWidget *parent)
    : QWidget(parent),
      m_timer(new QTimer(this)),
      m_label(new QLabel(this)) {
    qApp->installEventFilter(this);
    setWindowFlags(Qt::Popup);
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_label);
    m_label->setFont(QFont("consolas", 12));
    m_timer->setInterval(30);
    connect(m_timer, &QTimer::timeout, [this] {
        const QPoint logicalPos = QCursor::pos();
        this->move(logicalPos + QPoint(15, 15));
        POINT physicalPos;
        GetCursorPos(&physicalPos);
        m_label->setText(QString("X: %1, Y: %2").arg(QString::number(physicalPos.x), QString::number(physicalPos.y)));
    });
}

void TooltipPosition::showTooltip() {
    this->show();
    m_timer->start();
}

void TooltipPosition::hideTooltip() {
    this->hide();
    m_timer->stop();
}

// TooltipPosition protected
bool TooltipPosition::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress && this->isVisible()) {
        const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            POINT physicalPos;
            GetCursorPos(&physicalPos);
            emit fillPosition(physicalPos.x, physicalPos.y);
            hideTooltip();
        }
    }
    return QWidget::eventFilter(obj, event);
}

// TooltipSignatureHelp public
TooltipSignatureHelp::TooltipSignatureHelp(QWidget *parent) : QWidget(parent),
                                                              m_label(new QLabel(this)) {
    setWindowFlags(Qt::ToolTip);
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_label);
    m_label->setFont(QFont("consolas", 12));
    m_label->setStyleSheet("QLabel{background-color: white; border: 1px solid #d0d0d0;}");
}

void TooltipSignatureHelp::showTooltip(const QJsonObject &signature) {
    QString helpText;
    int index = 0;
    const int activeParameter = signature["activeParameter"].toInt();
    const QString label = signature["label"].toString();
    const QJsonArray parameters = signature["parameters"].toArray();
    for (const QJsonValue &value: parameters) {
        const QJsonObject parameter = value.toObject();
        const QJsonArray range = parameter["label"].toArray();
        const int startIndex = range[0].toInt();
        const int endIndex = range[1].toInt();
        QString param = label.mid(startIndex, endIndex - startIndex);
        if (index == activeParameter) {
            param = QString("<span style='color: orange;'>%1</span>").arg(param);
        }
        helpText += param;
        helpText += ", ";
        index++;
    }
    helpText.chop(2);
    m_label->setText(helpText);
    this->show();
}

void TooltipSignatureHelp::hideTooltip() {
    this->hide();
}

// TooltipSignatureHelp protected
bool TooltipSignatureHelp::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress && this->isVisible()) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
            case Qt::Key_Escape:
                hideTooltip();
                return true;
            default:
                return false;
        }
    }
    return QWidget::eventFilter(obj, event);
}

// ScriptEditor public
ScriptEditor::ScriptEditor(QWidget *parent)
    : QsciScintilla(parent) {
    // define markers
    this->markerDefine(Circle, MARKER_BREAKPOINT);
    this->setMarkerBackgroundColor(Qt::red, MARKER_BREAKPOINT);
    this->setMarkerForegroundColor(Qt::red, MARKER_BREAKPOINT);

    this->markerDefine(RightTriangle, MARKER_ARROW);
    this->setMarkerBackgroundColor(QColor(255, 165, 0), MARKER_ARROW);
    this->setMarkerForegroundColor(QColor(255, 165, 0), MARKER_ARROW);

    this->markerDefine(Background, MARKER_ERROR);
    this->setMarkerBackgroundColor(QColor(255, 230, 230), MARKER_ERROR);

    this->markerDefine(Background, MARKER_HINT);
    this->setMarkerBackgroundColor(Qt::cyan, MARKER_HINT);
    // define indicators
    this->indicatorDefine(StraightBoxIndicator, INDICATOR_ERROR);
    this->setIndicatorForegroundColor(QColor(255, 230, 230), INDICATOR_ERROR);
    this->setIndicatorDrawUnder(true, INDICATOR_ERROR);

    this->indicatorDefine(StraightBoxIndicator, INDICATOR_WARNING);
    this->setIndicatorForegroundColor(QColor(255, 245, 230), INDICATOR_WARNING);
    this->setIndicatorDrawUnder(true, INDICATOR_WARNING);

    this->indicatorDefine(StraightBoxIndicator, INDICATOR_INFO);
    this->setIndicatorForegroundColor(QColor(230, 240, 250), INDICATOR_INFO);
    this->setIndicatorDrawUnder(true, INDICATOR_INFO);

    this->indicatorDefine(StraightBoxIndicator, INDICATOR_HINT);
    this->setIndicatorForegroundColor(QColor(245, 245, 245), INDICATOR_HINT);
    this->setIndicatorDrawUnder(true, INDICATOR_HINT);

    this->indicatorDefine(BoxIndicator, INDICATOR_HIGHLIGHT);
    this->setIndicatorForegroundColor(Qt::red, INDICATOR_HIGHLIGHT);
    this->setIndicatorDrawUnder(true, INDICATOR_HIGHLIGHT);
    // set margins
    this->setMarginType(0, NumberMargin);
    this->QsciScintilla::setMarginWidth(0, "000");

    this->setMarginType(1, SymbolMargin);
    this->QsciScintilla::setMarginSensitivity(1, true);
    this->QsciScintilla::setMarginWidth(1, "16");

    this->QsciScintilla::setFolding(BoxedTreeFoldStyle);
    this->setMarginType(2, SymbolMargin);
    this->QsciScintilla::setMarginSensitivity(2, true);
    this->QsciScintilla::setMarginWidth(2, "16");
    // script scintilla settings
    this->setScrollWidth(1);
    this->QsciScintilla::setBraceMatching(SloppyBraceMatch);
    this->QsciScintilla::setAutoIndent(true);
    this->QsciScintilla::setBackspaceUnindents(true);
    this->QsciScintilla::setIndentationGuides(true);
    this->QsciScintilla::setTabWidth(4);
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
