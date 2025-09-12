#include "../include/script.h"

static Script *g_script = nullptr;
QList<int> g_breakpoint;

auto g_stateMachine = STATE_RUN;
int g_depth = 0;
int g_baseDepth = 0;

// Script public
Script::Script(QWidget *parent) : QWidget(parent), m_tooltipWidget(new TooltipWidget(this)) {
    g_script = this;
    // script module init
    auto *layout = new QHBoxLayout(this); // NOLINT
    auto *scriptSplitter = new QSplitter(Qt::Horizontal); // NOLINT
    layout->addWidget(scriptSplitter);
    // script widget -> script editor
    m_scriptTabWidget = new QTabWidget();
    scriptSplitter->addWidget(m_scriptTabWidget);
    connect(m_scriptTabWidget, &QTabWidget::currentChanged, this, &Script::scriptSelected);
    m_scriptTabWidget->setTabsClosable(true);
    connect(m_scriptTabWidget, &QTabWidget::tabCloseRequested, this, &Script::scriptClose);
    m_scriptTabWidget->setMovable(true);
    connect(m_scriptTabWidget->tabBar(), &QTabBar::tabMoved, this, &Script::scriptSwap);
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
            const QString scriptPath = value.toString();
            const QFileInfo fileInfo(scriptPath);
            const QString fileName = fileInfo.fileName();
            auto *newTab = new ScriptPageWidget(m_scriptConfig, scriptPath); // NOLINT
            m_scriptTabWidget->addTab(newTab, fileName);
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

    scriptSplitter->setStretchFactor(0, 4);
    scriptSplitter->setStretchFactor(1, 1);

    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "script module initialized");
}

void Script::scriptConfigSave() const {
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

void Script::scriptOpen(const QString &scriptPath) {
    // gui
    // switch to existing page if already opened
    QJsonArray scriptList = m_scriptConfig["scriptList"].toArray();
    for (int i = 0; i < scriptList.size(); i++) {
        if (scriptList[i].toString() == scriptPath) {
            m_scriptTabWidget->setCurrentIndex(i);
            return;
        }
    }
    // remove welcome page if exist
    if (m_scriptTabWidget->tabText(0) == "welcome") {
        m_scriptTabWidget->removeTab(0);
    }
    // open new tab
    const QFileInfo fileInfo(scriptPath);
    const QString fileName = fileInfo.fileName();
    auto *newTab = new ScriptPageWidget(m_scriptConfig, scriptPath); // NOLINT
    m_scriptTabWidget->addTab(newTab, fileName);
    m_scriptTabWidget->setCurrentWidget(newTab);
    connect(newTab, &ScriptPageWidget::modifyScript, this, [this, newTab] {
        scriptModify(m_scriptTabWidget->indexOf(newTab));
    });
    // config
    scriptList.append(scriptPath);
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

void Script::diagnosticsReceive(const QString &scriptPath, const QJsonArray &diagnosticsArray) {
    m_diagnosticsHash.insert(scriptPath, diagnosticsArray);
    diagnosticsPublish();
}

void Script::diagnosticsPublish() const {
    const QJsonArray &diagnosticsArray = m_diagnosticsHash[m_currentScriptPage->m_scriptPath];
    const int lastLine = m_currentScriptPage->m_scriptEditor->lines() - 1;
    const int lastIndex = m_currentScriptPage->m_scriptEditor->lineLength(lastLine);
    m_currentScriptPage->m_scriptEditor->clearIndicatorRange(0, 0, lastLine, lastIndex, INDICATOR_ERROR);
    for (const auto &diagnostic: diagnosticsArray) {
        const QJsonObject diagnosticObject = diagnostic.toObject();
        const int severity = diagnosticObject["severity"].toInt();
        const QJsonObject diagnosticRange = diagnosticObject["range"].toObject();
        const QJsonObject diagnosticStartPos = diagnosticRange["start"].toObject();
        const QJsonObject diagnosticEndPos = diagnosticRange["end"].toObject();
        if (severity == 2) {
            // error
            m_currentScriptPage->m_scriptEditor->fillIndicatorRange(diagnosticStartPos["line"].toInt(),
                                                                    diagnosticStartPos["character"].toInt(),
                                                                    diagnosticEndPos["line"].toInt(),
                                                                    diagnosticEndPos["character"].toInt(),
                                                                    INDICATOR_ERROR);
        } else if (severity == 4) {
            // warning
            m_currentScriptPage->m_scriptEditor->fillIndicatorRange(diagnosticStartPos["line"].toInt(),
                                                                    diagnosticStartPos["character"].toInt(),
                                                                    diagnosticEndPos["line"].toInt(),
                                                                    diagnosticEndPos["character"].toInt(),
                                                                    INDICATOR_WARNING);
        }
    }
}

void Script::textDocumentHover(const QString &message) const {
    m_tooltipWidget->showTooltip(message);
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
    m_scriptMonitorTabWidget->setCurrentIndex(1); // switch to debug tab
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
}

void Script::scriptSwap(const int srcIndex, const int dstIndex) {
    // config
    QJsonArray scriptList = m_scriptConfig["scriptList"].toArray();
    const QJsonValue tmp = scriptList.takeAt(srcIndex);
    scriptList.insert(dstIndex, tmp);
    m_scriptConfig["scriptList"] = scriptList;
    // qDebug() << m_scriptConfig;
}

// Tooltip public
TooltipWidget::TooltipWidget(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::Popup);
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    m_textBrowser = new QTextBrowser(this);
    layout->addWidget(m_textBrowser);
    m_textBrowser->setFixedWidth(600);
    m_textBrowser->setFont(QFont("Consolas", 10));
    m_textBrowser->setOpenExternalLinks(true);
    m_textBrowser->installEventFilter(this);
}

// Tooltip protected
bool TooltipWidget::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Leave) {
        hideTooltip();
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

// Tooltip private
void TooltipWidget::showTooltip(const QString &message) {
    m_textBrowser->setMarkdown(message);
    this->adjustSize();
    this->move(QCursor::pos() + QPoint(15, 15));
    this->show();
}

void TooltipWidget::hideTooltip() {
    this->hide();
}

// ScriptPageWidget public
ScriptPageWidget::ScriptPageWidget(const QJsonObject &scriptConfig, const QString &scriptPath, QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    m_editTimer = new QTimer(this);
    m_editTimer->setInterval(1000);
    m_editTimer->setSingleShot(true);
    connect(m_editTimer, &QTimer::timeout, [this] {
        scriptEditFinish();
    });
    m_scriptEditor = new ScriptEditor();
    layout->addWidget(m_scriptEditor);
    m_scriptEditor->m_scriptLexer->setFont(QFont(scriptConfig["fontFamily"].toString(), scriptConfig["fontSize"].toInt()), -1);
    m_scriptPath = scriptPath;
    const QDir scriptDir(QDir::current().filePath("script"));
    QFile file(scriptDir.filePath(scriptPath));
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    const QString content = in.readAll();
    file.close();
    m_scriptEditor->setText(content);
    // connect signals
    connect(m_scriptEditor, SIGNAL(modificationChanged(bool)), this, SLOT(scriptModify(bool)));
    connect(m_scriptEditor, SIGNAL(textChanged()), this, SLOT(scriptEdit()));
    connect(m_scriptEditor, SIGNAL(SCN_DWELLSTART(int,int,int)), this, SLOT(dwellStart(int,int,int)));
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, scriptPath, "opened");
    // didOpen notification to lua language server
    const QString scriptAbsolutePath = QCoreApplication::applicationDirPath() + "/script/" + m_scriptPath;
    const QString scriptUri = QUrl::fromLocalFile(scriptAbsolutePath).toString();
    QTimer::singleShot(0, this, [this, scriptUri, content] {
        const QJsonObject didOpenParams{
            {
                "textDocument", QJsonObject{
                    {"uri", scriptUri},
                    {"languageId", "lua"},
                    {"version", m_version++},
                    {"text", content}
                }
            }
        };
        emit notificationJson("textDocument/didOpen", didOpenParams);
    });
}

void ScriptPageWidget::scriptSave() {
    if (!m_scriptModify) return;
    // save file
    const QDir scriptDir(QDir::current().filePath("script"));
    QFile file(scriptDir.filePath(m_scriptPath));
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << m_scriptEditor->text();
    file.close();
    // update status
    m_scriptEditor->setModified(false);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, m_scriptPath, "saved");
}

void ScriptPageWidget::scriptEditFinish() {
    // didChange notification to lua language server
    const QString scriptAbsolutePath = QCoreApplication::applicationDirPath() + "/script/" + m_scriptPath;
    const QString scriptUri = QUrl::fromLocalFile(scriptAbsolutePath).toString();
    const QString content = m_scriptEditor->text();
    const QJsonObject didChangeParams{
        {
            "textDocument", QJsonObject{
                {"uri", scriptUri},
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
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, m_scriptPath, "edited");
}

// ScriptPageWidget private
void ScriptPageWidget::scriptModify(const bool status) {
    m_scriptModify = status;
    if (m_scriptModify) {
        emit modifyScript();
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3").arg(timestamp, m_scriptPath, "modified");
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
    // hover request to lua language server
    const QString scriptAbsolutePath = QCoreApplication::applicationDirPath() + "/script/" + m_scriptPath;
    const QString scriptUri = QUrl::fromLocalFile(scriptAbsolutePath).toString();
    const QJsonObject hoverParams{
        {
            "textDocument", QJsonObject{
                {"uri", scriptUri}
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
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, m_scriptPath, "hovered");
}

// void ScriptPageWidget::diagnosticsShow(const int pos, const int x, const int y) {
//     int line = 0, index = 0;
//     m_scriptEditor->lineIndexFromPosition(pos, &line, &index);
//     QString diagnosticHit;
//     for (const auto &diagnostic: m_diagnosticsArray) {
//         const QJsonObject diagnosticObject = diagnostic.toObject();
//         const int severity = diagnosticObject["severity"].toInt();
//         const QString code = diagnosticObject["code"].toString();
//         const QString message = diagnosticObject["message"].toString();
//         const QJsonObject diagnosticRange = diagnosticObject["range"].toObject();
//         const QJsonObject diagnosticStartPos = diagnosticRange["start"].toObject();
//         const QJsonObject diagnosticEndPos = diagnosticRange["end"].toObject();
//         if (line == diagnosticStartPos["line"].toInt() && index >= diagnosticStartPos["character"].toInt() && index <= diagnosticEndPos["character"].toInt()) {
//             if (severity == 2) {
//                 diagnosticHit = "error: " + code + "\n" + message;
//             } else if (severity == 4) {
//                 diagnosticHit = "warning: " + code + "\n" + message;
//             }
//             break;
//         }
//     }
//     m_scriptEditor->SendScintilla(QsciScintillaBase::SCI_CALLTIPSHOW, pos, diagnosticHit.toUtf8().constData()); // NOLINT
// }

// ScriptEditor public
ScriptEditor::ScriptEditor(QWidget *parent) : QsciScintilla(parent) {
    SendScintilla(SCI_SETWORDCHARS, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:."); // NOLINT
    // load lua lexer
    m_scriptLexer = new LuaLexer(); // NOLINT
    this->QsciScintilla::setLexer(m_scriptLexer);
    // configure auto complete
    auto *apis = new QsciAPIs(m_scriptLexer); // NOLINT
    apis->load(":/api/Lua-5.4.8.api");
    apis->load(":/api/Custom-1.0.0.api");
    apis->prepare();
    this->QsciScintilla::setAutoCompletionSource(AcsAPIs);
    this->QsciScintilla::setAutoCompletionCaseSensitivity(false);
    this->QsciScintilla::setAutoCompletionThreshold(1);
    this->setAutoCompletionFillupsEnabled(true);
    this->setAutoCompletionFillups(":.");
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
    // load settings from config
    m_scriptLexer->setPaper(Qt::white, -1);
    // style 0: default
    // style 1: comment
    m_scriptLexer->setColor(QColor(0x8C8C8C), 1);
    // style 2: line comment
    m_scriptLexer->setColor(QColor(0x8C8C8C), 2);
    // style 4: number
    m_scriptLexer->setColor(QColor(0x1750EB), 4);
    // style 5: keyword
    m_scriptLexer->setColor(QColor(0x0033B3), 5);
    // style 6: string
    m_scriptLexer->setColor(QColor(0x067D17), 6);
    // style 7: character
    // style 8: literal string
    // style 9: preprocessor
    // style 10: operator
    m_scriptLexer->setColor(QColor(0x2B2D30), 10);
    // style 11: identifier
    // m_scriptLexer->setColor(QColor(0x00627A), 11);
    // style 12: unclosed string
    // style 13: basic functions
    // m_scriptLexer->setColor(QColor(0x00627A), 13);
    // style 14: string, table and maths functions
    // m_scriptLexer->setColor(QColor(0x00627A), 14);
    // style 15: coroutines, i/o and system facilities
    // m_scriptLexer->setColor(QColor(0x00627A), 15);
    // style 16: user defined 1
    // m_scriptLexer->setColor(QColor(0x00627A), 16);
    // style 20: label
    // connect(this, SIGNAL(modificationChanged(bool m)), this, SLOT());
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
ScriptExplorer::ScriptExplorer(QWidget *parent) {
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

    // check if script dir exists
    if (const QDir scriptDir(scriptPath); !scriptDir.exists()) {
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "script directory generated");
        if (!scriptDir.mkpath(".")) {
            // logging
            timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] %2").arg(timestamp, "script directory generation failed");
            return;
        }
    } else {
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "script directory found");
    }

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
    // using relative file path
    const QString filePath = m_model->filePath(index);
    const QDir scriptDir(QDir::current().filePath("script"));
    emit openScript(scriptDir.relativeFilePath(filePath));
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
