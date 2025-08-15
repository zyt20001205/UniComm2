#include "../include/script.h"

static Script *g_script = nullptr;

Script::Script(QObject *parent)
    : QDockWidget("script", qobject_cast<QWidget *>(parent)) {
    uiInit();

    connect(this, &Script::start, this, &Script::scriptRunning);

    g_script = this;
}

void Script::uiInit() {
    m_widget = new QWidget();
    m_layout = new QVBoxLayout(m_widget);
    setWidget(m_widget);

    m_scriptWidget = new QWidget();
    m_layout->addWidget(m_scriptWidget);
    m_scriptWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_scriptLayout = new QVBoxLayout(m_scriptWidget);
    m_splitter = new QSplitter(Qt::Horizontal, m_scriptWidget);
    m_scriptLayout->addWidget(m_splitter);
    m_textEdit = new QTextEdit();
    m_splitter->addWidget(m_textEdit);
    m_listWidget = new QListWidget();
    m_splitter->addWidget(m_listWidget);
    m_listWidget->setSpacing(5);

    m_ctrlWidget = new QWidget();
    m_layout->addWidget(m_ctrlWidget);
    m_ctrlLayout = new QHBoxLayout(m_ctrlWidget);
    m_runButton = new QPushButton("run");
    m_ctrlLayout->addWidget(m_runButton);
    connect(m_runButton, &QPushButton::clicked, this, &Script::scriptRun);
    m_helpButton = new QPushButton("help");
    m_ctrlLayout->addWidget(m_helpButton);
    connect(m_helpButton, &QPushButton::clicked, this, &Script::manualDisplay);
}

void Script::manualDisplay() {
    const auto dlg = new QDialog(this);
    dlg->setWindowTitle("Manual");
    dlg->resize(900, 600);
    dlg->show();
    const auto layout = new QVBoxLayout(dlg);
    const auto view = new QTextBrowser(dlg);
    layout->addWidget(view);
    view->setOpenExternalLinks(true);
    view->setSearchPaths(QStringList() << ":/doc");
    view->setSource(QUrl("catalogue.md"));
    view->document()->setDefaultFont(QFont("Consolas", 11));
}

void Script::scriptRun() {
    const QString script = m_textEdit->toPlainText();
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
        lua_register(L, "write", Script::luaWrite);
        lua_register(L, "read", Script::luaRead);
        lua_register(L, "delay", Script::luaDelay);
        // exec lua script
        const int result = luaL_dostring(L, script.toUtf8().constData());
        if (result != LUA_OK) {
            const QString error = lua_tostring(L, -1);
            emit appendLog(QString("%1 %2").arg("script error:", error), "error");
            lua_pop(L, 1);
        }
        // close interpreter
        lua_close(L);
    });
    connect(worker, &QThread::finished, this, &Script::scriptFinished);
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    worker->start();
    emit start(worker);
}

void Script::scriptRunning(QThread *worker) {
    auto *scriptItem = new QListWidgetItem();
    m_listWidget->addItem(scriptItem);
    auto *scriptBar = new QWidget(m_listWidget);
    m_listWidget->setItemWidget(scriptItem, scriptBar);
    scriptBar->setMinimumHeight(40);
    auto *scriptLayout = new QHBoxLayout(scriptBar);
    scriptLayout->setContentsMargins(5, 0, 5, 0);
    auto *scriptLabel = new QLabel(QDateTime::currentDateTime().toString("HH:mm:ss.zzz") + " editor");
    scriptLayout->addWidget(scriptLabel);
    auto *scriptProgressBar = new QProgressBar();
    scriptLayout->addWidget(scriptProgressBar);
    scriptProgressBar->setFixedHeight(10);
    scriptProgressBar->setRange(0, 0);
    auto *abortButton = new QPushButton();
    scriptLayout->addWidget(abortButton);
    abortButton->setFixedSize(24, 24);
    abortButton->setIcon(QIcon(":/icon/stop.svg"));
    connect(abortButton, &QPushButton::clicked, this, [worker]() {
        worker->terminate();
    });
}

void Script::scriptFinished() {
    qDebug() << "script finished";
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

int Script::luaWrite(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 2)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const char *param1 = luaL_checkstring(L, 1);
    const int param2 = luaL_optinteger(L, 2, -1);
    // start operation
    emit g_script->writePort(QString::fromUtf8(param1), param2);
    return 0;
}

int Script::luaRead(lua_State *L) {
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
