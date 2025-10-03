#include "../include/script.h"

auto g_stateMachine = STATE_RUN;
int g_depth = 0;
int g_baseDepth = 0;

// Script public
Script::Script(QWidget *parent) : QWidget(parent) {
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
    if (m_scriptConfig["scriptList"].toArray().isEmpty()) {
        m_scriptTabWidget->addTab(welcomePage, "welcome");
        auto *welcomeLayout = new QVBoxLayout(welcomePage); // NOLINT
        auto *workspaceOpenButton = new QPushButton(tr("Open Workspace"));
        welcomeLayout->addWidget(workspaceOpenButton);
        connect(workspaceOpenButton, &QPushButton::clicked, this, [this] {
            emit openWorkspace(QUrl());
        });
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "script list is empty, welcome page created");
    } else {
        for (const QJsonValue &value: m_scriptConfig["scriptList"].toArray()) {
            const auto scriptUrl = QUrl(value.toString());
            scriptOpen(scriptUrl);
        }
    }
    connect(m_scriptTabWidget, &QTabWidget::tabCloseRequested, this, &Script::scriptClose);
    connect(m_scriptTabWidget->tabBar(), &QTabBar::tabMoved, this, &Script::scriptSwap);
    m_scriptTabWidget->setCurrentIndex(m_scriptConfig["scriptFocused"].toInt());

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
    connect(runButton, &QPushButton::clicked, this, [this] {
        if (const auto scriptPage = qobject_cast<ScriptPage *>(m_scriptTabWidget->currentWidget())) {
            const QUrl scriptUrl = scriptPage->m_scriptUrl;
            const QString script = scriptPage->m_scriptEditor->text();
            scriptRun(scriptUrl, script);
        }
    });
    auto *debugButton = new QPushButton(); // NOLINT
    ctrlLayout->addWidget(debugButton);
    debugButton->setFixedSize(24, 24);
    debugButton->setIcon(QIcon(":/icon/bug.svg"));
    connect(debugButton, &QPushButton::clicked, this, [this] {
        if (const auto scriptPage = qobject_cast<ScriptPage *>(m_scriptTabWidget->currentWidget())) {
            const QUrl scriptUrl = scriptPage->m_scriptUrl;
            const QString script = scriptPage->m_scriptEditor->text();
            scriptDebug(scriptUrl, script);
        }
    });
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
    // script monitor widget -> script monitor tab widget -> script thread pool widget
    m_scriptThreadPoolListWidget = new QListWidget();
    m_scriptMonitorTabWidget->addTab(m_scriptThreadPoolListWidget, "threadpool");
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

    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "script module initialized");

    // open workspace
    if (const QUrl rootUrl(g_config["mainConfig"].toObject()["workspace"].toString()); !rootUrl.isEmpty()) {
        workspaceOpen(rootUrl);
    }
}

void Script::workspaceOpen(const QUrl &rootUrl) {
    m_rootUrl = rootUrl;
    m_diagnosticsHash.clear();
}

void Script::scriptConfigSave() {
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

void Script::scriptOpen(const QUrl &scriptUrl) {
    // remove welcome page if exists
    if (m_scriptTabWidget->tabText(0) == "welcome") {
        m_scriptTabWidget->removeTab(0);
    }
    // check if tab exists
    auto *scriptPage = m_scriptPageHash[scriptUrl];
    if (scriptPage == nullptr) {
        // create script page
        scriptPage = new ScriptPage(m_scriptConfig, scriptUrl);
        m_scriptPageHash[scriptUrl] = scriptPage;
        connect(scriptPage, &ScriptPage::modifyScript, this, [this, scriptPage] { scriptModify(m_scriptTabWidget->indexOf(scriptPage)); });
        connect(scriptPage, &ScriptPage::insertBreakpoint, this, [this](const QUrl &url, const int line) { m_breakpoints[url].insert(line); });
        connect(scriptPage, &ScriptPage::removeBreakpoint, this, [this](const QUrl &url, const int line) { m_breakpoints[url].remove(line); });
        connect(scriptPage, &ScriptPage::requestJson, this, &Script::requestJson);
        connect(scriptPage, &ScriptPage::notificationJson, this, &Script::notificationJson);
        m_scriptTabWidget->addTab(scriptPage, scriptUrl.fileName());
        scriptPage->diagnosticsReturn(m_diagnosticsHash[scriptUrl]);
        // append to list
        m_scriptList.append(scriptUrl);
        // qDebug() << m_scriptList;
    }
    m_scriptTabWidget->setCurrentWidget(scriptPage);

    m_currentScriptWidget = scriptPage;
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, scriptUrl.toString(), "opened");
}

void Script::scriptExec(const QString &scriptPath) {
    const QString fullPath = QDir::current().filePath(m_rootUrl.toLocalFile() + "/" + scriptPath);
    QFile file(fullPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QString script = in.readAll();
    file.close();
    // call script run
    const QUrl scriptUrl = QUrl::fromLocalFile(fullPath);
    scriptRun(scriptUrl, script);
}

void Script::cursorPositionSet(const QUrl &scriptUrl, const int startLine, const int startCharacter) {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_scriptEditor->setCursorPosition(startLine, startCharacter);
}

void Script::annotateHighlight(const QUrl &scriptUrl, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_scriptEditor->fillIndicatorRange(startLine, startCharacter, endLine, endCharacter, INDICATOR_HIGHLIGHT);
    QTimer::singleShot(1000, [scriptPage, startLine, startCharacter, endLine, endCharacter] {
        scriptPage->m_scriptEditor->clearIndicatorRange(startLine, startCharacter, endLine, endCharacter, INDICATOR_HIGHLIGHT);
    });
}

void Script::markerHighlight(const QUrl &scriptUrl, const int line) const {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_scriptEditor->markerDeleteAll(MARKER_HIGHLIGHT);
    if (line == -1) return;
    scriptPage->m_scriptEditor->markerAdd(line - 1, MARKER_HIGHLIGHT);
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

void Script::diagnosticsReturn(const QUrl &scriptUrl, const QJsonArray &diagnosticsArray) {
    m_diagnosticsHash.insert(scriptUrl, diagnosticsArray);
    if (m_scriptPageHash.contains(scriptUrl)) {
        m_scriptPageHash[scriptUrl]->diagnosticsReturn(diagnosticsArray);
    }
}

void Script::completionReturn(const QUrl &scriptUrl, const QJsonArray &items) const {
    m_scriptPageHash[scriptUrl]->completionReturn(items);
}

void Script::foldingRangeReturn(const QUrl &scriptUrl, const QJsonArray &result) const {
    m_scriptPageHash[scriptUrl]->foldingRangeReturn(result);
}

void Script::formattingReturn(const QUrl &scriptUrl, const QString &newText) const {
    m_scriptPageHash[scriptUrl]->formattingReturn(newText);
}

void Script::hoverReturn(const QUrl &scriptUrl, const QString &message) const {
    m_scriptPageHash[scriptUrl]->hoverReturn(message);
}

void Script::semanticTokensReturn(const QUrl &scriptUrl, const QJsonArray &data) const {
    m_scriptPageHash[scriptUrl]->semanticTokensReturn(data);
}

void Script::signatureHelpReturn(const QUrl &scriptUrl, const QJsonObject &signature) const {
    m_scriptPageHash[scriptUrl]->signatureHelpReturn(signature);
}

// Script private
void Script::scriptRun(const QUrl &scriptUrl, const QString &script) {
    // launch lua interpreter thread
    auto *worker = new QThread(); // NOLINT
    auto *interpreter = new LuaInterpreter(m_rootUrl, scriptUrl); // NOLINT
    interpreter->moveToThread(worker);
    connect(worker, &QThread::finished, interpreter, &LuaInterpreter::deleteLater);
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QThread::started, [interpreter, script] {
        interpreter->run(script);
        QThread::currentThread()->quit();
    });
    scriptRunning(scriptUrl.fileName(), worker);
    m_scriptMonitorTabWidget->setCurrentIndex(THREADPOOL_TAB); // switch to threadpool tab
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

void Script::scriptDebug(const QUrl &scriptUrl, const QString &script) {
    DebugData debugData;
    debugData.currentUrl = scriptUrl;
    debugData.breakpoints = &m_breakpoints;
    // launch lua interpreter thread
    auto *worker = new QThread(); // NOLINT
    m_debugInterpreter = new LuaInterpreter(m_rootUrl, scriptUrl); // NOLINT
    m_debugInterpreter->moveToThread(worker);
    connect(worker, &QThread::finished, m_debugInterpreter, &LuaInterpreter::deleteLater);
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QThread::started, [this, script, debugData] {
        m_debugInterpreter->debug(script, debugData);
        QThread::currentThread()->quit();
    });
    m_scriptMonitorTabWidget->setCurrentIndex(DEBUG_TAB); // switch to debug tab
    worker->start();
}

void Script::scriptModify(const int index) const {
    QString tabName = m_scriptTabWidget->tabText(index);
    if (!tabName.endsWith("*")) {
        m_scriptTabWidget->setTabText(index, tabName + "*");
    }
}

void Script::scriptClose(const int index) {
    // find page
    if (auto *scriptPage = qobject_cast<ScriptPage *>(m_scriptTabWidget->widget(index))) {
        // remove hash & list
        const QUrl scriptUrl = scriptPage->m_scriptUrl;
        m_scriptPageHash.remove(scriptUrl);
        m_scriptList.removeAt(index);
        // qDebug() << m_scriptList;
        // ask for saving
        if (scriptPage && scriptPage->m_scriptModify) {
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

void Script::scriptSwap(const int srcIndex, const int dstIndex) {
    const QUrl tmp = m_scriptList.takeAt(srcIndex);
    m_scriptList.insert(dstIndex, tmp);
    // qDebug() << m_scriptList;
}

// ScriptPage public
ScriptPage::ScriptPage(const QJsonObject &scriptConfig, const QUrl &scriptUrl, QWidget *parent)
    : QWidget(parent),
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
    const auto *editor = qobject_cast<QsciScintilla *>(m_scriptEditor);
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
    const auto *editor = qobject_cast<QsciScintilla *>(m_scriptEditor);
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
            emit removeBreakpoint(m_scriptUrl, line + 1);
        } else {
            m_scriptEditor->markerAdd(line, MARKER_BREAKPOINT);
            emit insertBreakpoint(m_scriptUrl, line + 1);
        }
    }
}

// ScriptPage private
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
    foldingRangeRequest();
    semanticTokensRequest();
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, m_scriptUrl.toString(), "edited");
}

void ScriptPage::dwellSwitch(const bool status) const {
    if (status) m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETMOUSEDWELLTIME, 500); // NOLINT
    else m_scriptEditor->SendScintilla(QsciScintilla::SCI_SETMOUSEDWELLTIME, QsciScintilla::SC_TIME_FOREVER); // NOLINT
}

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
TooltipHover::TooltipHover(QWidget *parent) : QWidget(parent), m_textBrowser(new QTextBrowser(this)) {
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
TooltipPosition::TooltipPosition(QWidget *parent) : QWidget(parent), m_label(new QLabel(this)), m_timer(new QTimer(this)) {
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

    this->markerDefine(Background, MARKER_HIGHLIGHT);
    this->setMarkerBackgroundColor(QColor(255, 255, 0), MARKER_HIGHLIGHT);
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

// LuaInterpreter public
LuaInterpreter::LuaInterpreter(const QUrl &rootUrl, const QUrl &scriptUrl, QObject *parent)
    : QObject(parent) {
    m_scriptUrl = scriptUrl;
    // init lua interpreter
    L = luaL_newstate();
    if (L) {
        auto *ptrHolder = static_cast<void **>(lua_getextraspace(L));
        *ptrHolder = nullptr;
    }
    luaL_openlibs(L);
    lua_getglobal(L, "package");
    // set workspace
    const QString rootPath = QString("%1/?.lua").arg(rootUrl.toLocalFile());
    lua_pushstring(L, rootPath.toUtf8().constData());
    lua_setfield(L, -2, "path");
    // register C++ functions
    lua_register(L, "exec", lua_exec);
    lua_register(L, "input", lua_input);
    lua_register(L, "print", lua_print);
    lua_register(L, "sleep", lua_sleep);
    lua_register(L, "speak", lua_speak);
    // register control class
    lua_newtable(L);
    lua_pushcfunction(L, lua_leftClick);
    lua_setfield(L, -2, "leftClick");
    lua_pushcfunction(L, lua_leftDoubleClick);
    lua_setfield(L, -2, "leftDoubleClick");
    lua_pushcfunction(L, lua_rightClick);
    lua_setfield(L, -2, "rightClick");
    lua_pushcfunction(L, lua_rightDoubleClick);
    lua_setfield(L, -2, "rightDoubleClick");
    lua_pushcfunction(L, lua_keyPress);
    lua_setfield(L, -2, "keyPress");
    lua_setglobal(L, "control");
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
    lua_pushcfunction(L, lua_datatableClear);
    lua_setfield(L, -2, "clear");
    lua_pushcfunction(L, lua_datatableExport);
    lua_setfield(L, -2, "export");
    lua_setglobal(L, "datatable");
    // register dataplot class
    lua_newtable(L);
    lua_pushcfunction(L, lua_dataplotAppend);
    lua_setfield(L, -2, "append");
    lua_setglobal(L, "dataplot");
}

LuaInterpreter::~LuaInterpreter() {
    if (L) {
        // delete extra space
        if (const auto ptrHolder = static_cast<void **>(lua_getextraspace(L)); *ptrHolder) {
            delete static_cast<DebugData *>(*ptrHolder);
            *ptrHolder = nullptr;
        }
        // close interpreter
        lua_close(L);
        L = nullptr;
    }
}

void LuaInterpreter::run(const QString &script) const {
    // set terminate hook
    lua_sethook(L, luaTerminateHook, LUA_MASKCOUNT, 100);
    // lua exec preparation
    g_script->markerHighlight(m_scriptUrl, -1);
    // lua exec
    if (const int result = luaL_dostring(L, script.toUtf8().constData()); result != LUA_OK) {
        handleError();
    }
    // remove terminate hook
    lua_sethook(L, nullptr, 0, 0);
}

void LuaInterpreter::debug(const QString &script, const DebugData &debugData) const {
    // set debug hook
    lua_sethook(L, &luaDebugHook, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0);
    // pass debug data
    const auto ptrHolder = static_cast<void **>(lua_getextraspace(L));
    const auto data = new DebugData{debugData};
    *ptrHolder = data;
    // lua debug preparation
    g_script->markerHighlight(m_scriptUrl, -1);
    g_stateMachine = STATE_RUN;
    g_depth = 0;
    // lua debug
    const QString filePath = "@" + debugData.currentUrl.toLocalFile();
    const int load_result = luaL_loadbuffer(L, script.toUtf8().constData(), script.size(), filePath.toUtf8().constData());
    if (load_result == LUA_OK) {
        const int pcall_result = lua_pcall(L, 0, LUA_MULTRET, 0);
        if (pcall_result == LUA_OK) {
            QMetaObject::invokeMethod(g_script, [this] {
                g_script->markerHighlight(m_scriptUrl, -1);
            }, Qt::QueuedConnection);
        } else {
            handleError();
        }
    } else {
        handleError();
    }
    // remove debug hook
    lua_sethook(L, nullptr, 0, 0);
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
    const auto ptrHolder = static_cast<void **>(lua_getextraspace(L));
    const auto debugData = static_cast<DebugData *>(*ptrHolder);
    if (ar->event == LUA_HOOKCALL) {
        g_depth += 1;
    } else if (ar->event == LUA_HOOKRET) {
        g_depth -= 1;
    } else if (ar->event == LUA_HOOKLINE) {
        // get file info
        lua_getinfo(L, "Sl", ar);
        const char *src = ar->source;
        QUrl nextUrl;
        if (src[0] == '@' && src[1] != '\0') {
            nextUrl = QUrl::fromLocalFile(QString::fromUtf8(src + 1));
        }
        const int line = ar->currentline;
        if (g_stateMachine == STATE_TERMINATE) {
            lua_pushstring(L, "terminated");
            lua_error(L);
            return;
        }
        if (g_stateMachine == STATE_RUN && debugData->breakpoints->value(nextUrl).contains(line)) g_stateMachine = STATE_PAUSE;
        if (g_stateMachine == STATE_STEPOVER && g_depth == g_baseDepth) g_stateMachine = STATE_PAUSE;
        if (g_stateMachine == STATE_STEPOUT && g_depth < g_baseDepth) g_stateMachine = STATE_PAUSE;
        if (g_stateMachine == STATE_STEPINTO) g_stateMachine = STATE_PAUSE;
        if (g_stateMachine == STATE_PAUSE) {
            // src handle
            if (nextUrl != debugData->currentUrl) {
                QMetaObject::invokeMethod(g_script, [nextUrl] {
                    g_script->scriptOpen(nextUrl);
                }, Qt::BlockingQueuedConnection);
                debugData->currentUrl = nextUrl;
            }
            // line handle
            QMetaObject::invokeMethod(g_script, [debugData, line] {
                g_script->markerHighlight(debugData->currentUrl, line);
            }, Qt::BlockingQueuedConnection);
            // var handle
            {
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
            }
            // hold thread
            QEventLoop loop;
            connect(g_script, &Script::debugResume, &loop, &QEventLoop::quit);
            loop.exec();
        }
    }
}

void LuaInterpreter::handleError() const {
    const QString error = lua_tostring(L, -1);
    int line = -1;
    static const QRegularExpression re(R"(\]:(\d+):)");
    if (const auto match = re.match(error); match.hasMatch()) line = match.captured(1).toInt();
    QMetaObject::invokeMethod(g_script, [this, line, error] {
        g_script->markerHighlight(m_scriptUrl, line);
        g_script->appendLog(error, "error");
    }, Qt::QueuedConnection);
    lua_pop(L, 1);
}

// ScriptExplorer public
ScriptExplorer::ScriptExplorer(QWidget *parent)
    : QTreeView(parent),
      m_model(new QFileSystemModel()) {
    this->installEventFilter(this);
    connect(this, &QTreeView::doubleClicked, this, &ScriptExplorer::scriptOpen);

    this->QTreeView::setModel(m_model);
    this->setHeaderHidden(true);
    this->setColumnHidden(1, true);
    this->setColumnHidden(2, true);
    this->setColumnHidden(3, true);
    this->setColumnHidden(4, true);
    m_model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
    // open workspace
    if (const QUrl rootUrl(g_config["mainConfig"].toObject()["workspace"].toString()); !rootUrl.isEmpty()) {
        workspaceOpen(rootUrl);
    }
}

void ScriptExplorer::workspaceOpen(const QUrl &rootUrl) {
    const QString rootPath = rootUrl.toLocalFile();
    m_model->setRootPath(rootPath);
    this->QTreeView::setRootIndex(m_model->index(rootPath));
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
    const QString filePath = QDir::current().filePath(m_model->rootPath() + "/" + fileName);

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

void ScriptExplorer::scriptOpenInExplorer() const {
    const QDir folderPath = m_model->rootPath();
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
