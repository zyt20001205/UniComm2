#include "../include/script.h"

static Script *g_script = nullptr;

// Script public
Script::Script(QWidget *parent) : QWidget(parent) {
    g_script = this;
    // script module init
    auto *layout = new QHBoxLayout(this); // NOLINT
    auto *scriptSplitter = new QSplitter(Qt::Horizontal); // NOLINT
    layout->addWidget(scriptSplitter);
    // script widget -> script editor
    m_scriptTabWidget = new QTabWidget();
    scriptSplitter->addWidget(m_scriptTabWidget);
    m_scriptTabWidget->setTabsClosable(true);
    connect(m_scriptTabWidget, &QTabWidget::tabCloseRequested, this, &Script::scriptClose);
    auto welcomePage = new QWidget(); // NOLINT
    auto welcomeLayout = new QVBoxLayout(welcomePage); // NOLINT
    auto welcomeLabel = new QLabel("welcome"); // NOLINT
    welcomeLayout->addWidget(welcomeLabel);
    m_scriptTabWidget->addTab(welcomePage, "welcome");

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

    // script monitor widget
    auto *scriptMonitorWidget = new QWidget(); // NOLINT
    scriptSplitter->addWidget(scriptMonitorWidget);
    auto *scriptMonitorLayout = new QVBoxLayout(scriptMonitorWidget); // NOLINT
    scriptMonitorLayout->setContentsMargins(0, 0, 0, 0);
    auto *scriptMonitorSplitter = new QSplitter(Qt::Vertical); // NOLINT
    scriptMonitorLayout->addWidget(scriptMonitorSplitter);
    // script monitor widget -> script list widget
    m_scriptListWidget = new QListWidget();
    scriptMonitorSplitter->addWidget(m_scriptListWidget);
    m_scriptListWidget->setStyleSheet("QListWidget::item { min-height: 40px; }");
    // script monitor widget -> script explorer treeview
    m_scriptExplorerTreeView = new ScriptExplorer();
    scriptMonitorSplitter->addWidget(m_scriptExplorerTreeView);
    connect(m_scriptExplorerTreeView, &ScriptExplorer::appendLog, this, &Script::appendLog);
    connect(m_scriptExplorerTreeView, &ScriptExplorer::openScript, this, &Script::scriptOpen);
    connect(m_scriptExplorerTreeView, &ScriptExplorer::runScript, this, &Script::scriptRun);

    scriptSplitter->setStretchFactor(0, 10);
    scriptSplitter->setStretchFactor(1, 1);
}

void Script::scriptConfigSave() const {
    for (int i = 0; i < m_scriptTabWidget->count(); ++i) {
        auto scriptPageWidget = qobject_cast<ScriptPageWidget *>(m_scriptTabWidget->widget(i));
        if (scriptPageWidget && scriptPageWidget->m_scriptEdited) {
            scriptPageWidget->m_scriptEdited = false;
            scriptPageWidget->scriptSave();
            QString tabName = m_scriptTabWidget->tabText(i);
            tabName.chop(1);
            m_scriptTabWidget->setTabText(i, tabName);
        }
    }

    g_config["scriptConfig"] = m_scriptConfig;
}

void Script::scriptOpen(const QString &scriptPath) {
    const QFileInfo fileInfo(scriptPath);
    QString fileName = fileInfo.fileName();

    auto *newTab = new ScriptPageWidget(m_scriptConfig, scriptPath); // NOLINT
    m_scriptTabWidget->addTab(newTab, fileName);
    connect(newTab, &ScriptPageWidget::editScript, this, [this,newTab] {
        scriptEdited(m_scriptTabWidget->indexOf(newTab));
    });

    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, scriptPath, "opened");
}

// Script private
void Script::scriptRun() {
    const int currentIndex = m_scriptTabWidget->currentIndex();
    if (currentIndex < 0) {
        return;
    }
    const QString name = m_scriptTabWidget->tabText(currentIndex);
    auto scriptPageWidget = qobject_cast<ScriptPageWidget *>(m_scriptTabWidget->widget(currentIndex));
    if (!scriptPageWidget) return;
    QString script = scriptPageWidget->m_scriptEditor->text();
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
        lua_register(L, "sleep", Script::luaSleep);
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

void Script::scriptEdited(const int index) const {
    const QString tabName = m_scriptTabWidget->tabText(index) + "*";
    m_scriptTabWidget->setTabText(index, tabName);
}

void Script::scriptClose(const int index) const {
    auto scriptPageWidget = qobject_cast<ScriptPageWidget *>(m_scriptTabWidget->widget(index));
    if (scriptPageWidget && scriptPageWidget->m_scriptEdited) {
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

int Script::luaSleep(lua_State *L) {
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

// ScriptPageWidget public
ScriptPageWidget::ScriptPageWidget(const QJsonObject &scriptConfig, const QString &scriptPath, QObject *parent) {
    auto *layout = new QVBoxLayout(this); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    m_scriptEditor = new ScriptEditor();
    layout->addWidget(m_scriptEditor);
    m_scriptEditor->m_scriptLexer->setFont(QFont(scriptConfig["fontFamily"].toString(), scriptConfig["fontSize"].toInt()), -1);
    m_scriptPath = scriptPath;
    QFile file(scriptPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    const QString content = in.readAll();
    file.close();
    m_scriptEditor->setText(content);
    // import!!! must use old connect method!!! do not modify!!!
    connect(m_scriptEditor, SIGNAL(textChanged()), this, SLOT(scriptEdited()));
}

void ScriptPageWidget::scriptSave() {
    QFile file(m_scriptPath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << m_scriptEditor->text();
    file.close();

    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, m_scriptPath, "saved");
}

// ScriptPageWidget private
void ScriptPageWidget::scriptEdited() {
    if (!m_scriptEdited) {
        emit editScript();
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3").arg(timestamp, m_scriptPath, "edited");
    }
    m_scriptEdited = true;
}

// ScriptEditor public
ScriptEditor::ScriptEditor(QWidget *parent) {
    // load lua lexer
    m_scriptLexer = new LuaLexer(); // NOLINT
    this->QsciScintilla::setLexer(m_scriptLexer);
    // configure auto complete
    auto *apis = new QsciAPIs(m_scriptLexer); // NOLINT
    const QStringList completeList = {
        // custom
        "sleep", "input", "print",
        "port.close", "port.info", "port.open", "port.read", "port.write",
        "database.write",
        // keywords
        "and", "break", "do", "else", "elseif", "end", "false", "for", "function", "goto", "if", "in", "local", "nil", "not", "or", "repeat", "return", "then", "true", "until",
        "while",
        // basic
        "_G", "_VERSION", "assert", "collectgarbage", "dofile", "error", "getmetatable", "ipairs", "load", "loadfile", "next", "pairs", "pcall", "rawequal", "rawget", "rawlen",
        "rawset", "require", "select", "setmetatable", "tonumber", "tostring", "type", "warn", "xpcall",
        // coroutine
        "coroutine.close", "coroutine.create", "coroutine.isyieldable", "coroutine.resume", "coroutine.running", "coroutine.status", "coroutine.wrap", "coroutine.yield",
        // debug
        "debug.debug", "debug.gethook", "debug.getinfo", "debug.getlocal", "debug.getmetatable", "debug.getregistry", "debug.getupvalue", "debug.getuservalue", "debug.sethook",
        "debug.setlocal", "debug.setmetatable", "debug.setupvalue", "debug.setuservalue", "debug.traceback", "debug.upvalueid", "debug.upvaluejoin",
        // io
        "io.close", "io.flush", "io.input", "io.lines", "io.open", "io.output", "io.popen", "io.read", "io.stderr", "io.stdin", "io.stdout", "io.tmpfile", "io.type", "io.write",
        "file:close", "file:flush", "file:lines", "file:read", "file:seek", "file:setvbuf", "file:write",
        // math
        "math.abs", "math.acos", "math.asin", "math.atan", "math.ceil", "math.cos", "math.deg", "math.exp", "math.floor", "math.fmod", "math.huge", "math.log", "math.max",
        "math.maxinteger", "math.min", "math.mininteger", "math.modf", "math.pi", "math.rad", "math.random", "math.randomseed", "math.sin", "math.sqrt", "math.tan",
        "math.tointeger", "math.type", "math.ult",
        // os
        "os.clock", "os.date", "os.difftime", "os.execute", "os.exit", "os.getenv", "os.remove", "os.rename", "os.setlocale", "os.time", "os.tmpname",
        // package
        "package.config", "package.cpath", "package.loaded", "package.loadlib", "package.path", "package.preload", "package.searchers", "package.searchpath",
        // string
        "string.byte", "string.char", "string.dump", "string.find", "string.format", "string.gmatch", "string.gsub", "string.len", "string.lower", "string.match", "string.pack",
        "string.packsize", "string.rep", "string.reverse", "string.sub", "string.unpack", "string.upper",
        // table
        "table.concat", "table.insert", "table.move", "table.pack", "table.remove", "table.sort", "table.unpack",
        // utf-8
        "utf8.char", "utf8.charpattern", "utf8.codepoint", "utf8.codes", "utf8.len", "utf8.offset",
    };
    for (const QString &kw: completeList) apis->add(kw);
    apis->prepare();
    this->QsciScintilla::setAutoCompletionSource(QsciScintilla::AcsAPIs);
    this->QsciScintilla::setAutoCompletionCaseSensitivity(false);
    this->QsciScintilla::setAutoCompletionThreshold(1);
    // set margins
    this->setMarginType(0, QsciScintilla::NumberMargin);
    this->QsciScintilla::setMarginWidth(0, "000");
    // this->setMarginType(1, QsciScintilla::SymbolMargin);
    // this->setMarginSensitivity(1, true);
    // this->setMarginWidth(1, "16");
    this->QsciScintilla::setMarginWidth(1, "0"); // WIP
    this->QsciScintilla::setFolding(QsciScintilla::BoxedTreeFoldStyle);
    this->setMarginType(2, QsciScintilla::SymbolMargin);
    this->QsciScintilla::setMarginSensitivity(2, true);
    this->QsciScintilla::setMarginWidth(2, "16");
    // script scintilla settings
    this->setScrollWidth(1);
    this->QsciScintilla::setBraceMatching(QsciScintilla::SloppyBraceMatch);
    this->QsciScintilla::setAutoIndent(true);
    this->QsciScintilla::setIndentationGuides(true);
    this->QsciScintilla::setTabWidth(4);
    // load settings from config
    m_scriptLexer->setPaper(Qt::white, -1);
    // style 0: default
    // style 1: comment
    m_scriptLexer->setColor(QColor(0x969896), 1);
    // style 2: line comment
    m_scriptLexer->setColor(QColor(0x969896), 2);
    // style 4: number
    m_scriptLexer->setColor(QColor(0x0086B3), 4);
    // style 5: keyword
    m_scriptLexer->setColor(QColor(0xA71D5D), 5);
    // style 6: string
    m_scriptLexer->setColor(QColor(0x183691), 6);
    // style 7: character
    // style 8: literal string
    // style 9: preprocessor
    // style 10: operator
    m_scriptLexer->setColor(QColor(0xA71D5D), 10);
    // style 11: identifier
    m_scriptLexer->setColor(QColor(0x0086B3), 11);
    // style 12: unclosed string
    // style 13: basic functions
    m_scriptLexer->setColor(QColor(0x0086B3), 13);
    // style 14: string, table and maths functions
    m_scriptLexer->setColor(QColor(0x0086B3), 14);
    // style 15: coroutines, i/o and system facilities
    m_scriptLexer->setColor(QColor(0x0086B3), 15);
    // style 16: user defined 1

    // style 20: label
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
        menu.addAction(tr("load"), [this, index] {
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

void ScriptExplorer::scriptOpen(const QModelIndex &index) {
    const QString filePath = m_model->filePath(index);
    emit openScript(filePath);
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