#include "luaModule/luaInterpreter.h"

#include <QStandardItemModel>
#include <QTimer>
#include <sol/sol.hpp>

#include "globals.h"
#include "luaModule/luaAT.h"
#include "luaModule/luaControl.h"
#include "luaModule/luaDataProcess.h"
#include "luaModule/luaIO.h"
#include "luaModule/luaPort.h"
#include "luaModule/luaModbusAscii.h"
#include "luaModule/luaModbusRtu.h"
#include "luaModule/luaSmtp.h"
#include "luaModule/luaThread.h"
#include "scriptModule/scriptModule.h"
#include "utils/luaUtils.h"

// LuaInterpreter public
LuaInterpreter::LuaInterpreter(const QVariantMap &luaSession, QObject *parent)
    : QObject(parent),
      m_luaSession(luaSession),
      m_luaAT(new LuaAT(this)),
      m_luaDataProcess(new LuaDataProcess(this)),
      m_luaIO(new LuaIO(this)),
      m_luaModbusAscii(new LuaModbusAscii(this)),
      m_luaModbusRtu(new LuaModbusRtu(this)),
      m_luaPort(new LuaPort(this)),
      m_luaSMTP(new LuaSMTP(this)),
      m_luaThread(new LuaThread(this)) {
    // standard lib
    m_lua.open_libraries();
    // add workspace to search path
    sol::table package = m_lua["package"];
    const std::string current_path = package["path"];
    const QString workspacePath = QString("%1/?.lua").arg(m_luaSession["workspaceUrl"].toUrl().toLocalFile());
    const QString new_path = QString("%1;%2").arg(QString::fromStdString(current_path), workspacePath);
    package["path"] = new_path.toStdString();
    // LuaAT lib
    sol::table AT = m_lua.create_table();
    AT.set_function("exec", [this](const std::string &portName, const std::string &command, const sol::optional<std::string> &peerIp) {
        m_luaAT->exec(portName, command, peerIp.value_or(""));
    });
    AT.set_function("read", [this](const std::string &portName, const std::string &command, const sol::optional<std::string> &peerIp) {
        m_luaAT->read(portName, command, peerIp.value_or(""));
    });
    AT.set_function("set", [this](const std::string &portName, const std::string &command, const std::string &data, const sol::optional<std::string> &peerIp) {
        m_luaAT->set(portName, command, data, peerIp.value_or(""));
    });
    AT.set_function("test", [this](const std::string &portName, const std::string &command, const sol::optional<std::string> &peerIp) {
        m_luaAT->test(portName, command, peerIp.value_or(""));
    });
    m_lua["AT"] = AT;
    // LuaDataProcess lib
    sol::table database = m_lua.create_table();
    database.set_function("list", [this] { return sol::as_table(m_luaDataProcess->databaseList()); });
    database.set_function("write", [this](const std::string &key, const sol::object &value) { m_luaDataProcess->databaseWrite(key, value); });
    m_lua["database"] = database;
    connect(m_luaDataProcess, &LuaDataProcess::listDatabase, this, &LuaInterpreter::listDatabase);
    connect(m_luaDataProcess, &LuaDataProcess::writeDatabase, this, &LuaInterpreter::writeDatabase);

    sol::table datatable = m_lua.create_table();
    datatable.set_function("list", [this] { return sol::as_table(m_luaDataProcess->datatableList()); });
    datatable.set_function("write", [this](const std::string &key, const sol::object &value) { m_luaDataProcess->datatableWrite(key, value); });
    m_lua["datatable"] = datatable;
    connect(m_luaDataProcess, &LuaDataProcess::listDatatable, this, &LuaInterpreter::listDatatable);
    connect(m_luaDataProcess, &LuaDataProcess::writeDatatable, this, &LuaInterpreter::writeDatatable);
    // LuaIO lib
    sol::table io = m_lua.create_table();
    io.set_function("log", [this](const sol::variadic_args &args) { m_luaIO->log(args); });
    io.set_function("message", [this](const std::string &text) { m_luaIO->message(text); });
    io.set_function("speak", [](const std::string &text) { LuaIO::speak(text); });
    m_lua["io"] = io;
    connect(m_luaIO, &LuaIO::appendLog, this, &LuaInterpreter::appendLog);
    connect(m_luaIO, &LuaIO::newMessageDialog, this, [this](const QEventLoop *eventloop, const QString &text) {
        emit newMessageDialog(eventloop, m_luaSession["threadId"].toString(), text);
    });
    // LuaModbus lib
    sol::table modbusRtu = m_lua.create_table();
    modbusRtu.set_function("readHoldingRegisters",
                           [this](const std::string &portName, const int slaveAddr, const int startAddr, const int quantity, const sol::optional<int> timeout) {
                               return m_luaModbusRtu->readHoldingRegisters(portName, slaveAddr, startAddr, quantity, timeout.value_or(1000));
                           });
    modbusRtu.set_function("writeSingleRegister",
                           [this](const std::string &portName, const int slaveAddr, const int regAddr, const std::string_view &data, const sol::optional<int> timeout) {
                               m_luaModbusRtu->writeSingleRegister(portName, slaveAddr, regAddr, data, timeout.value_or(1000));
                           });
    modbusRtu.set_function("writeMultipleRegisters",
                           [this](const std::string &portName, const int slaveAddr, const int startAddr, const std::string_view &data, const sol::optional<int> timeout) {
                               m_luaModbusRtu->writeMultipleRegisters(portName, slaveAddr, startAddr, data, timeout.value_or(1000));
                           });
    m_lua["modbusRtu"] = modbusRtu;

    sol::table modbusAscii = m_lua.create_table();
    modbusAscii.set_function("readHoldingRegisters",
                             [this](const std::string &portName, const int slaveAddr, const int startAddr, const int quantity, const sol::optional<int> timeout) {
                                 return m_luaModbusAscii->readHoldingRegisters(portName, slaveAddr, startAddr, quantity, timeout.value_or(1000));
                             });
    modbusAscii.set_function("writeSingleRegister",
                             [this](const std::string &portName, const int slaveAddr, const int regAddr, const std::string_view &data, const sol::optional<int> timeout) {
                                 m_luaModbusAscii->writeSingleRegister(portName, slaveAddr, regAddr, data, timeout.value_or(1000));
                             });
    modbusAscii.set_function("writeMultipleRegisters",
                             [this](const std::string &portName, const int slaveAddr, const int startAddr, const std::string_view &data, const sol::optional<int> timeout) {
                                 m_luaModbusAscii->writeMultipleRegisters(portName, slaveAddr, startAddr, data, timeout.value_or(1000));
                             });
    m_lua["modbusAscii"] = modbusAscii;
    // LuaPort lib
    sol::table port = m_lua.create_table();
    port.set_function("list", [this] { return sol::as_table(m_luaPort->list()); });
    port.set_function("info", [this](const std::string &portName) { return sol::as_table(m_luaPort->info(portName)); });
    port.set_function("open", [this](const std::string &portName) { m_luaPort->open(portName); });
    port.set_function("close", [this](const std::string &portName) { m_luaPort->close(portName); });
    port.set_function("write", [this](const std::string &portName, const std::string_view &data, const sol::optional<std::string> &peerIp) {
        m_luaPort->write(portName, data, peerIp.value_or(""));
    });
    port.set_function("read",
                      [this](const sol::this_state ts, const std::string &portName, const sol::optional<int> timeout, const sol::optional<int> length,
                             const sol::optional<std::string> &peerIp) {
                          return m_luaPort->read(ts, portName, timeout.value_or(0), length.value_or(0), peerIp.value_or(""));
                      });
    m_lua["port"] = port;
    connect(m_luaPort, &LuaPort::listPort, this, &LuaInterpreter::listPort);
    // LuaSMTP lib
    sol::table SMTP = m_lua.create_table();
    SMTP.set_function("ehlo", [this](const std::string &portName) { m_luaSMTP->ehlo(portName); });
    SMTP.set_function("authLogin", [this](const std::string &portName, const std::string &username, const std::string &password) {
        m_luaSMTP->authLogin(portName, username, password);
    });
    SMTP.set_function("mail",
                      [this](const std::string &portName, const std::string &from, const std::string &to, const std::string &subject, const std::string &body,
                             const sol::optional<std::string> &attachment) {
                          m_luaSMTP->mail(portName, from, to, subject, body, attachment.value_or(""));
                      });
    m_lua["SMTP"] = SMTP;
    // LuaThread lib
    sol::table thread = m_lua.create_table();
    thread.set_function("start", [this](const sol::this_state ts, const std::string &scriptPath) { return m_luaThread->start(ts, scriptPath); });
    thread.set_function("stop", [this](const std::string &threadId) { m_luaThread->stop(threadId); });
    thread.set_function("sleep", [this](const int ms) { m_luaThread->sleep(ms); });
    m_lua["thread"] = thread;
    connect(m_luaThread, &LuaThread::startThread, this, &LuaInterpreter::startThread);
    connect(m_luaThread, &LuaThread::stopThread, this, &LuaInterpreter::stopThread); {
        // // register control class
        // lua_newtable(L);
        // lua_pushcfunction(L, lua_leftClick);
        // lua_setfield(L, -2, "leftClick");
        // lua_pushcfunction(L, lua_leftDoubleClick);
        // lua_setfield(L, -2, "leftDoubleClick");
        // lua_pushcfunction(L, lua_rightClick);
        // lua_setfield(L, -2, "rightClick");
        // lua_pushcfunction(L, lua_rightDoubleClick);
        // lua_setfield(L, -2, "rightDoubleClick");
        // lua_pushcfunction(L, lua_keyPress);
        // lua_setfield(L, -2, "keyPress");
        // lua_setglobal(L, "control");
    }
}

void LuaInterpreter::start(const QString &script) {
    lua_State *L = m_lua.lua_state();
    // load session
    m_lua["session"] = &m_luaSession;
    // set hook
    if (m_luaSession["mode"] == LUATHREAD_RUN) {
        // set run hook
        lua_sethook(L, &luaRunHook, LUA_MASKCOUNT, 100);
    } else {
        m_luaSession.insert("this", QVariant::fromValue(this));
        // set debug hook
        lua_sethook(L, &luaDebugHook, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0);
    }
    // frontend
    emit removeMarker(m_luaSession["scriptUrl"].toUrl(), MARKER_DEBUG, -1);
    emit removeMarker(m_luaSession["scriptUrl"].toUrl(), MARKER_ERROR, -1);

    const QString filePath = "@" + m_luaSession["scriptUrl"].toUrl().toLocalFile();
    const sol::protected_function_result result = m_lua.safe_script(
        script.toStdString(),
        sol::script_pass_on_error,
        filePath.toStdString()
    );
    if (!result.valid()) {
        const sol::error err = result;
        emit appendLog(QString::fromStdString(err.what()), "error");
    }
    // frontend
    emit removeMarker(m_luaSession["scriptUrl"].toUrl(), MARKER_DEBUG, -1);
    emit removeMarker(m_luaSession["scriptUrl"].toUrl(), MARKER_ERROR, -1);
    // remove terminate hook
    lua_sethook(L, nullptr, 0, 0);
}

void LuaInterpreter::stateSet(const int state) {
    m_luaSession["state"] = state;
    if (state == DEBUG_STEPOVER || state == DEBUG_STEPOUT) {
        m_luaSession["baseDepth"] = m_luaSession["currentDepth"].toInt();
    }
    if (state != DEBUG_PAUSE) emit quitLoop();
}

void LuaInterpreter::valueSet(const QString &scriptUrl, const QString &expression, const QString &value, const QString &type) {
    emit setValue(scriptUrl, expression, value, type);
}

// LuaInterpreter private
void LuaInterpreter::luaRunHook(lua_State *L, lua_Debug *ar) {
    // check if thread interruption is requested
    if (QThread::currentThread()->isInterruptionRequested()) {
        luaL_error(L, "terminated");
    }
}

void LuaInterpreter::luaDebugHook(lua_State *L, lua_Debug *ar) {
    // handle pause/terminate request
    QThread *thread = QThread::currentThread();
    thread->eventDispatcher()->processEvents(QEventLoop::AllEvents);

    sol::state_view lua(L);
    QVariantMap &session = *lua["session"].get<QVariantMap *>();
    auto *This = qvariant_cast<LuaInterpreter *>(session["this"]);
    emit This->removeMarker(session["currentUrl"].toUrl(), MARKER_DEBUG, -1);
    if (ar->event == LUA_HOOKCALL) {
        session["currentDepth"] = session["currentDepth"].toInt() + 1;
    } else if (ar->event == LUA_HOOKRET) {
        session["currentDepth"] = session["currentDepth"].toInt() - 1;
    } else if (ar->event == LUA_HOOKLINE) {
        // get file info
        lua_getinfo(L, "Sl", ar);
        QUrl currentUrl;
        if (ar->source[0] == '@' && ar->source[1] != '\0') {
            currentUrl = QUrl::fromLocalFile(QString::fromUtf8(ar->source + 1));
        }
        const int currentLine = ar->currentline;
        // debug state machine
        if (session["state"].toInt() == DEBUG_RESUME && g_breakpoints.contains(currentUrl.toString())) {
            if (g_breakpoints[currentUrl].contains(currentLine)) {
                // conditional breakpoint
                {
                    QString condition = g_breakpoints[currentUrl][currentLine]["condition"].toString();
                    if (condition.isEmpty()) {
                        session["state"] = DEBUG_PAUSE;
                    } else {
                        const int base = lua_gettop(L);
                        if (!condition.trimmed().startsWith("return ")) condition = "return " + condition;
                        // create env table
                        sol::environment env(lua, sol::create);
                        sol::table mt = lua.create_table();
                        mt[sol::meta_function::index] = lua.globals();
                        env[sol::metatable_key] = mt;
                        // load upvalues
                        if (lua_getinfo(L, "f", ar)) {
                            const char *name = nullptr;
                            int i = 1;
                            while ((name = lua_getupvalue(L, -1, i++)) != nullptr) {
                                if (name[0] != '(') env[name] = sol::object(L, -1);
                                lua_pop(L, 1);
                            }
                            lua_pop(L, 1);
                        }
                        // load locals
                        const char *name = nullptr;
                        int i = 1;
                        while ((name = lua_getlocal(L, ar, i++)) != nullptr) {
                            if (name[0] != '(') {
                                env[name] = sol::object(L, -1);
                            }
                            lua_pop(L, 1);
                        }
                        // judge condition
                        const sol::protected_function_result condition_result = lua.safe_script(
                            condition.toStdString(),
                            env,
                            sol::script_pass_on_error
                        );
                        if (condition_result.valid()) {
                            const sol::object result = condition_result;
                            if (result.as<bool>()) session["state"] = DEBUG_PAUSE;
                        } else {
                            const sol::error err = condition_result;
                            emit This->appendLog(QString::fromStdString(err.what()), "error");
                        }
                        lua_settop(L, base);
                    }
                }
                // TODO: log/count
            }
        } else if (session["state"].toInt() == DEBUG_STEPOVER && session["currentDepth"].toInt() == session["baseDepth"].toInt()) session["state"] = DEBUG_PAUSE;
        else if (session["state"].toInt() == DEBUG_STEPOUT && session["currentDepth"].toInt() < session["baseDepth"].toInt()) session["state"] = DEBUG_PAUSE;
        else if (session["state"].toInt() == DEBUG_STEPINTO) session["state"] = DEBUG_PAUSE;
        else if (session["state"].toInt() == DEBUG_RUNTOCURSOR && g_cursorPosition["url"].toUrl() == currentUrl && g_cursorPosition["line"].toInt() == currentLine)
            session["state"] = DEBUG_PAUSE;
        if (session["state"].toInt() == DEBUG_PAUSE) {
            // url handle
            emit This->openScript(currentUrl);
            if (currentUrl != session["currentUrl"].toUrl()) session["currentUrl"] = currentUrl;
            // line handle
            emit This->insertMarker(currentUrl, MARKER_DEBUG, currentLine - 1, -1);
            // call stack handle
            auto *callStackModel = new QStandardItemModel(); // NOLINT
            int level = 0;
            while (lua_getstack(L, level, ar)) {
                lua_getinfo(L, "nSl", ar);
                const QUrl scriptUrl = QUrl::fromLocalFile(QString::fromUtf8(ar->source + 1));
                const int line = ar->currentline;
                const QVariantHash position = {
                    {"scriptUrl", scriptUrl},
                    {"line", line}
                };
                auto *fileItem = new QStandardItem(scriptUrl.fileName()); // NOLINT
                fileItem->setData(position, Qt::WhatsThisRole);
                auto *lineItem = new QStandardItem(QString::number(line)); // NOLINT
                auto *nameItem = new QStandardItem(ar->name ? ar->name : "main"); // NOLINT
                callStackModel->insertRow(0, {fileItem, lineItem, nameItem});
                level++;
            }
            emit This->insertCallStack(session["threadId"].toString(), callStackModel);
            // watch handle
            QHash<QString, int> watchHash{};
            for (int i = 0; i < g_watchStandardItemModel->rowCount(); ++i) {
                const QString url = g_watchStandardItemModel->item(i, 0)->data(Qt::WhatsThisRole).toString();
                if (url == currentUrl) {
                    const QString key = g_watchStandardItemModel->item(i, 0)->text();
                    watchHash.insert(key, i);
                }
            }
            if (!watchHash.isEmpty()) {
                // create env table
                sol::environment env(lua, sol::create);
                sol::table mt = lua.create_table();
                mt[sol::meta_function::index] = lua.globals();
                env[sol::metatable_key] = mt;
                // load upvalues
                if (lua_getinfo(L, "f", ar)) {
                    const char *name = nullptr;
                    int i = 1;
                    while ((name = lua_getupvalue(L, -1, i++)) != nullptr) {
                        if (name[0] != '(') env[name] = sol::object(L, -1);
                        lua_pop(L, 1);
                    }
                    lua_pop(L, 1);
                }
                // load locals
                const char *name = nullptr;
                int i = 1;
                while ((name = lua_getlocal(L, ar, i++)) != nullptr) {
                    if (name[0] != '(') env[name] = sol::object(L, -1);
                    lua_pop(L, 1);
                }
                // eval
                for (auto it = watchHash.constBegin(); it != watchHash.constEnd(); ++it) {
                    const auto result = lua.safe_script("return " + it.key().toStdString(), env, sol::script_pass_on_error);
                    QString val = "nil";
                    QString type = "nil";
                    if (result.valid()) {
                        sol::object obj = result;
                        type = lua_typename(L, static_cast<int>(obj.get_type()));
                        val = lua2qstring(obj);
                    }
                    g_watchStandardItemModel->item(it.value(), 1)->setText(val);
                    g_watchStandardItemModel->item(it.value(), 1)->setData(type, Qt::WhatsThisRole);
                }
            }
            // hold thread
            QEventLoop loop;
            connect(This, &LuaInterpreter::quitLoop, &loop, &QEventLoop::quit);
            connect(This, &LuaInterpreter::setValue, This,
                    [This, L, ar, currentUrl](const QString &scriptUrl, const QString &expression, const QString &value, const QString &type) {
                        disconnect(This, &LuaInterpreter::setValue, This, nullptr);
                        if (currentUrl != scriptUrl) {
                            emit This->appendLog(QString("Hot update failed: Not in the current file scope"), "error");
                            return;
                        }
                        bool updated = false;
                        // try local value
                        const char *localValue = nullptr;
                        int local_i = 1;
                        while ((localValue = lua_getlocal(L, ar, local_i)) != nullptr) {
                            if (expression == localValue) {
                                lua_pushvariant(L, value, type);
                                lua_setlocal(L, ar, local_i);
                                lua_pop(L, 1);
                                updated = true;
                                emit This->appendLog(QString("Hot update executed: local %1 = %2").arg(expression, value), "info");
                                break;
                            }
                            lua_pop(L, 1);
                            local_i++;
                        }
                        // try up value
                        if (!updated) {
                            if (lua_getinfo(L, "f", ar)) {
                                const char *upValue = nullptr;
                                int upvalue_i = 1;
                                while ((upValue = lua_getupvalue(L, -1, upvalue_i)) != nullptr) {
                                    if (expression == upValue) {
                                        lua_pushvariant(L, value, type);
                                        lua_setupvalue(L, -3, upvalue_i);
                                        lua_pop(L, 1);
                                        updated = true;
                                        emit This->appendLog(QString("Hot update executed: upvalue %1 = %2").arg(expression, value), "info");
                                        break;
                                    }
                                    lua_pop(L, 1);
                                    upvalue_i++;
                                }
                                lua_pop(L, 1);
                            }
                        }
                        if (!updated) {
                            emit This->appendLog(QString("Hot update failed: variable '%1' not found").arg(expression), "error");
                        }
                    });
            loop.exec();
        }
        if (QThread::currentThread()->isInterruptionRequested()) {
            luaL_error(L, "terminated");
        }
    }
}
