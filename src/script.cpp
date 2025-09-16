#include "../include/script.h"

static Script *g_script = nullptr;
QList<int> g_breakpoint;

auto g_stateMachine = STATE_RUN;
int g_depth = 0;
int g_baseDepth = 0;

// Script public
Script::Script(QWidget *parent) : QWidget(parent), m_tooltipHover(new TooltipHover(this)) {
    g_script = this;
    auto shortcutSave = new QShortcut(QKeySequence(m_scriptConfig["formatting"].toString()), this); // NOLINT
    connect(shortcutSave, &QShortcut::activated, this, [this] {
        if (!m_currentScriptPage) return;
        m_currentScriptPage->formattingRequest();
        emit appendLog("script formatted", "info");
    });
    // script module init
    auto *layout = new QHBoxLayout(this); // NOLINT
    auto *scriptSplitter = new QSplitter(Qt::Horizontal); // NOLINT
    layout->addWidget(scriptSplitter);
    // script widget -> script editor
    m_scriptTabWidget = new QTabWidget();
    scriptSplitter->addWidget(m_scriptTabWidget);
    m_scriptTabWidget->setTabsClosable(true);
    m_scriptTabWidget->setMovable(true);
    auto *welcomePage = new QWidget(); // NOLINT
    if (const int scriptCount = m_scriptConfig["scriptList"].toArray().size(); scriptCount == 0) {
        m_scriptTabWidget->addTab(welcomePage, "welcome");
        auto *welcomeLayout = new QVBoxLayout(welcomePage); // NOLINT
        auto *welcomeBrowser = new QTextBrowser(); // NOLINT
        welcomeLayout->addWidget(welcomeBrowser);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "no script config found, create a welcome page");
    } else {
        for (const QJsonValue &value: m_scriptConfig["scriptList"].toArray()) {
            const QString scriptUrl = value.toString();
            auto *newTab = new ScriptPageWidget(m_scriptConfig, scriptUrl); // NOLINT
            m_currentScriptPage = newTab;
            const QFileInfo scriptInfo(scriptUrl);
            const QString scriptName = scriptInfo.fileName();
            m_scriptTabWidget->addTab(newTab, scriptName);
            m_scriptTabWidget->setCurrentWidget(newTab);
            connect(newTab, &ScriptPageWidget::modifyScript, this, [this, newTab] {
                scriptModify(m_scriptTabWidget->indexOf(newTab));
            });
            connect(newTab, &ScriptPageWidget::requestJson, this, &Script::requestJson);
            connect(newTab, &ScriptPageWidget::notificationJson, this, &Script::notificationJson);
        }
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3").arg(timestamp, QString::number(scriptCount), "script config found");
    }
    connect(m_scriptTabWidget, &QTabWidget::currentChanged, this, &Script::scriptSelected);
    connect(m_scriptTabWidget, &QTabWidget::tabCloseRequested, this, &Script::scriptClose);
    connect(m_scriptTabWidget->tabBar(), &QTabBar::tabMoved, this, &Script::scriptSwap);

    // script widget -> ctrl widget
    auto *ctrlWidget = new QWidget(); // NOLINT
    m_scriptTabWidget->setCornerWidget(ctrlWidget);
    auto *ctrlLayout = new QHBoxLayout(ctrlWidget); // NOLINT
    ctrlLayout->setContentsMargins(0, 0, 0, 0);
    ctrlLayout->setAlignment(Qt::AlignRight);
    auto *runButton = new QPushButton(); // NOLINT
    ctrlLayout->addWidget(runButton);
    runButton->setFixedSize(24, 24);
    runButton->setIcon(QIcon(":/icon/play.svg"));
    connect(runButton, &QPushButton::clicked, this, &Script::scriptRun);
    auto *debugButton = new QPushButton(); // NOLINT
    ctrlLayout->addWidget(debugButton);
    debugButton->setFixedSize(24, 24);
    debugButton->setIcon(QIcon(":/icon/bug.svg"));
    connect(debugButton, &QPushButton::clicked, this, &Script::scriptDebug);

    // script monitor widget
    auto *scriptMonitorWidget = new QWidget(); // NOLINT
    scriptSplitter->addWidget(scriptMonitorWidget);
    auto *scriptMonitorLayout = new QVBoxLayout(scriptMonitorWidget); // NOLINT
    scriptMonitorLayout->setContentsMargins(0, 0, 0, 0);
    auto *scriptMonitorSplitter = new QSplitter(Qt::Vertical); // NOLINT
    scriptMonitorLayout->addWidget(scriptMonitorSplitter);
    // script monitor widget -> script monitor tab widget
    m_scriptMonitorTabWidget = new QTabWidget();
    scriptMonitorSplitter->addWidget(m_scriptMonitorTabWidget);
    // script monitor widget -> script monitor tab widget -> script diagnostics widget
    m_scriptDiagnosticsTableWidget = new QTableWidget();
    m_scriptMonitorTabWidget->addTab(m_scriptDiagnosticsTableWidget, "diagnostics");
    m_scriptDiagnosticsTableWidget->setColumnCount(2);
    m_scriptDiagnosticsTableWidget->setHorizontalHeaderLabels({"code", "message"});
    m_scriptDiagnosticsTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_scriptDiagnosticsTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_scriptDiagnosticsTableWidget->verticalHeader()->setVisible(false);
    m_scriptDiagnosticsTableWidget->verticalHeader()->setDefaultSectionSize(20);
    m_scriptDiagnosticsTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_scriptDiagnosticsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(m_scriptDiagnosticsTableWidget, &QTableWidget::cellDoubleClicked, this, [this](const int row, const int col) {
        QVariantList pos = m_scriptDiagnosticsTableWidget->item(row, 0)->data(Qt::UserRole + 1).toList();
        const int startLine = pos[0].toInt();
        const int startCharacter = pos[1].toInt();
        const int endLine = pos[2].toInt();
        const int endCharacter = pos[3].toInt();
        m_currentScriptPage->m_scriptEditor->setCursorPosition(startLine, startCharacter);
        m_currentScriptPage->m_scriptEditor->fillIndicatorRange(startLine, startCharacter, endLine, endCharacter, INDICATOR_HINT);
        QTimer::singleShot(1000, [this, startLine, startCharacter, endLine, endCharacter] {
            m_currentScriptPage->m_scriptEditor->clearIndicatorRange(startLine, startCharacter, endLine, endCharacter, INDICATOR_HINT);
        });
    });
    // script monitor widget -> script monitor tab widget -> script thread pool widget
    m_scriptThreadPoolListWidget = new QListWidget();
    m_scriptMonitorTabWidget->addTab(m_scriptThreadPoolListWidget, "thread pool");
    m_scriptThreadPoolListWidget->setStyleSheet("QListWidget::item { min-height: 40px; }");
    // script monitor widget -> script monitor tab widget -> script debug widget
    m_scriptDebugWidget = new QWidget();
    m_scriptMonitorTabWidget->addTab(m_scriptDebugWidget, "debug");
    auto *scriptDebugLayout = new QVBoxLayout(m_scriptDebugWidget); // NOLINT
    scriptDebugLayout->setContentsMargins(0, 0, 0, 0);
    scriptDebugLayout->setSpacing(0);
    auto *debugCtrlWidget = new QWidget(); // NOLINT
    scriptDebugLayout->addWidget(debugCtrlWidget);
    auto *debugCtrlLayout = new QHBoxLayout(debugCtrlWidget); // NOLINT
    debugCtrlLayout->setContentsMargins(0, 0, 0, 0);
    debugCtrlLayout->setAlignment(Qt::AlignLeft);
    auto *debugContinueButton = new QPushButton(); // NOLINT
    debugCtrlLayout->addWidget(debugContinueButton);
    debugContinueButton->setFixedSize(24, 24);
    debugContinueButton->setIcon(QIcon(":/icon/debugContinue.svg"));
    debugContinueButton->setToolTip(tr("resume"));
    connect(debugContinueButton, &QPushButton::clicked, this, [this] {
        g_stateMachine = STATE_RUN;
        emit debugResume();
    });
    auto *debugPauseButton = new QPushButton(); // NOLINT
    debugCtrlLayout->addWidget(debugPauseButton);
    debugPauseButton->setFixedSize(24, 24);
    debugPauseButton->setIcon(QIcon(":/icon/pause.svg"));
    debugPauseButton->setToolTip(tr("pause"));
    connect(debugPauseButton, &QPushButton::clicked, this, [] {
        g_stateMachine = STATE_PAUSE;
    });
    auto *debugStepOverButton = new QPushButton(); // NOLINT
    debugCtrlLayout->addWidget(debugStepOverButton);
    debugStepOverButton->setFixedSize(24, 24);
    debugStepOverButton->setIcon(QIcon(":/icon/debugStepOver.svg"));
    debugStepOverButton->setToolTip(tr("step over"));
    connect(debugStepOverButton, &QPushButton::clicked, this, [this] {
        g_baseDepth = g_depth;
        g_stateMachine = STATE_STEPOVER;
        emit debugResume();
    });
    auto *debugStepIntoButton = new QPushButton(); // NOLINT
    debugCtrlLayout->addWidget(debugStepIntoButton);
    debugStepIntoButton->setFixedSize(24, 24);
    debugStepIntoButton->setIcon(QIcon(":/icon/debugStepInto.svg"));
    debugStepIntoButton->setToolTip(tr("step into"));
    connect(debugStepIntoButton, &QPushButton::clicked, this, [this] {
        g_stateMachine = STATE_STEPINTO;
        emit debugResume();
    });
    auto *debugStepOutButton = new QPushButton(); // NOLINT
    debugCtrlLayout->addWidget(debugStepOutButton);
    debugStepOutButton->setFixedSize(24, 24);
    debugStepOutButton->setIcon(QIcon(":/icon/debugStepOut.svg"));
    debugStepOutButton->setToolTip(tr("step out"));
    connect(debugStepOutButton, &QPushButton::clicked, this, [this] {
        g_baseDepth = g_depth;
        g_stateMachine = STATE_STEPOUT;
        emit debugResume();
    });
    auto *debugTerminateButton = new QPushButton(); // NOLINT
    debugCtrlLayout->addWidget(debugTerminateButton);
    debugTerminateButton->setFixedSize(24, 24);
    debugTerminateButton->setIcon(QIcon(":/icon/stop.svg"));
    debugTerminateButton->setToolTip(tr("terminate"));
    connect(debugTerminateButton, &QPushButton::clicked, this, [this] {
        g_stateMachine = STATE_TERMINATE;
        emit debugResume();
    });
    m_scriptDebugTreeView = new QTreeView();
    scriptDebugLayout->addWidget(m_scriptDebugTreeView);
    scriptTreeViewLoad({});
    // script monitor widget -> script explorer treeview
    m_scriptExplorerTreeView = new ScriptExplorer();
    scriptMonitorSplitter->addWidget(m_scriptExplorerTreeView);
    connect(m_scriptExplorerTreeView, &ScriptExplorer::appendLog, this, &Script::appendLog);
    connect(m_scriptExplorerTreeView, &ScriptExplorer::openScript, this, &Script::scriptOpen);
    connect(m_scriptExplorerTreeView, &ScriptExplorer::runScript, this, &Script::scriptRun);

    scriptSplitter->setStretchFactor(0, 3);
    scriptSplitter->setStretchFactor(1, 1);
    // diagnosticsPublish
    diagnosticsPublish();

    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "script module initialized");

    QTimer::singleShot(0, [this] {
        if (m_currentScriptPage) {
            m_currentScriptPage->foldingRangeRequest();
            m_currentScriptPage->semanticTokensRequest();
        }
    });
}

void Script::scriptConfigSave() const {
    if (!m_currentScriptPage) return;
    for (int i = 0; i < m_scriptTabWidget->count(); ++i) {
        auto *scriptPageWidget = qobject_cast<ScriptPageWidget *>(m_scriptTabWidget->widget(i));
        if (scriptPageWidget) {
            if (scriptPageWidget->m_scriptModify) {
                // update tab name
                QString tabName = m_scriptTabWidget->tabText(i);
                tabName.chop(1);
                m_scriptTabWidget->setTabText(i, tabName);
            }
            // save script
            scriptPageWidget->scriptSave();
        }
    }
    g_config["scriptConfig"] = m_scriptConfig;
}

void Script::scriptOpen(const QString &scriptUrl) {
    // gui
    // switch to existing page if already opened
    QJsonArray scriptList = m_scriptConfig["scriptList"].toArray();
    for (int i = 0; i < scriptList.size(); i++) {
        if (scriptList[i].toString() == scriptUrl) {
            m_scriptTabWidget->setCurrentIndex(i);
            return;
        }
    }
    // remove welcome page if exist
    if (m_scriptTabWidget->tabText(0) == "welcome") {
        m_scriptTabWidget->removeTab(0);
    }
    // open new tab
    auto *newTab = new ScriptPageWidget(m_scriptConfig, scriptUrl); // NOLINT
    const QFileInfo scriptInfo(scriptUrl);
    const QString scriptName = scriptInfo.fileName();
    m_scriptTabWidget->addTab(newTab, scriptName);
    m_scriptTabWidget->setCurrentWidget(newTab);
    connect(newTab, &ScriptPageWidget::modifyScript, this, [this, newTab] {
        scriptModify(m_scriptTabWidget->indexOf(newTab));
    });
    connect(newTab, &ScriptPageWidget::requestJson, this, &Script::requestJson);
    connect(newTab, &ScriptPageWidget::notificationJson, this, &Script::notificationJson);
    // config
    scriptList.append(scriptUrl);
    m_scriptConfig["scriptList"] = scriptList;
    // qDebug() << m_scriptConfig;
}

void Script::scriptHighlight(const int row) const {
    const int currentIndex = m_scriptTabWidget->currentIndex();
    if (currentIndex < 0) {
        return;
    }
    const auto scriptPageWidget = qobject_cast<ScriptPageWidget *>(m_scriptTabWidget->widget(currentIndex));
    if (!scriptPageWidget) return;

    scriptPageWidget->m_scriptEditor->markerDeleteAll(MARKER_HIGHLIGHT);
    if (row == -1) return;
    scriptPageWidget->m_scriptEditor->markerAdd(row - 1, MARKER_HIGHLIGHT);
}

void Script::scriptTreeViewLoad(QStandardItemModel *varMap) const {
    if (!varMap) {
        varMap = new QStandardItemModel(); // NOLINT
        varMap->setHorizontalHeaderLabels({"Name", "Type", "Value"});
    }
    m_scriptDebugTreeView->setModel(varMap);
    connect(varMap, &QStandardItemModel::itemChanged, this, [this](const QStandardItem *item) {
        if (item->column() == 2) {
            const QString varScope = item->data(Qt::UserRole + 1).toString();
            const QString varName = item->data(Qt::UserRole + 2).toString();
            const QString varValue = item->text();
            QMetaObject::invokeMethod(m_debugInterpreter, [this, varScope, varName, varValue] {
                m_debugInterpreter->hotUpdate(varScope, varName, varValue);
            }, Qt::QueuedConnection);
        }
    });
    m_scriptDebugTreeView->expandAll();
    m_scriptDebugTreeView->resizeColumnToContents(0);
    m_scriptDebugTreeView->resizeColumnToContents(1);
}

void Script::diagnosticsReturn(const QString &scriptUri, const QJsonArray &diagnosticsArray) {
    m_diagnosticsHash.insert(scriptUri, diagnosticsArray);
    diagnosticsPublish();
}

void Script::diagnosticsPublish() const {
    if (!m_currentScriptPage) return;
    const QJsonArray &diagnosticsArray = m_diagnosticsHash[m_currentScriptPage->m_scriptUrl];
    // diagnostics annotate
    const int lastLine = m_currentScriptPage->m_scriptEditor->lines() - 1;
    const int lastIndex = m_currentScriptPage->m_scriptEditor->lineLength(lastLine);
    m_currentScriptPage->m_scriptEditor->clearIndicatorRange(0, 0, lastLine, lastIndex, INDICATOR_ERROR);
    // diagnostics table
    m_scriptDiagnosticsTableWidget->setRowCount(0);
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
        if (severity == 2) {
            // error
            m_currentScriptPage->m_scriptEditor->fillIndicatorRange(startLine, startCharacter, endLine, endCharacter, INDICATOR_ERROR);
        } else if (severity == 4) {
            // warning
            m_currentScriptPage->m_scriptEditor->fillIndicatorRange(startLine, startCharacter, endLine, endCharacter, INDICATOR_WARNING);
        }
        const QString code = diagnosticObject["code"].toString();
        const QString message = diagnosticObject["message"].toString();
        m_scriptDiagnosticsTableWidget->insertRow(row);
        auto *codeItem = new QTableWidgetItem(code); // NOLINT
        codeItem->setData(Qt::UserRole + 1, QVariantList({startLine, startCharacter, endLine, endCharacter}));
        auto *messageItem = new QTableWidgetItem(message); // NOLINT
        if (severity == 2) {
            // error
            codeItem->setBackground(QColor(255, 230, 230));
            messageItem->setBackground(QColor(255, 230, 230));
        } else if (severity == 4) {
            // warning
            codeItem->setBackground(QColor(255, 245, 230));
            messageItem->setBackground(QColor(255, 245, 230));
        }
        m_scriptDiagnosticsTableWidget->setItem(row, 0, codeItem);
        m_scriptDiagnosticsTableWidget->setItem(row, 1, messageItem);
        row++;
    }
}

void Script::completionReturn(const QJsonArray &items) const {
    m_currentScriptPage->m_tooltipCompletion->showTooltip(items);
    const auto *editor = qobject_cast<QsciScintilla *>(m_currentScriptPage->m_scriptEditor);
    const long currentPos = editor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    const long wordStartPos = editor->SendScintilla(QsciScintilla::SCI_WORDSTARTPOSITION, currentPos, true);
    const int x = editor->SendScintilla(QsciScintilla::SCI_POINTXFROMPOSITION, 0, wordStartPos);
    const int y = editor->SendScintilla(QsciScintilla::SCI_POINTYFROMPOSITION, 0, wordStartPos);
    const QPoint cursorGlobalPos = editor->mapToGlobal(QPoint(x, y));
    const int lineHeight = editor->SendScintilla(QsciScintilla::SCI_TEXTHEIGHT, 0);
    m_currentScriptPage->m_tooltipCompletion->move(cursorGlobalPos.x() - 18, cursorGlobalPos.y() + lineHeight);
}

void Script::foldingRangeReturn(const QJsonArray &result) const {
    QMap<int, int> deltaDepthMap;
    for (const QJsonValue &value: result) {
        const int startLine = value["startLine"].toInt();
        const int endLine = value["endLine"].toInt();
        deltaDepthMap.insert(startLine + 1, deltaDepthMap.value(startLine + 1, 0) + 1);
        deltaDepthMap.insert(endLine + 1, deltaDepthMap.value(endLine + 1, 0) - 1);
    }
    int currentDepth = 0;
    for (int line = 0; line < m_currentScriptPage->m_scriptEditor->lines(); line++) {
        const int deltaDepth = deltaDepthMap.value(line, 0);
        currentDepth += deltaDepth;
        int level = QsciScintilla::SC_FOLDLEVELBASE + currentDepth;
        if (deltaDepthMap.value(line + 1, 0) > 0) level |= QsciScintilla::SC_FOLDLEVELHEADERFLAG;
        m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETFOLDLEVEL, line, level); // NOLINT
    }
}

void Script::formattingReturn(const QString &newText) const {
    m_currentScriptPage->m_scriptEditor->setText(newText);
}

void Script::hoverReturn(const QString &message) const {
    m_tooltipHover->showTooltip(message);
}

void Script::semanticTokensReturn(const QJsonArray &data) const {
    // color format is BGR!!! DO NOT FORGET!!!
    m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_PARAMETER, static_cast<long>(0x000000)); // NOLINT
    m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_VARIABLE, static_cast<long>(0x000000)); // NOLINT
    m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_FUNCTION_DECLARATION, static_cast<long>(0x7A6200)); // NOLINT
    m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_FUNCTION_CALL, static_cast<long>(0x000000)); // NOLINT
    m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_METHOD, static_cast<long>(0x000000)); // NOLINT
    m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_KEYWORD, static_cast<long>(0xB33300)); // NOLINT
    m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_COMMENT, static_cast<long>(0x8C8C8C)); // NOLINT
    m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_STRING, static_cast<long>(0x177D06)); // NOLINT
    m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_NUMBER, static_cast<long>(0xEB5017)); // NOLINT
    m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUATOKEN_OPERATOR, static_cast<long>(0x000000)); // NOLINT

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
        const int startPos = m_currentScriptPage->m_scriptEditor->positionFromLineIndex(currentLine, currentChar);
        const int endPos = startPos + length;
        if (startPos < 0 || endPos > m_currentScriptPage->m_scriptEditor->length() || length <= 0) {
            qDebug() << "skip token" << currentLine << currentChar << length << tokenType;
            continue;
        }
        // qDebug() << currentLine << currentChar << length << tokenType << tokenModifiers;
        // start styling
        m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_STARTSTYLING, startPos, 0xFF); // NOLINT
        switch (tokenType) {
            case TOKENTYPE_PARAMETER:
                m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_PARAMETER); // NOLINT
                break;
            case TOKENTYPE_VARIABLE:
                m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_VARIABLE); // NOLINT
                break;
            case TOKENTYPE_FUNCTION:
                if (tokenModifiers == TOKENMODIFIERS_DECLARATION) {
                    m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_FUNCTION_DECLARATION); // NOLINT
                } else {
                    m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_FUNCTION_CALL); // NOLINT
                }
                break;
            case TOKENTYPE_METHOD:
                m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_METHOD); // NOLINT
                break;
            case TOKENTYPE_KEYWORD:
                m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_KEYWORD); // NOLINT
                break;
            case TOKENTYPE_COMMENT:
                m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_COMMENT); // NOLINT
                break;
            case TOKENTYPE_STRING:
                m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_STRING); // NOLINT
                break;
            case TOKENTYPE_NUMBER:
                m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_NUMBER); // NOLINT
                break;
            case TOKENTYPE_OPERATOR:
                m_currentScriptPage->m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_SETSTYLING, length, LUATOKEN_OPERATOR); // NOLINT
                break;
            default: break;
        }
    }
}

// Script private
void Script::scriptRun() {
    const int currentIndex = m_scriptTabWidget->currentIndex();
    if (currentIndex < 0) {
        return;
    }
    const QString name = m_scriptTabWidget->tabText(currentIndex);
    const auto *scriptPageWidget = qobject_cast<ScriptPageWidget *>(m_scriptTabWidget->widget(currentIndex));
    if (!scriptPageWidget) return;
    const QString script = scriptPageWidget->m_scriptEditor->text();
    // launch lua interpreter thread
    auto *worker = new QThread(); // NOLINT
    auto *interpreter = new LuaInterpreter(); // NOLINT
    interpreter->moveToThread(worker);
    connect(worker, &QThread::finished, interpreter, &LuaInterpreter::deleteLater);
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QThread::started, [interpreter, script] {
        interpreter->run(script);
        QThread::currentThread()->quit();
    });
    scriptRunning(name, worker);
    worker->start();
}

void Script::scriptRunning(const QString &name, QThread *worker) {
    auto *scriptListWidgetItem = new QListWidgetItem(); // NOLINT
    m_scriptThreadPoolListWidget->addItem(scriptListWidgetItem);
    connect(worker, &QThread::finished, this, [this, scriptListWidgetItem] {
        const int row = m_scriptThreadPoolListWidget->row(scriptListWidgetItem);
        m_scriptThreadPoolListWidget->takeItem(row);
        delete scriptListWidgetItem;
    });
    auto *scriptInfoWidget = new QWidget(); // NOLINT
    m_scriptThreadPoolListWidget->setItemWidget(scriptListWidgetItem, scriptInfoWidget);
    auto *scriptInfoLayout = new QHBoxLayout(scriptInfoWidget); // NOLINT
    scriptInfoLayout->setContentsMargins(5, 0, 5, 0);
    auto *scriptLabel = new QLabel(QDateTime::currentDateTime().toString("HH:mm:ss") + " " + name); // NOLINT
    scriptInfoLayout->addWidget(scriptLabel);
    auto *abortButton = new QPushButton(); // NOLINT
    scriptInfoLayout->addWidget(abortButton);
    abortButton->setFixedSize(24, 24);
    abortButton->setIcon(QIcon(":/icon/stop.svg"));
    connect(abortButton, &QPushButton::clicked, this, [worker] {
        worker->requestInterruption();
    });
}

void Script::scriptDebug() {
    const int currentIndex = m_scriptTabWidget->currentIndex();
    if (currentIndex < 0) {
        return;
    }
    const auto *scriptPageWidget = qobject_cast<ScriptPageWidget *>(m_scriptTabWidget->widget(currentIndex));
    if (!scriptPageWidget) return;
    QString script = scriptPageWidget->m_scriptEditor->text();
    // launch lua interpreter thread
    auto *worker = new QThread(); // NOLINT
    m_debugInterpreter = new LuaInterpreter(); // NOLINT
    m_debugInterpreter->moveToThread(worker);
    connect(worker, &QThread::finished, m_debugInterpreter, &LuaInterpreter::deleteLater);
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QThread::started, [this, script] {
        m_debugInterpreter->debug(script);
        QThread::currentThread()->quit();
    });
    m_scriptMonitorTabWidget->setCurrentIndex(DEBUG_TAB); // switch to debug tab
    worker->start();
}

void Script::scriptModify(const int index) const {
    const QString tabName = m_scriptTabWidget->tabText(index) + "*";
    m_scriptTabWidget->setTabText(index, tabName);
}

void Script::scriptClose(const int index) {
    // gui
    auto *scriptPageWidget = qobject_cast<ScriptPageWidget *>(m_scriptTabWidget->widget(index));
    if (scriptPageWidget && scriptPageWidget->m_scriptModify) {
        const QMessageBox::StandardButton reply =
                QMessageBox::question(nullptr, tr("Close Script"), tr("The script has been edited. Save changes?"), QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            scriptPageWidget->scriptSave();
        }
    }
    const QWidget *tabToClose = m_scriptTabWidget->widget(index);
    m_scriptTabWidget->removeTab(index);
    delete tabToClose;
    // config
    QJsonArray scriptList = m_scriptConfig["scriptList"].toArray();
    scriptList.removeAt(index);
    m_scriptConfig["scriptList"] = scriptList;
    // qDebug() << m_scriptConfig;
}

void Script::scriptSelected(const int index) {
    const auto scriptPageWidget = qobject_cast<ScriptPageWidget *>(m_scriptTabWidget->widget(index));
    if (!scriptPageWidget) return;
    m_currentScriptPage = scriptPageWidget;
    diagnosticsPublish();
    m_currentScriptPage->foldingRangeRequest();
    m_currentScriptPage->semanticTokensRequest();
}

void Script::scriptSwap(const int srcIndex, const int dstIndex) {
    // config
    QJsonArray scriptList = m_scriptConfig["scriptList"].toArray();
    const QJsonValue tmp = scriptList.takeAt(srcIndex);
    scriptList.insert(dstIndex, tmp);
    m_scriptConfig["scriptList"] = scriptList;
    // qDebug() << m_scriptConfig;
}

// TooltipHover public
TooltipHover::TooltipHover(QWidget *parent) : QWidget(parent), m_textBrowser(new QTextBrowser(this)) {
    setWindowFlags(Qt::Popup);
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
    m_textBrowser->setMarkdown(message);
    this->adjustSize();
    this->move(QCursor::pos() + QPoint(15, 15));
    this->show();
}

void TooltipHover::hideTooltip() {
    this->hide();
}

// ScriptPageWidget public
ScriptPageWidget::ScriptPageWidget(const QJsonObject &scriptConfig, const QString &scriptUrl, QWidget *parent) : QWidget(parent), m_tooltipCompletion(new TooltipCompletion(this)) {
    auto *layout = new QVBoxLayout(this); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    m_editTimer = new QTimer(this);
    m_editTimer->setInterval(300);
    m_editTimer->setSingleShot(true);
    connect(m_editTimer, &QTimer::timeout, [this] {
        scriptEditFinish();
    });
    m_scriptEditor = new ScriptEditor();
    layout->addWidget(m_scriptEditor);
    m_scriptEditor->setFont(QFont(scriptConfig["fontFamily"].toString(), scriptConfig["fontSize"].toInt()));
    m_scriptUrl = scriptUrl;
    const QUrl url(scriptUrl);
    const QString scriptPath = url.toLocalFile();
    QFile file(scriptPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    const QString content = in.readAll();
    file.close();
    m_scriptEditor->setText(content);
    m_scriptEditor->installEventFilter(m_tooltipCompletion);
    // connect signals
    connect(m_scriptEditor, SIGNAL(modificationChanged(bool)), this, SLOT(scriptModify(bool)));
    connect(m_scriptEditor, SIGNAL(textChanged()), this, SLOT(scriptEdit()));
    connect(m_scriptEditor, SIGNAL(SCN_DWELLSTART(int,int,int)), this, SLOT(dwellStart(int,int,int)));
    connect(m_tooltipCompletion, &TooltipCompletion::replaceText, this, &ScriptPageWidget::textReplace);
    connect(m_tooltipCompletion, &TooltipCompletion::insertText, this, &ScriptPageWidget::textInsert);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, scriptPath, "opened");
    // didOpen notification to lua language server
    QTimer::singleShot(0, this, [this] {
        didOpenNotification();
    });
}

void ScriptPageWidget::scriptSave() {
    if (!m_scriptModify) return;
    // save file
    const QUrl url(m_scriptUrl);
    const QString scriptPath = url.toLocalFile();
    QFile file(scriptPath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << m_scriptEditor->text();
    file.close();
    // update status
    m_scriptEditor->setModified(false);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, m_scriptUrl, "saved");
}

void ScriptPageWidget::scriptEditFinish() {
    const long currentPos = m_scriptEditor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    const int prevChar = m_scriptEditor->SendScintilla(QsciScintilla::SCI_GETCHARAT, currentPos - 1);
    didChangeNotification();
    if ((prevChar >= 'a' && prevChar <= 'z') ||
        (prevChar >= 'A' && prevChar <= 'Z') ||
        prevChar == '.' || prevChar == ':') {
        completionRequest();
    } else {
        m_tooltipCompletion->hideTooltip();
    }
    foldingRangeRequest();
    semanticTokensRequest();
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, m_scriptUrl, "edited");
}

void ScriptPageWidget::completionRequest() {
    // completion request to lua language server
    int line, character;
    m_scriptEditor->getCursorPosition(&line, &character);
    const QJsonObject completionParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl}
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

void ScriptPageWidget::foldingRangeRequest() {
    // folding range request to lua language server
    const QJsonObject foldingRangeParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl}
            }
        }
    };
    emit requestJson("textDocument/foldingRange", foldingRangeParams);
}

void ScriptPageWidget::formattingRequest() {
    // formatting request to lua language server
    const QJsonObject formattingParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl}
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

void ScriptPageWidget::semanticTokensRequest() {
    // semanticTokens request to lua language server
    const QJsonObject semanticTokensParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl}
            }
        }
    };
    emit requestJson("textDocument/semanticTokens/full", semanticTokensParams);
}

// ScriptPageWidget private
void ScriptPageWidget::scriptModify(const bool status) {
    m_scriptModify = status;
    if (m_scriptModify) {
        emit modifyScript();
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3").arg(timestamp, m_scriptUrl, "modified");
    }
}

void ScriptPageWidget::scriptEdit() const {
    m_editTimer->stop();
    m_editTimer->start();
}

void ScriptPageWidget::dwellStart(const int pos, const int x, const int y) {
    int line, character;
    m_scriptEditor->lineIndexFromPosition(pos, &line, &character);
    if (line == 0 && character == 0) return;
    hoverRequest(line, character);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, m_scriptUrl, "hovered");
}

void ScriptPageWidget::didChangeNotification() {
    // didChange notification to lua language server
    const QString content = m_scriptEditor->text();
    const QJsonObject didChangeParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl},
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

void ScriptPageWidget::didOpenNotification() {
    // didOpen notification to lua language server
    const QJsonObject didOpenParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl},
                {"languageId", "lua"},
                {"version", m_version++},
                {"text", m_scriptEditor->text()}
            }
        }
    };
    emit notificationJson("textDocument/didOpen", didOpenParams);
}

void ScriptPageWidget::hoverRequest(const int line, const int character) {
    // hover request to lua language server
    const QJsonObject hoverParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl}
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

void ScriptPageWidget::textReplace(const QString &kind, QString &text) const {
    if (kind == "F") {
        text += "()";
    }
    const long currentPos = m_scriptEditor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    const long startPos = m_scriptEditor->SendScintilla(QsciScintilla::SCI_WORDSTARTPOSITION, currentPos, true); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETTARGETRANGE, startPos, currentPos); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintilla::SCI_REPLACETARGET, text.length(), text.toUtf8().constData()); // NOLINT
    long cursorPos;
    if (kind == "F") {
        cursorPos = startPos + text.length() - 1;
    } else {
        cursorPos = startPos + text.length();
    }
    m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETCURRENTPOS, cursorPos); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETSELECTIONSTART, cursorPos); // NOLINT
    m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETSELECTIONEND, cursorPos); // NOLINT
}

void ScriptPageWidget::textInsert(const QString &kind, QString &text) const {
    if (kind == "F") {
        text += "()";
    }
    m_scriptEditor->insert(text);
    const long currentPos = m_scriptEditor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    long cursorPos;
    if (kind == "F") {
        cursorPos = currentPos + text.length() - 1;
    } else {
        cursorPos = currentPos + text.length();
    }
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
    m_kindList = {"0", "1", "2", "F", "4", "C", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21"};
}

// TooltipCompletion protected
bool TooltipCompletion::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress && this->isVisible()) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
            case Qt::Key_Tab:
                if (!m_insertText.isEmpty()) emit replaceText(m_kind, m_insertText);
                return true;
            case Qt::Key_Return:
                if (!m_insertText.isEmpty()) emit insertText(m_kind, m_insertText);
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
void TooltipCompletion::showTooltip(const QJsonArray &items) {
    m_tableWidget->setRowCount(0);
    int row = 0;
    for (const QJsonValue &value: items) {
        QJsonObject item = value.toObject();
        const QString kind = m_kindList[item["kind"].toInt()];
        const QString label = item["label"].toString();
        const QString insertText = item["insertText"].toString(label);
        m_tableWidget->insertRow(row);
        auto *kindItem = new QTableWidgetItem(kind); // NOLINT
        auto *insertTextItem = new QTableWidgetItem(insertText); // NOLINT
        auto *labelItem = new QTableWidgetItem(label); // NOLINT
        m_tableWidget->setItem(row, 0, kindItem);
        m_tableWidget->setItem(row, 1, insertTextItem);
        m_tableWidget->setItem(row, 2, labelItem);
        row++;
    }
    if (m_tableWidget->rowCount() > 0) {
        m_currentRow = 0;
        m_tableWidget->selectRow(m_currentRow);
        m_kind = m_tableWidget->item(m_currentRow, 0)->text();
        m_insertText = m_tableWidget->item(m_currentRow, 1)->text();
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

void TooltipCompletion::moveUp() {
    if (m_currentRow == -1) return;
    if (m_currentRow > 0) {
        m_currentRow--;
        m_tableWidget->selectRow(m_currentRow);
        m_kind = m_tableWidget->item(m_currentRow, 0)->text();
        m_insertText = m_tableWidget->item(m_currentRow, 1)->text();
    }
}

void TooltipCompletion::moveDown() {
    if (m_currentRow == -1) return;
    if (m_currentRow < m_tableWidget->rowCount() - 1) {
        m_currentRow++;
        m_tableWidget->selectRow(m_currentRow);
        m_kind = m_tableWidget->item(m_currentRow, 0)->text();
        m_insertText = m_tableWidget->item(m_currentRow, 1)->text();
    }
}

// ScriptEditor public
ScriptEditor::ScriptEditor(QWidget *parent) : QsciScintilla(parent) {
    // init mouse dwell
    SendScintilla(SCI_SETMOUSEDWELLTIME, 500); // NOLINT
    // define markers
    this->markerDefine(Circle, MARKER_BREAKPOINT);
    this->setMarkerBackgroundColor(Qt::red, MARKER_BREAKPOINT);
    this->setMarkerForegroundColor(Qt::red, MARKER_BREAKPOINT);

    this->markerDefine(Background, MARKER_HIGHLIGHT);
    this->setMarkerBackgroundColor(QColor(255, 255, 0, 100), MARKER_HIGHLIGHT);
    // define indicators
    this->indicatorDefine(BoxIndicator, INDICATOR_ERROR);
    this->setIndicatorForegroundColor(Qt::red, INDICATOR_ERROR);
    this->setIndicatorDrawUnder(true, INDICATOR_ERROR);

    this->indicatorDefine(BoxIndicator, INDICATOR_WARNING);
    this->setIndicatorForegroundColor(Qt::yellow, INDICATOR_WARNING);
    this->setIndicatorDrawUnder(true, INDICATOR_WARNING);

    this->indicatorDefine(StraightBoxIndicator, INDICATOR_HINT);
    this->setIndicatorForegroundColor(Qt::cyan, INDICATOR_HINT);
    this->setIndicatorDrawUnder(true, INDICATOR_HINT);
    // set margins
    this->setMarginType(0, NumberMargin);
    this->QsciScintilla::setMarginWidth(0, "000");

    this->setMarginType(1, SymbolMargin);
    this->QsciScintilla::setMarginSensitivity(1, true);
    this->QsciScintilla::setMarginWidth(1, "16");
    connect(this, SIGNAL(marginClicked(int,int,Qt::KeyboardModifiers)),
            this, SLOT(onMarginClick(int,int,Qt::KeyboardModifiers)));

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
}

// ScriptEditor private
void ScriptEditor::onMarginClick(const int margin, const int line, Qt::KeyboardModifiers state) {
    if (margin == 1 && line >= 0) {
        if (this->markersAtLine(line) & 1 << MARKER_BREAKPOINT) {
            this->markerDelete(line, MARKER_BREAKPOINT);
        } else {
            this->markerAdd(line, MARKER_BREAKPOINT);
        }
    }
    // update g_breakpoint
    breakpointUpdate();
}

void ScriptEditor::breakpointUpdate() const {
    g_breakpoint.clear();
    for (int i = 0; i < this->lines(); ++i) {
        if (this->markersAtLine(i) & 1 << MARKER_BREAKPOINT) {
            g_breakpoint.append(i + 1);
        }
    }
    qDebug() << g_breakpoint;
}

// LuaInterpreter public
LuaInterpreter::LuaInterpreter(QObject *parent) {
    // init lua interpreter
    L = luaL_newstate();
    luaL_openlibs(L);
    // register C++ functions
    lua_register(L, "input", lua_input);
    lua_register(L, "print", lua_print);
    lua_register(L, "sleep", lua_sleep);
    // register port class
    lua_newtable(L);
    lua_pushcfunction(L, lua_portOpen);
    lua_setfield(L, -2, "open");
    lua_pushcfunction(L, lua_portClose);
    lua_setfield(L, -2, "close");
    lua_pushcfunction(L, lua_portInfo);
    lua_setfield(L, -2, "info");
    lua_pushcfunction(L, lua_portWriteText);
    lua_setfield(L, -2, "writeText");
    lua_pushcfunction(L, lua_portWriteData);
    lua_setfield(L, -2, "writeData");
    lua_pushcfunction(L, lua_portReadText);
    lua_setfield(L, -2, "readText");
    lua_pushcfunction(L, lua_portReadData);
    lua_setfield(L, -2, "readData");
    lua_setglobal(L, "port");
    // register modbus rtu class
    lua_newtable(L);
    lua_pushcfunction(L, lua_modbusRtuReadHoldingRegisters);
    lua_setfield(L, -2, "readHoldingRegisters");
    lua_pushcfunction(L, lua_modbusRtuWriteMultipleRegisters);
    lua_setfield(L, -2, "writeMultipleRegisters");
    lua_setglobal(L, "modbusRtu");
    // register modbus ascii class
    lua_newtable(L);
    lua_pushcfunction(L, lua_modbusAsciiReadHoldingRegisters);
    lua_setfield(L, -2, "readHoldingRegisters");
    lua_setglobal(L, "modbusAscii");
    // register database class
    lua_newtable(L);
    lua_pushcfunction(L, lua_databaseWrite);
    lua_setfield(L, -2, "write");
    lua_pushcfunction(L, lua_databaseClear);
    lua_setfield(L, -2, "clear");
    lua_setglobal(L, "database");
    // register datatable class
    lua_newtable(L);
    lua_pushcfunction(L, lua_datatableWrite);
    lua_setfield(L, -2, "write");
    lua_setglobal(L, "datatable");
}

void LuaInterpreter::run(const QString &script) const {
    // set terminate hook
    lua_sethook(L, luaTerminateHook, LUA_MASKCOUNT, 100);
    // lua exec
    if (const int result = luaL_dostring(L, script.toUtf8().constData()); result != LUA_OK) {
        const QString error = lua_tostring(L, -1);
        int row = -1;
        static const QRegularExpression re(R"(\]:(\d+):)");
        if (const auto match = re.match(error); match.hasMatch()) row = match.captured(1).toInt();
        QMetaObject::invokeMethod(g_script, [row, error] {
            g_script->scriptHighlight(row);
            g_script->appendLog(error, "error");
        }, Qt::QueuedConnection);
        lua_pop(L, 1);
    }
    // remove terminate hook
    lua_sethook(L, nullptr, 0, 0);
    // close interpreter
    lua_close(L);
}

void LuaInterpreter::debug(const QString &script) {
    co = lua_newthread(L);
    // set debug hook
    lua_sethook(co, &luaDebugHook, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0);
    // lua load
    if (luaL_loadstring(co, script.toUtf8().constData()) != LUA_OK) {
        const QString error = lua_tostring(co, -1);
        int row = -1;
        static const QRegularExpression re(R"(\]:(\d+):)");
        if (const auto match = re.match(error); match.hasMatch()) row = match.captured(1).toInt();
        QMetaObject::invokeMethod(g_script, [row, error] {
            g_script->scriptHighlight(row);
            g_script->appendLog(error, "error");
        }, Qt::QueuedConnection);
        lua_pop(co, 1);
        lua_close(L);
        return;
    }
    g_script->scriptHighlight(-1);
    g_stateMachine = STATE_RUN;
    g_depth = 0;
    // lua exec
    while (true) {
        int nresults = 0;
        int status = lua_resume(co, L, 0, &nresults);
        if (status == LUA_OK) {
            QMetaObject::invokeMethod(g_script, [] {
                g_script->scriptTreeViewLoad({});
            }, Qt::QueuedConnection);
            break;
        } else if (status == LUA_YIELD) {
            QEventLoop loop;
            connect(g_script, &Script::debugResume, &loop, &QEventLoop::quit);
            loop.exec();
            QMetaObject::invokeMethod(g_script, [] {
                g_script->scriptHighlight(-1);
            }, Qt::QueuedConnection);
        } else {
            const QString error = lua_tostring(co, -1);
            int row = -1;
            static const QRegularExpression re(R"(\]:(\d+):)");
            if (const auto match = re.match(error); match.hasMatch()) row = match.captured(1).toInt();
            QMetaObject::invokeMethod(g_script, [row, error] {
                g_script->scriptHighlight(row);
                g_script->appendLog(error, "error");
                // clear if manually terminated
                if (g_stateMachine == STATE_TERMINATE) {
                    g_script->scriptHighlight(-1);
                    g_script->scriptTreeViewLoad({});
                }
            }, Qt::QueuedConnection);
            lua_pop(co, 1);
            break;
        }
    }
    // close interpreter
    lua_close(L);
}

void LuaInterpreter::hotUpdate(const QString &varScope, const QString &varName, const QString &varValue) const {
    lua_Debug ar;
    if (varScope == "local") {
        // local table
        if (varName.contains(".")) {
            QStringList path = varName.split('.');

            if (lua_getstack(co, 0, &ar)) {
                int i = 1;
                QString firstname;
                while ((firstname = lua_getlocal(co, &ar, i)) != nullptr) {
                    if (path.first() == firstname) {
                        break;
                    }
                    lua_pop(co, 1);
                    i++;
                }

                for (int j = 1; j + 1 < path.size(); ++j) {
                    const QString &part = path[j];
                    lua_pushstring(co, part.toUtf8().constData());
                    lua_gettable(co, -2);
                    lua_remove(co, -2);
                }

                const QByteArray lastname = path.last().toUtf8();
                lua_pushstring(co, lastname.constData());
                lua_gettable(co, -2);
                lua_pushqstring(co, -1, varValue);
                lua_remove(co, -2);
                lua_setfield(co, -2, lastname.constData());
                lua_pop(co, 1);

                qDebug() << "local table" << varName << "updated to" << varValue;
            }
        } else {
            // local boolean/number/string
            if (lua_getstack(co, 0, &ar)) {
                int i = 1;
                QString name;
                while ((name = lua_getlocal(co, &ar, i)) != nullptr) {
                    if (varName == name) {
                        lua_pushqstring(co, -1, varValue);
                        lua_setlocal(co, &ar, i);
                        lua_pop(co, 1);
                        qDebug() << "local variable" << varName << "updated to" << varValue;
                        break;
                    }
                    lua_pop(co, 1);
                    i++;
                }
            }
        }
    } else {
        if (lua_getstack(co, 0, &ar)) {
            lua_getinfo(co, "f", &ar);
            int i = 1;
            // up table
            if (varName.contains('.')) {
                QStringList path = varName.split('.');
                QString firstname;
                while ((firstname = lua_getupvalue(co, -1, i)) != nullptr) {
                    if (path.first() == firstname) {
                        break;
                    }
                    lua_pop(co, 1);
                    i++;
                }
                for (int j = 1; j + 1 < path.size(); ++j) {
                    const QString &part = path[j];
                    lua_pushstring(co, part.toUtf8().constData());
                    lua_gettable(co, -2);
                    lua_remove(co, -2);
                }
                const QByteArray lastname = path.last().toUtf8();
                lua_pushstring(co, lastname.constData());
                lua_gettable(co, -2);
                lua_pushqstring(co, -1, varValue);
                lua_remove(co, -2);
                lua_setfield(co, -2, lastname.constData());
                lua_pop(co, 1);
                qDebug() << "up table" << varName << "updated to" << varValue;
                lua_pop(co, 1);
            } else {
                // up boolean/number/string
                QString name;
                while ((name = lua_getupvalue(co, -1, i)) != nullptr) {
                    if (varName == name) {
                        lua_pushqstring(co, -1, varValue);
                        lua_setupvalue(co, -3, i);
                        qDebug() << "up variable" << varName << "updated to" << varValue;
                        break;
                    }
                    lua_pop(co, 1);
                    i++;
                }
                lua_pop(co, 1);
            }
        }
    }
}

// LuaInterpreter private
void LuaInterpreter::luaTerminateHook(lua_State *L, lua_Debug *ar) {
    (void) ar;
    // check if thread interruption is requested
    if (QThread::currentThread()->isInterruptionRequested()) {
        luaL_error(L, "terminated");
    }
}

void LuaInterpreter::luaDebugHook(lua_State *L, lua_Debug *ar) {
    if (ar->event == LUA_HOOKCALL) {
        g_depth += 1;
    } else if (ar->event == LUA_HOOKRET) {
        g_depth -= 1;
    } else if (ar->event == LUA_HOOKLINE) {
        const int row = ar->currentline;
        if (g_stateMachine == STATE_TERMINATE) {
            lua_pushstring(L, "terminated");
            lua_error(L);
            return;
        }
        if (g_breakpoint.contains(row)) g_stateMachine = STATE_PAUSE;
        if (g_stateMachine == STATE_STEPOVER && g_depth == g_baseDepth) g_stateMachine = STATE_PAUSE;
        if (g_stateMachine == STATE_STEPOUT && g_depth < g_baseDepth) g_stateMachine = STATE_PAUSE;
        if (g_stateMachine == STATE_STEPINTO) g_stateMachine = STATE_PAUSE;
        if (g_stateMachine == STATE_PAUSE) {
            // highlight
            QMetaObject::invokeMethod(g_script, [row] {
                g_script->scriptHighlight(row);
            }, Qt::BlockingQueuedConnection);
            // init var map
            auto *varMap = new QStandardItemModel(); // NOLINT
            varMap->setHorizontalHeaderLabels({"Name", "Type", "Value"});
            // table recursion lambda
            auto appendTable = [](lua_State *L, QStandardItem *parentNameItem, const QString &parentVarname, const QString &parentVarScope, const int tableIndex,
                                  auto &&self) -> void {
                lua_pushnil(L);
                while (lua_next(L, tableIndex) != 0) {
                    lua_pushvalue(L, -2);
                    QString varName = lua_tostring(L, -1);
                    lua_pop(L, 1);
                    QString varType = lua_typename(L, lua_type(L, -1));
                    lua_pushvalue(L, -1);
                    QString varValue = lua_toqstring(L, -1);
                    lua_pop(L, 1);

                    auto *localNameItem = new QStandardItem(varName); // NOLINT
                    localNameItem->setEditable(false);
                    auto *localTypeItem = new QStandardItem(varType); // NOLINT
                    localTypeItem->setEditable(false);
                    QStandardItem *valueItem = new QStandardItem(varValue); // NOLINT

                    if (lua_type(L, -1) == LUA_TBOOLEAN || lua_type(L, -1) == LUA_TNUMBER || lua_type(L, -1) == LUA_TSTRING) {
                        valueItem->setData(parentVarScope, Qt::UserRole + 1);
                        valueItem->setData(parentVarname + "." + varName, Qt::UserRole + 2);
                    } else if (lua_type(L, -1) == LUA_TTABLE) {
                        valueItem->setEditable(false);
                        self(L, localNameItem, parentVarname + "." + varName, parentVarScope, lua_gettop(L), self);
                    } else {
                        valueItem->setEditable(false);
                    }
                    parentNameItem->appendRow({localNameItem, localTypeItem, valueItem});
                    lua_pop(L, 1);
                }
            };
            // local var
            auto *localVar = new QStandardItem("local"); // NOLINT
            localVar->setEditable(false);
            varMap->appendRow(localVar);
            int i = 1;
            QString localVarName;
            while ((localVarName = lua_getlocal(L, ar, i)) != nullptr) {
                if (localVarName[0] != '(') {
                    QString localVarType = lua_typename(L, lua_type(L, -1));
                    QString localVarValue = lua_toqstring(L, -1);

                    QStandardItem *localNameItem = new QStandardItem(localVarName); // NOLINT
                    localNameItem->setEditable(false);
                    QStandardItem *localTypeItem = new QStandardItem(localVarType); // NOLINT
                    localTypeItem->setEditable(false);
                    QStandardItem *localValueItem = new QStandardItem(localVarValue); // NOLINT

                    if (lua_type(L, -1) == LUA_TBOOLEAN || lua_type(L, -1) == LUA_TNUMBER || lua_type(L, -1) == LUA_TSTRING) {
                        localValueItem->setData("local", Qt::UserRole + 1);
                        localValueItem->setData(localVarName, Qt::UserRole + 2);
                    } else if (lua_type(L, -1) == LUA_TTABLE) {
                        localValueItem->setEditable(false);
                        appendTable(L, localNameItem, localVarName, "local", lua_gettop(L), appendTable);
                    } else {
                        localValueItem->setEditable(false);
                    }
                    localVar->appendRow({localNameItem, localTypeItem, localValueItem});
                }
                lua_pop(L, 1);
                i++;
            }
            // up var
            auto *upVar = new QStandardItem("up"); // NOLINT
            upVar->setEditable(false);
            varMap->appendRow(upVar);
            lua_getinfo(L, "f", ar);
            i = 1;
            QString upVarName;
            while ((upVarName = lua_getupvalue(L, -1, i)) != nullptr) {
                if (upVarName[0] != '(' && upVarName[0] != '_') {
                    QString upVarType = lua_typename(L, lua_type(L, -1));
                    QString upVarValue = lua_toqstring(L, -1);

                    QStandardItem *upNameItem = new QStandardItem(upVarName); // NOLINT
                    upNameItem->setEditable(false);
                    QStandardItem *upTypeItem = new QStandardItem(upVarType); // NOLINT
                    upTypeItem->setEditable(false);
                    QStandardItem *upValueItem = new QStandardItem(upVarValue); // NOLINT

                    if (lua_type(L, -1) == LUA_TBOOLEAN || lua_type(L, -1) == LUA_TNUMBER || lua_type(L, -1) == LUA_TSTRING) {
                        upValueItem->setData("up", Qt::UserRole + 1);
                        upValueItem->setData(upVarName, Qt::UserRole + 2);
                    } else if (lua_type(L, -1) == LUA_TTABLE) {
                        upValueItem->setEditable(false);
                        appendTable(L, upNameItem, upVarName, "up", lua_gettop(L), appendTable);
                    } else {
                        upValueItem->setEditable(false);
                    }
                    upVar->appendRow({upNameItem, upTypeItem, upValueItem});
                }
                lua_pop(L, 1);
                i++;
            }
            lua_pop(L, 1);
            // sync to gui
            QMetaObject::invokeMethod(g_script, [varMap] {
                g_script->scriptTreeViewLoad(varMap);
            }, Qt::BlockingQueuedConnection);
            // hold thread
            lua_yield(L, 0);
        }
    }
}

// ScriptExplorer public
ScriptExplorer::ScriptExplorer(QWidget *parent) : QTreeView(parent) {
    this->installEventFilter(this);
    connect(this, &QTreeView::doubleClicked, this, &ScriptExplorer::scriptOpen);

    m_model = new QFileSystemModel();
    this->QTreeView::setModel(m_model);
    this->setHeaderHidden(true);
    this->setColumnHidden(1, true);
    this->setColumnHidden(2, true);
    this->setColumnHidden(3, true);
    this->setColumnHidden(4, true);
    m_model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);

    const QString scriptPath = QDir::current().filePath("script");

    m_model->setRootPath(scriptPath);
    this->QTreeView::setRootIndex(m_model->index(scriptPath));
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "script directory loaded");
}

// ScriptExplorer protected
void ScriptExplorer::contextMenuEvent(QContextMenuEvent *event) {
    QModelIndex index = indexAt(event->pos());
    QMenu menu(this);
    if (!index.isValid()) {
        menu.addAction(tr("new script"), this, &ScriptExplorer::scriptNew);
        menu.addAction(tr("open in explorer"), this, &ScriptExplorer::scriptOpenInExplorer);
    } else {
        menu.addAction(tr("run"), [this, index] {
            scriptRun(index);
        });
        menu.addAction(tr("open"), [this, index] {
            scriptOpen(index);
        });
        menu.addAction(tr("delete"), [this, index] {
            scriptDelete(index);
        });
    }
    menu.exec(event->globalPos());
}

bool ScriptExplorer::eventFilter(QObject *obj, QEvent *event) {
    if (obj == this && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Delete) {
            if (const QModelIndex index = this->currentIndex(); index.isValid()) {
                scriptDelete(index);
                return true;
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

// ScriptExplorer private
void ScriptExplorer::scriptRun(const QModelIndex &index) {
    const QString fileName = m_model->fileName(index);
    const QString filePath = m_model->filePath(index);
    QFile file(filePath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QString script = in.readAll();
    file.close();
    emit runScript(fileName, script);
}

void ScriptExplorer::scriptOpen(const QModelIndex &index) {
    const QString scriptPath = m_model->filePath(index);
    const QString scriptUrl = QUrl::fromLocalFile(scriptPath).toString();
    emit openScript(scriptUrl);
}

void ScriptExplorer::scriptDelete(const QModelIndex &index) {
    const QString fileName = m_model->fileName(index);
    const QMessageBox::StandardButton reply =
            QMessageBox::question(nullptr, tr("Delete Script"), tr("Are you sure to delete script?"), QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }
    if (!m_model->remove(index)) {
        emit appendLog(QString("%1 %2").arg(fileName, "delete failed"), "info");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3").arg(timestamp, fileName, "delete failed");
        return;
    }
    emit appendLog(QString("%1 %2").arg(fileName, "deleted"), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, fileName, "deleted");
}

void ScriptExplorer::scriptNew() {
    bool ok;
    QString fileName = QInputDialog::getText(nullptr, "New Script", "script name:", QLineEdit::Normal, "new", &ok);
    if (!ok || fileName.isEmpty()) {
        return;
    }
    fileName += ".lua";
    const QString filePath = QDir::current().filePath("script/" + fileName);

    if (QFile::exists(filePath)) {
        const QMessageBox::StandardButton reply =
                QMessageBox::question(nullptr, tr("File Exists"), tr("File already exists. Overwrite?"), QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) {
            return;
        }
    }

    QFile file(filePath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    file.close();

    emit appendLog(QString("%1 %2").arg(fileName, "created"), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, fileName, "created");
}

void ScriptExplorer::scriptOpenInExplorer() {
    const QDir folderPath = QDir::current().filePath("script");
    const QString folderAbsolutePath = folderPath.absolutePath();
#ifdef Q_OS_WIN
    const QString command = "explorer.exe";
    QStringList args;
    args << QDir::toNativeSeparators(folderAbsolutePath);
    QProcess::startDetached(command, args);
#endif

    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "opened in explorer");
}
