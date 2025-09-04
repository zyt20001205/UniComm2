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
    auto *welcomePage = new QWidget(); // NOLINT
    m_scriptTabWidget->addTab(welcomePage, "welcome");
    auto *welcomeLayout = new QVBoxLayout(welcomePage); // NOLINT
    auto *welcomeBrowser = new QTextBrowser(); // NOLINT
    welcomeLayout->addWidget(welcomeBrowser);

    // welcomeBrowser->document()->setDefaultFont(QFont("Consolas", 11));

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

    scriptSplitter->setStretchFactor(0, 4);
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
    const QString fileName = fileInfo.fileName();

    auto *newTab = new ScriptPageWidget(m_scriptConfig, scriptPath); // NOLINT
    m_scriptTabWidget->addTab(newTab, fileName);
    m_scriptTabWidget->setCurrentWidget(newTab);
    connect(newTab, &ScriptPageWidget::showManual, this, &Script::showManual);
    connect(newTab, &ScriptPageWidget::editScript, this, [this, newTab] {
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
    const auto scriptPageWidget = qobject_cast<ScriptPageWidget *>(m_scriptTabWidget->widget(currentIndex));
    if (!scriptPageWidget) return;
    QString script = scriptPageWidget->m_scriptEditor->text();
    if (script.isEmpty()) {
        emit appendLog("script is empty", "warning");
        return;
    }
    // launch lua interpreter thread
    const auto worker = new QThread(); // NOLINT
    const auto interpreter = new LuaInterpreter(); // NOLINT
    interpreter->moveToThread(worker);
    connect(interpreter, &LuaInterpreter::appendLog, this, &Script::appendLog);
    connect(worker, &QThread::finished, interpreter, &LuaInterpreter::deleteLater);
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QThread::started, [interpreter, script] {
        interpreter->exec(script);
        QThread::currentThread()->quit();
    });
    scriptRunning(name, worker);
    worker->start();
}

void Script::scriptRunning(const QString &name, QThread *worker) {
    auto *scriptListWidgetItem = new QListWidgetItem(); // NOLINT
    m_scriptListWidget->addItem(scriptListWidgetItem);
    connect(worker, &QThread::finished, this, [this, scriptListWidgetItem] {
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
    connect(abortButton, &QPushButton::clicked, this, [worker] {
        worker->requestInterruption();
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

// LuaInterpreter public
LuaInterpreter::LuaInterpreter(QObject *parent) {
    // init lua interpreter
    L = luaL_newstate();
    luaL_openlibs(L);
    // register C++ functions
    lua_register(L, "print", LuaInterpreter::luaPrint);
    lua_register(L, "sleep", LuaInterpreter::luaSleep);
    lua_register(L, "input", LuaInterpreter::luaInput);
    // register port class
    lua_newtable(L);
    lua_pushcfunction(L, LuaInterpreter::luaPortOpen);
    lua_setfield(L, -2, "open");
    lua_pushcfunction(L, LuaInterpreter::luaPortClose);
    lua_setfield(L, -2, "close");
    lua_pushcfunction(L, LuaInterpreter::luaPortInfo);
    lua_setfield(L, -2, "info");
    lua_pushcfunction(L, LuaInterpreter::luaPortWriteText);
    lua_setfield(L, -2, "writeText");
    lua_pushcfunction(L, LuaInterpreter::luaPortWriteData);
    lua_setfield(L, -2, "writeData");
    lua_pushcfunction(L, LuaInterpreter::luaPortReadText);
    lua_setfield(L, -2, "readText");
    lua_pushcfunction(L, LuaInterpreter::luaPortReadData);
    lua_setfield(L, -2, "readData");
    lua_setglobal(L, "port");
    // register modbus rtu class
    lua_newtable(L);
    lua_pushcfunction(L, LuaInterpreter::luaModbusRtuReadHoldingRegisters);
    lua_setfield(L, -2, "readHoldingRegisters");
    lua_pushcfunction(L, LuaInterpreter::luaModbusRtuWriteMultipleRegisters);
    lua_setfield(L, -2, "writeMultipleRegisters");
    lua_setglobal(L, "modbusRtu");
    // register modbus ascii class
    lua_newtable(L);
    lua_pushcfunction(L, LuaInterpreter::luaModbusAsciiReadHoldingRegisters);
    lua_setfield(L, -2, "readHoldingRegisters");
    lua_setglobal(L, "modbusAscii");
    // register database class
    lua_newtable(L);
    lua_pushcfunction(L, LuaInterpreter::luaDatabaseWrite);
    lua_setfield(L, -2, "write");
    lua_setglobal(L, "database");
    // register datatable class
    lua_newtable(L);
    lua_pushcfunction(L, LuaInterpreter::luaDatatableWrite);
    lua_setfield(L, -2, "write");
    lua_setglobal(L, "datatable");
    // set terminate hook
    lua_sethook(L, luaHook, LUA_MASKCOUNT, 100);
}

void LuaInterpreter::exec(const QString &script) {
    if (const int result = luaL_dostring(L, script.toUtf8().constData()); result != LUA_OK) {
        const QString error = lua_tostring(L, -1);
        emit appendLog(QString("%1").arg(error), "error");
        lua_pop(L, 1);
    }
    // close interpreter
    lua_close(L);
}

// LuaInterpreter private
void LuaInterpreter::luaHook(lua_State *L, lua_Debug *ar) {
    (void) ar;
    // check if thread interruption is requested
    if (QThread::currentThread()->isInterruptionRequested()) {
        luaL_error(L, "terminated");
    }
}

int LuaInterpreter::luaPrint(lua_State *L) {
    const int n = lua_gettop(L);
    QString message;
    for (int i = 1; i <= n; i++) {
        size_t len = 0;
        const char *s = luaL_tolstring(L, i, &len);
        if (i > 1) message += " ";
        if (s) message += QString::fromUtf8(s, static_cast<int>(len));
        lua_pop(L, 1);
    }
    if (g_script && !message.isEmpty()) emit g_script->appendLog(message, "info");
    return 0;
}

int LuaInterpreter::luaSleep(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 1)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const int param = static_cast<int>(luaL_checkinteger(L, 1));
    // start operation
    QThread::msleep(param);
    return 0;
}

int LuaInterpreter::luaInput(lua_State *L) {
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

int LuaInterpreter::luaPortOpen(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 1)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const int param1 = static_cast<int>(luaL_optinteger(L, 1, -1));
    // start operation
    const int index = param1;
    auto *portObject = g_script->m_port->portObject(index);
    bool status;
    QMetaObject::invokeMethod(portObject, [&] {
        status = portObject->open();
    }, Qt::BlockingQueuedConnection);
    lua_pushboolean(L, status);
    return 1;
}

int LuaInterpreter::luaPortClose(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 1)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const int param1 = static_cast<int>(luaL_optinteger(L, 1, -1));
    // start operation
    const int index = param1;
    auto *portObject = g_script->m_port->portObject(index);
    QMetaObject::invokeMethod(portObject, [&] {
        portObject->close();
    }, Qt::BlockingQueuedConnection);
    return 0;
}

int LuaInterpreter::luaPortInfo(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 1)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const int param1 = static_cast<int>(luaL_optinteger(L, 1, -1));
    // start operation
    const int index = param1;
    auto *portObject = g_script->m_port->portObject(index);
    QString info;
    QMetaObject::invokeMethod(portObject, [&] {
        info = portObject->info();
    }, Qt::BlockingQueuedConnection);
    emit g_script->appendLog(info, "info");
    return 0;
}

int LuaInterpreter::luaPortWriteText(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 3)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    int param1;
    const char *param2;
    const char *param3 = nullptr;
    if (lua_isinteger(L, 1)) {
        param1 = static_cast<int>(luaL_checkinteger(L, 1));
        param2 = luaL_checkstring(L, 2);
        if (!lua_isnoneornil(L, 3)) param3 = luaL_checkstring(L, 3);
    } else {
        param1 = -1;
        param2 = luaL_checkstring(L, 1);
        if (!lua_isnoneornil(L, 2)) param3 = luaL_checkstring(L, 2);
    }
    // start operation
    const int index = param1;
    const QString txText = QString::fromUtf8(param2);
    auto *portObject = g_script->m_port->portObject(index);
    if (param3) {
        const QString peerIp = QString::fromUtf8(param3);
        QMetaObject::invokeMethod(portObject, [&, txText, peerIp] {
            portObject->writeText(txText, peerIp);
        }, Qt::BlockingQueuedConnection);
    } else {
        QMetaObject::invokeMethod(portObject, [&, txText] {
            portObject->writeText(txText);
        }, Qt::BlockingQueuedConnection);
    }
    return 0;
}

int LuaInterpreter::luaPortWriteData(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 3)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    int param1;
    const char *param2;
    size_t len2;
    const char *param3 = nullptr;
    if (lua_isinteger(L, 1)) {
        param1 = static_cast<int>(luaL_checkinteger(L, 1));
        param2 = luaL_checklstring(L, 2, &len2);
        if (!lua_isnoneornil(L, 3)) param3 = luaL_checkstring(L, 3);
    } else {
        param1 = -1;
        param2 = luaL_checklstring(L, 1, &len2);
        if (!lua_isnoneornil(L, 2)) param3 = luaL_checkstring(L, 2);
    }
    // start operation
    const int index = param1;
    const QByteArray txData(param2, static_cast<qsizetype>(len2));
    auto *portObject = g_script->m_port->portObject(index);
    if (param3) {
        const QString peerIp = QString::fromUtf8(param3);
        QMetaObject::invokeMethod(portObject, [&, txData, peerIp] {
            portObject->writeData(txData, peerIp);
        }, Qt::BlockingQueuedConnection);
    } else {
        QMetaObject::invokeMethod(portObject, [&, txData] {
            portObject->writeData(txData);
        }, Qt::BlockingQueuedConnection);
    }
    return 0;
}

int LuaInterpreter::luaPortReadText(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 2)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const int param1 = static_cast<int>(luaL_optinteger(L, 1, -1));
    const int param2 = static_cast<int>(luaL_optinteger(L, 2, 0));
    // start operation
    const int index = param1;
    const int timeout = param2;
    QString rxText;
    auto *portObject = g_script->m_port->portObject(index);
    QMetaObject::invokeMethod(portObject, [&, timeout] {
        rxText = portObject->readText(timeout);
    }, Qt::BlockingQueuedConnection);
    lua_pushstring(L, rxText.toUtf8().constData());
    return 1;
}

int LuaInterpreter::luaPortReadData(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 2)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const int param1 = static_cast<int>(luaL_optinteger(L, 1, -1));
    const int param2 = static_cast<int>(luaL_optinteger(L, 2, 0));
    // start operation
    const int index = param1;
    const int timeout = param2;
    QByteArray rxData;
    auto *portObject = g_script->m_port->portObject(index);
    QMetaObject::invokeMethod(portObject, [&, timeout] {
        rxData = portObject->readData(timeout);
    }, Qt::BlockingQueuedConnection);
    lua_pushlstring(L, rxData.constData(), rxData.size());
    return 1;
}

int LuaInterpreter::luaModbusRtuReadHoldingRegisters(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 5)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const int param1 = static_cast<int>(luaL_checkinteger(L, 1));
    const int param2 = static_cast<int>(luaL_checkinteger(L, 2));
    const int param3 = static_cast<int>(luaL_checkinteger(L, 3));
    const int param4 = static_cast<int>(luaL_optinteger(L, 4, 1000));
    const int param5 = static_cast<int>(luaL_optinteger(L, 5, -1));
    // start operation
    auto *portObject = g_script->m_port->portObject(param5);
    const int txSlaveAddr = param1;
    constexpr int txFuncCode = 0x03;
    const int txStartAddr = param2;
    const int txQuantity = param3;
    QByteArray txData;
    txData.append(txSlaveAddr);
    txData.append(txFuncCode);
    txData.append(static_cast<char>(txStartAddr >> 8 & 0xFF));
    txData.append(static_cast<char>(txStartAddr & 0xFF));
    txData.append(static_cast<char>(txQuantity >> 8 & 0xFF));
    txData.append(static_cast<char>(txQuantity & 0xFF));
    txData += modbusCRC(txData);
    QMetaObject::invokeMethod(portObject, [&, txData] {
        portObject->writeData(txData);
    }, Qt::BlockingQueuedConnection);
    QByteArray rxData;
    const int timeout = param4;
    QMetaObject::invokeMethod(portObject, [&, timeout] {
        rxData = portObject->readData(timeout);
    }, Qt::BlockingQueuedConnection);
    if (rxData == "timeout") {
        luaL_error(L, "modbus rtu read holding registers timeout");
        return 0;
    }
    if (const int rxSlaveAddr = rxData.at(0); rxSlaveAddr != txSlaveAddr) {
        luaL_error(L, "modbus rtu read holding registers slave address inconsistent");
        return 0;
    }
    if (const int rxFuncCode = rxData.at(1); rxFuncCode != txFuncCode) {
        luaL_error(L, "modbus rtu read holding registers function code inconsistent");
        return 0;
    }
    const QByteArray rxChecksum = rxData.right(2);
    rxData.chop(2);
    if (rxChecksum != modbusCRC(rxData)) {
        luaL_error(L, "modbus rtu read holding registers checksum error");
        return 0;
    }
    const QByteArray registerData = rxData.mid(3);
    lua_pushlstring(L, registerData.constData(), registerData.size());
    return 1;
}

int LuaInterpreter::luaModbusRtuWriteMultipleRegisters(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 5)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const int param1 = static_cast<int>(luaL_checkinteger(L, 1));
    const int param2 = static_cast<int>(luaL_checkinteger(L, 2));
    size_t len3;
    const char *param3 = luaL_checklstring(L, 3, &len3);
    const int param4 = static_cast<int>(luaL_optinteger(L, 4, 1000));
    const int param5 = static_cast<int>(luaL_optinteger(L, 5, -1));
    // start operation
    auto *portObject = g_script->m_port->portObject(param5);
    const int txSlaveAddr = param1;
    constexpr int txFuncCode = 0x10;
    const int txStartAddr = param2;
    const QByteArray txRegData(param3, static_cast<qsizetype>(len3));
    const int txRegCount = static_cast<qsizetype>(len3) / 2;
    const int txByteCount = static_cast<qsizetype>(len3);
    QByteArray txData;
    txData.append(txSlaveAddr);
    txData.append(txFuncCode);
    txData.append(static_cast<char>(txStartAddr >> 8 & 0xFF));
    txData.append(static_cast<char>(txStartAddr & 0xFF));
    txData.append(static_cast<char>(txRegCount >> 8 & 0xFF));
    txData.append(static_cast<char>(txRegCount & 0xFF));
    txData.append(txByteCount);
    txData += txRegData;
    txData += modbusCRC(txData);
    QMetaObject::invokeMethod(portObject, [&, txData] {
        portObject->writeData(txData);
    }, Qt::BlockingQueuedConnection);
    QByteArray rxData;
    const int timeout = param4;
    QMetaObject::invokeMethod(portObject, [&, timeout] {
        rxData = portObject->readData(timeout);
    }, Qt::BlockingQueuedConnection);
    if (rxData == "timeout") {
        luaL_error(L, "modbus rtu write multiple registers timeout");
    }
    if (const int rxSlaveAddr = rxData.at(0); rxSlaveAddr != txSlaveAddr) {
        luaL_error(L, "modbus rtu write multiple registers slave address inconsistent");
    }
    if (const int rxFuncCode = rxData.at(1); rxFuncCode != txFuncCode) {
        luaL_error(L, "modbus rtu write multiple registers function code inconsistent");
    }
    if (const int rxStartAddr = rxData.at(2) << 8 | rxData.at(3); rxStartAddr != txStartAddr) {
        luaL_error(L, "modbus rtu write multiple registers start address inconsistent");
    }
    if (const int rxRegCount = rxData.at(4) << 8 | rxData.at(5); rxRegCount != txRegCount) {
        luaL_error(L, "modbus rtu write multiple registers register count inconsistent");
    }
    const QByteArray rxChecksum = rxData.right(2);
    rxData.chop(2);
    if (rxChecksum != modbusCRC(rxData)) {
        luaL_error(L, "modbus rtu write multiple registers checksum error");
    }
    return 0;
}

int LuaInterpreter::luaModbusAsciiReadHoldingRegisters(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 5)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const int param1 = static_cast<int>(luaL_checkinteger(L, 1));
    const int param2 = static_cast<int>(luaL_checkinteger(L, 2));
    const int param3 = static_cast<int>(luaL_checkinteger(L, 3));
    const int param4 = static_cast<int>(luaL_optinteger(L, 4, 1000));
    const int param5 = static_cast<int>(luaL_optinteger(L, 5, -1));
    // start operation
    auto *portObject = g_script->m_port->portObject(param5);
    const QString txSlaveAddr = QString("%1").arg(param1, 2, 10, QLatin1Char('0'));
    const QString txFuncCode = "03";
    const QString txStartAddr = QString("%1").arg(param2, 4, 10, QLatin1Char('0'));
    const QString txQuantity = QString("%1").arg(param3, 4, 10, QLatin1Char('0'));
    QString txText = ":";
    txText.append(txSlaveAddr);
    txText.append(txFuncCode);
    txText.append(txStartAddr);
    txText.append(txQuantity);
    txText += modbusLRC(txText);
    txText += "\r\n";
    QMetaObject::invokeMethod(portObject, [&, txText] {
        portObject->writeText(txText);
    }, Qt::BlockingQueuedConnection);
    QString rxText;
    const int timeout = param4;
    QMetaObject::invokeMethod(portObject, [&, timeout] {
        rxText = portObject->readText(timeout);
    }, Qt::BlockingQueuedConnection);
    if (rxText == "timeout") {
        luaL_error(L, "modbus ascii read holding registers timeout");
        return 0;
    }
    if (rxText.at(0) != ":") {
        luaL_error(L, "modbus ascii read holding registers header missing");
        return 0;
    }
    if (const QString rxSlaveAddr = rxText.mid(1, 2); rxSlaveAddr != txSlaveAddr) {
        luaL_error(L, "modbus ascii read holding registers slave address inconsistent");
        return 0;
    }
    if (const QString rxFuncCode = rxText.mid(3, 2); rxFuncCode != txFuncCode) {
        luaL_error(L, "modbus ascii read holding registers function code inconsistent");
        return 0;
    }
    rxText.chop(2);
    const QString rxChecksum = rxText.right(2);
    rxText.chop(2);
    if (rxChecksum != modbusLRC(rxText)) {
        luaL_error(L, "modbus ascii read holding registers checksum error");
        return 0;
    }
    const QString registerData = rxText.mid(7);
    lua_pushstring(L, registerData.toUtf8().constData());
    return 1;
}

int LuaInterpreter::luaDatabaseWrite(lua_State *L) {
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

int LuaInterpreter::luaDatatableWrite(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 2)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const char *param1 = luaL_checkstring(L, 1);
    const char *param2 = luaL_checkstring(L, 2);
    // start operation
    emit g_script->writeDatatable(param1, param2);
    return 0;
}

// ScriptPageWidget public
ScriptPageWidget::ScriptPageWidget(const QJsonObject &scriptConfig, const QString &scriptPath, QObject *parent) {
    auto *layout = new QVBoxLayout(this); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    m_scriptEditor = new ScriptEditor();
    layout->addWidget(m_scriptEditor);
    m_scriptEditor->m_scriptLexer->setFont(QFont(scriptConfig["fontFamily"].toString(), scriptConfig["fontSize"].toInt()), -1);
    connect(m_scriptEditor, &ScriptEditor::showManual, this, &ScriptPageWidget::showManual);
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
ScriptEditor::ScriptEditor(QWidget *parent) : QsciScintilla(parent) {
    SendScintilla(SCI_SETWORDCHARS, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:.");
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
    // set margins

    this->setMarginType(0, NumberMargin);
    this->QsciScintilla::setMarginWidth(0, "000");

    this->setMarginType(1, SymbolMargin);
    this->QsciScintilla::setMarginSensitivity(1, true);
    this->QsciScintilla::setMarginWidth(1, "16");
    this->markerDefine(Circle, 1);
    this->setMarkerBackgroundColor(Qt::red, 1);
    this->setMarkerForegroundColor(Qt::red, 1);
    connect(this, SIGNAL(marginClicked(int, int, Qt::KeyboardModifiers)),
            this, SLOT(onMarginClicked(int, int, Qt::KeyboardModifiers)));

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
}

// ScriptEditor protected
void ScriptEditor::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && event->modifiers() & Qt::ControlModifier) {
        const QString clickedWord = this->wordAtPoint(event->pos());
        emit showManual(clickedWord);
    }
    QsciScintilla::mousePressEvent(event);
}

// ScriptEditor private
void ScriptEditor::onMarginClicked(const int margin, const int line, Qt::KeyboardModifiers state) {
    if (margin == 1 && line >= 0) {
        const int mask = this->markersAtLine(line);
        if (mask & 1 << 1) {
            this->markerDelete(line, 1);
        } else {
            this->markerAdd(line, 1);
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
