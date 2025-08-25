#include "../include/script.h"

static Script *g_script = nullptr;

// Script public
Script::Script(QWidget *parent)
    : QWidget(parent) {
    // script module init
    auto *layout = new QVBoxLayout(this); // NOLINT
    m_scriptWidget = new QWidget();
    layout->addWidget(m_scriptWidget);
    m_scriptWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *scriptLayout = new QVBoxLayout(m_scriptWidget); // NOLINT
    auto *scriptSplitter = new QSplitter(Qt::Horizontal); // NOLINT
    scriptLayout->addWidget(scriptSplitter);
    m_scriptPlainTextEdit = new ScriptEditor();
    scriptSplitter->addWidget(m_scriptPlainTextEdit);
    m_scriptPlainTextEdit->setPlainText(g_config["scriptConfig"].toString());
    m_scriptPlainTextEdit->document()->setDefaultFont(QFont("Consolas", 11));
    new ScriptHighlighter(m_scriptPlainTextEdit->document());
    // script monitor widget
    auto *scriptMonitorWidget = new QWidget(); // NOLINT
    auto *scriptMonitorLayout = new QVBoxLayout(scriptMonitorWidget); // NOLINT
    scriptMonitorLayout->setContentsMargins(0, 0, 0, 0);
    auto *scriptMonitorSplitter = new QSplitter(Qt::Vertical); // NOLINT
    scriptMonitorLayout->addWidget(scriptMonitorSplitter);
    m_scriptListWidget = new QListWidget();
    scriptMonitorSplitter->addWidget(m_scriptListWidget);
    m_scriptListWidget->setStyleSheet("QListWidget::item { min-height: 40px; }");
    m_scriptExplorerTreeView = new ScriptExplorer();
    scriptMonitorSplitter->addWidget(m_scriptExplorerTreeView);
    connect(m_scriptExplorerTreeView, &ScriptExplorer::appendLog, this, &Script::appendLog);
    connect(m_scriptExplorerTreeView, &ScriptExplorer::loadScript, this, &Script::scriptLoad);
    connect(m_scriptExplorerTreeView, &ScriptExplorer::runScript, this, &Script::scriptRun);


    scriptSplitter->addWidget(scriptMonitorWidget);

    scriptSplitter->setStretchFactor(0, 3);
    scriptSplitter->setStretchFactor(1, 1);

    m_ctrlWidget = new QWidget();
    layout->addWidget(m_ctrlWidget);
    m_ctrlLayout = new QHBoxLayout(m_ctrlWidget);
    auto *runButton = new QPushButton("run"); // NOLINT
    m_ctrlLayout->addWidget(runButton);
    connect(runButton, &QPushButton::clicked, this, [this] {
        scriptRun("editor", m_scriptPlainTextEdit->toPlainText());
    });
    auto *saveButton = new QPushButton("save"); // NOLINT
    m_ctrlLayout->addWidget(saveButton);
    connect(saveButton, &QPushButton::clicked, this, &Script::scriptSave);
    auto *helpButton = new QPushButton("help"); // NOLINT
    m_ctrlLayout->addWidget(helpButton);
    connect(helpButton, &QPushButton::clicked, this, [this] {
        m_manualDialog->show();
    });

    manualUiInit();
    g_script = this;
}

void Script::scriptConfigSave() const {
    g_config["scriptConfig"] = m_scriptPlainTextEdit->toPlainText();
}

void Script::scriptLoad(const QString &scriptPath) {
    QFile file(scriptPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    const QString content = in.readAll();
    file.close();
    m_scriptPlainTextEdit->setPlainText(content);

    const QFileInfo fileInfo(scriptPath);
    QString fileName = fileInfo.fileName();
    emit appendLog(QString("%1 %2").arg(fileName, "loaded"), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, fileName, "loaded");
}

// Script private
void Script::manualUiInit() {
    m_manualDialog = new QDialog(this);
    m_manualDialog->setWindowTitle("Manual");
    m_manualDialog->resize(900, 600);
    const auto manualLayout = new QVBoxLayout(m_manualDialog); // NOLINT
    const auto manualSplitter = new QSplitter(Qt::Horizontal); // NOLINT
    manualLayout->addWidget(manualSplitter);

    const auto manualTreeView = new QTreeView(); // NOLINT
    manualSplitter->addWidget(manualTreeView);
    manualTreeView->setHeaderHidden(true);
    manualTreeView->setFont(QFont("Consolas", 12));

    const auto manualStandardItemModel = new QStandardItemModel(); // NOLINT
    manualTreeView->setModel(manualStandardItemModel);

    const auto manualPortStandardItem = new QStandardItem("port"); // NOLINT
    manualStandardItemModel->appendRow(manualPortStandardItem);
    const auto manualOpenStandardItem = new QStandardItem("open"); // NOLINT
    manualPortStandardItem->appendRow(manualOpenStandardItem);
    const auto manualCloseStandardItem = new QStandardItem("close"); // NOLINT
    manualPortStandardItem->appendRow(manualCloseStandardItem);
    const auto manualInfoStandardItem = new QStandardItem("info"); // NOLINT
    manualPortStandardItem->appendRow(manualInfoStandardItem);
    const auto manualWriteStandardItem = new QStandardItem("write"); // NOLINT
    manualPortStandardItem->appendRow(manualWriteStandardItem);
    const auto manualReadStandardItem = new QStandardItem("read"); // NOLINT
    manualPortStandardItem->appendRow(manualReadStandardItem);

    manualTreeView->expandAll();
    connect(manualTreeView, &QTreeView::clicked, [this](const QModelIndex &index) {
        if (const QString itemText = index.data(Qt::DisplayRole).toString(); itemText == "open") {
            m_manualTextBrowser->setSource(QUrl("open.md"));
        } else if (itemText == "close") {
            m_manualTextBrowser->setSource(QUrl("close.md"));
        } else if (itemText == "info") {
            m_manualTextBrowser->setSource(QUrl("info.md"));
        } else if (itemText == "write") {
            m_manualTextBrowser->setSource(QUrl("write.md"));
        } else if (itemText == "read") {
            m_manualTextBrowser->setSource(QUrl("read.md"));
        }
    });

    m_manualTextBrowser = new QTextBrowser();
    manualSplitter->addWidget(m_manualTextBrowser);
    m_manualTextBrowser->setOpenExternalLinks(true);
    m_manualTextBrowser->setSearchPaths(QStringList() << ":/doc");

    m_manualTextBrowser->document()->setDefaultFont(QFont("Consolas", 11));

    manualSplitter->setStretchFactor(0, 1);
    manualSplitter->setStretchFactor(1, 3);
}

void Script::scriptRun(const QString &name, const QString &script) {
    if (script.isEmpty()) {
        emit appendLog("script is empty", "warning");
        return;
    }
    // launch lua interpreter thread
    QThread *worker = QThread::create([this, script]() {
        // init lua interpreter
        lua_State *L = luaL_newstate();
        luaL_openlibs(L);
        // register C++ functions
        lua_register(L, "print", Script::luaPrint);
        lua_register(L, "delay", Script::luaDelay);
        lua_register(L, "input", Script::luaInput);
        // register port class
        lua_newtable(L);
        lua_pushcfunction(L, Script::luaPortOpen);
        lua_setfield(L, -2, "open");
        lua_pushcfunction(L, Script::luaPortClose);
        lua_setfield(L, -2, "close");
        lua_pushcfunction(L, Script::luaPortInfo);
        lua_setfield(L, -2, "info");
        lua_pushcfunction(L, Script::luaPortWrite);
        lua_setfield(L, -2, "write");
        lua_pushcfunction(L, Script::luaPortRead);
        lua_setfield(L, -2, "read");
        lua_setglobal(L, "port");
        // register database class
        lua_newtable(L);
        lua_pushcfunction(L, Script::luaDatabaseWrite);
        lua_setfield(L, -2, "write");
        // lua_pushcfunction(L, Script::luaDatabaseRead);
        // lua_setfield(L, -2, "read");
        lua_setglobal(L, "database");
        // exec lua script
        if (const int result = luaL_dostring(L, script.toUtf8().constData()); result != LUA_OK) {
            const QString error = lua_tostring(L, -1);
            emit appendLog(QString("%1").arg(error), "error");
            lua_pop(L, 1);
        }
        // close interpreter
        lua_close(L);
    });
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    scriptRunning(name, worker);
    worker->start();
}

void Script::scriptRunning(const QString &name, QThread *worker) {
    auto *scriptListWidgetItem = new QListWidgetItem(); // NOLINT
    m_scriptListWidget->addItem(scriptListWidgetItem);

    connect(worker, &QThread::finished, this, [this,scriptListWidgetItem] {
        const int row = m_scriptListWidget->row(scriptListWidgetItem);
        m_scriptListWidget->takeItem(row);
        delete scriptListWidgetItem;
    });

    auto *scriptInfoWidget = new QWidget(); // NOLINT
    m_scriptListWidget->setItemWidget(scriptListWidgetItem, scriptInfoWidget);
    auto *scriptInfoLayout = new QHBoxLayout(scriptInfoWidget); // NOLINT
    scriptInfoLayout->setContentsMargins(5, 0, 5, 0);
    auto *scriptLabel = new QLabel(QDateTime::currentDateTime().toString("HH:mm:ss") + " " + name); // NOLINT
    scriptInfoLayout->addWidget(scriptLabel);
    auto *abortButton = new QPushButton(); // NOLINT
    scriptInfoLayout->addWidget(abortButton);
    abortButton->setFixedSize(24, 24);
    abortButton->setIcon(QIcon(":/icon/stop.svg"));
    connect(abortButton, &QPushButton::clicked, this, [worker]() {
        worker->terminate();
    });
}

void Script::scriptSave() {
    bool ok;
    QString fileName = QInputDialog::getText(nullptr, "Save Script", "script name:", QLineEdit::Normal, QString(), &ok);
    if (!ok || fileName.isEmpty()) {
        return;
    }
    fileName += ".lua";
    const QString filePath = QDir::current().filePath("script/" + fileName);

    if (QFile::exists(filePath)) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(nullptr, tr("File Exists"), tr("File already exists. Overwrite?"), QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) {
            return;
        }
    }

    QFile file(filePath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << m_scriptPlainTextEdit->toPlainText();
    file.close();

    emit appendLog(QString("%1 %2").arg(fileName, "saved"), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, fileName, "saved");
}

int Script::luaPrint(lua_State *L) {
    const int n = lua_gettop(L);
    QString message;
    for (int i = 1; i <= n; i++) {
        if (const char *str = lua_tostring(L, i)) {
            if (i > 1) message += " ";
            message += QString::fromUtf8(str);
        }
    }
    if (g_script && !message.isEmpty()) {
        emit g_script->appendLog(message, "info");
    }
    return 0;
}

int Script::luaDelay(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 1)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const int param = luaL_checkinteger(L, 1);
    // start operation
    QThread::msleep(param);
    return 0;
}

int Script::luaInput(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 0)
        luaL_error(L, "unexpected number of arguments");
    // start operation
    bool ok = false;
    QString input;
    QMetaObject::invokeMethod(qApp, [&] {
        QWidget *parent = QApplication::activeWindow();
        input = QInputDialog::getText(parent, "Input Dialog", "input:", QLineEdit::Normal, QString(), &ok);
    }, Qt::BlockingQueuedConnection);
    if (!ok)
        return 0;
    lua_pushstring(L, input.toUtf8().constData());
    return 1;
}

int Script::luaPortOpen(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 1)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const int param = luaL_optinteger(L, 1, -1);
    // start operation
    emit g_script->openPort(param);
    return 0;
}

int Script::luaPortClose(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 1)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const int param = luaL_optinteger(L, 1, -1);
    // start operation
    emit g_script->closePort(param);
    return 0;
}

int Script::luaPortInfo(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 1)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const int param = luaL_optinteger(L, 1, -1);
    // start operation
    QString info;
    QMetaObject::invokeMethod(g_script->m_port, [&, param]() {
        info = g_script->m_port->portInfo(param);
    }, Qt::BlockingQueuedConnection);
    emit g_script->appendLog(info, "info");
    return 0;
}

int Script::luaPortWrite(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 3)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    int param1;
    const char *param2;
    const char *param3;
    if (lua_isinteger(L, 1)) {
        param1 = luaL_checkinteger(L, 1);
        param2 = luaL_checkstring(L, 2);
        if (lua_isnoneornil(L, 3)) {
            param3 = "";
        } else {
            param3 = luaL_checkstring(L, 3);
        }
    } else {
        param1 = -1;
        param2 = luaL_checkstring(L, 1);
        if (lua_isnoneornil(L, 2)) {
            param3 = "";
        } else {
            param3 = luaL_checkstring(L, 2);
        }
    }
    // start operation
    emit g_script->writePort(param1, QString::fromUtf8(param2), QString::fromUtf8(param3));
    return 0;
}

int Script::luaPortRead(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 1)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const int param = luaL_optinteger(L, 1, -1);
    // start operation
    QString data;
    QMetaObject::invokeMethod(g_script->m_port, [&, param]() {
        data = g_script->m_port->portRead(param);
    }, Qt::BlockingQueuedConnection);
    lua_pushstring(L, data.toUtf8().constData());
    return 1;
}

int Script::luaDatabaseWrite(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 2)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const char *param1 = luaL_checkstring(L, 1);
    const char *param2 = luaL_checkstring(L, 2);
    // start operation
    emit g_script->writeDatabase(param1, param2);
    return 0;
}

// ScriptEditor public
ScriptEditor::ScriptEditor(QWidget *parent) {
}

// ScriptEditor public
ScriptHighlighter::ScriptHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
}

// ScriptEditor protected
void ScriptHighlighter::highlightBlock(const QString &text) {
    QTextCharFormat format;
    QStringList words;
    // functions #008683
    {
        // basic
        format.setForeground(QColor("#008683"));
        words = {
            "_G", "_VERSION", "assert", "collectgarbage", "dofile", "error", "getmetatable", "ipairs", "load", "loadfile", "next", "pairs", "pcall", "rawequal", "rawget",
            "rawlen", "rawset", "require", "select", "setmetatable", "tonumber", "tostring", "type", "warn", "xpcall"
        };
        foreach(const QString &word, words) {
            QRegularExpression expression("\\b" + word + "\\b");
            QRegularExpressionMatchIterator it = expression.globalMatch(text);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                setFormat(match.capturedStart(), match.capturedLength(), format);
            }
        }
        // custom
        words = {"delay", "input", "print"};
        foreach(const QString &word, words) {
            QRegularExpression expression("\\b" + word + "\\b");
            QRegularExpressionMatchIterator it = expression.globalMatch(text);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                setFormat(match.capturedStart(), match.capturedLength(), format);
            }
        }
        words = {"close", "info", "open", "read", "write"};
        foreach(const QString &word, words) {
            QRegularExpression expression("\\bport\\." + word + "\\b");
            QRegularExpressionMatchIterator it = expression.globalMatch(text);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                setFormat(match.capturedStart(), match.capturedLength(), format);
            }
        }
        words = {"write"};
        foreach(const QString &word, words) {
            QRegularExpression expression("\\bdatabase\\." + word + "\\b");
            QRegularExpressionMatchIterator it = expression.globalMatch(text);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                setFormat(match.capturedStart(), match.capturedLength(), format);
            }
        }
        // coroutine
        words = {"close", "create", "isyieldable", "resume", "running", "status", "wrap", "yield"};
        foreach(const QString &word, words) {
            QRegularExpression expression("\\bcoroutine\\." + word + "\\b");
            QRegularExpressionMatchIterator it = expression.globalMatch(text);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                setFormat(match.capturedStart(), match.capturedLength(), format);
            }
        }
        // debug
        // io
        // math
        // os
        // package
        // string
        words = {"byte", "char", "dump", "find", "format", "gmatch", "gsub", "len", "lower", "match", "pack", "packsize", "rep", "reverse", "sub", "unpack", "upper"};
        foreach(const QString &word, words) {
            QRegularExpression expression("\\bstring\\." + word + "\\b");
            QRegularExpressionMatchIterator it = expression.globalMatch(text);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                setFormat(match.capturedStart(), match.capturedLength(), format);
            }
        }
        // table
        // utf8
    }
    // keywords #A71D5D
    {
        format.setForeground(QColor("#A71D5D"));
        words = {
            "and", "break", "do", "else", "elseif", "end", "false", "for", "function", "goto", "if", "in", "local", "nil", "not", "or", "repeat", "return", "then", "true", "until",
            "while"
        };
        foreach(const QString &word, words) {
            QRegularExpression expression("\\b" + word + "\\b");
            QRegularExpressionMatchIterator it = expression.globalMatch(text);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                setFormat(match.capturedStart(), match.capturedLength(), format);
            }
        }
    }
    // string
    format.setForeground(QColor("#183691"));
    QRegularExpression expression(R"xx("([^"\\]|\\.)*")xx");
    QRegularExpressionMatchIterator it = expression.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        setFormat(match.capturedStart(), match.capturedLength(), format);
    }
    // comment
    format.setForeground(QColor("#969896"));
    expression = QRegularExpression(R"(--[^\n]*)");
    it = expression.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        setFormat(match.capturedStart(), match.capturedLength(), format);
    }
}

// ScriptExplorer public
ScriptExplorer::ScriptExplorer(QWidget *parent) {
    this->installEventFilter(this);
    connect(this, &QTreeView::doubleClicked, this, &ScriptExplorer::scriptLoad);

    m_model = new QFileSystemModel();
    this->setModel(m_model);
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
    this->setRootIndex(m_model->index(scriptPath));
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "script directory loaded");
}

// ScriptExplorer protected
void ScriptExplorer::contextMenuEvent(QContextMenuEvent *event) {
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid()) return;
    QMenu menu(this);
    menu.addAction(tr("run"), [this, index] {
        scriptRun(index);
    });
    menu.addAction(tr("load"), [this, index] {
        scriptLoad(index);
    });
    menu.addAction(tr("delete"), [this, index] {
        scriptDelete(index);
    });
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
    return true;
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

void ScriptExplorer::scriptLoad(const QModelIndex &index) {
    const QString filePath = m_model->filePath(index);
    emit loadScript(filePath);
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
