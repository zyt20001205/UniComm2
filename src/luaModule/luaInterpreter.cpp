#include "luaModule/luaInterpreter.h"

#include <QStandardItemModel>
#include <QTimer>
#include <sol/sol.hpp>

#include "globals.h"
#include "luaModule/luaControl.h"
#include "luaModule/luaDataProcess.h"
#include "luaModule/luaIO.h"
#include "luaModule/luaPort.h"
#include "luaModule/luaModbusAscii.h"
#include "luaModule/luaModbusRtu.h"
#include "luaModule/luaSmtp.h"
#include "luaModule/luaString.h"
#include "luaModule/luaThread.h"
#include "scriptModule/scriptModule.h"
#include "utils/luaUtils.h"
#include "utils/uniCast.h"

// LuaInterpreter public
LuaInterpreter::LuaInterpreter(const QVariantMap &luaSession, QObject *parent)
    : QObject(parent),
      m_luaSession(luaSession),
      m_luaDataProcess(new LuaDataProcess(this)),
      m_luaIO(new LuaIO(this)),
      m_luaModbusAscii(new LuaModbusAscii(this)),
      m_luaModbusRtu(new LuaModbusRtu(this)),
      m_luaPort(new LuaPort(this)),
      m_luaSmtp(new LuaSmtp(this)),
      m_luaString(new LuaString(this)),
      m_luaThread(new LuaThread(this)) {
    // standard lib
    m_lua.open_libraries();
    // add workspace to search path
    sol::table package = m_lua["package"];
    const std::string currentPath = package["path"];
    const QString workspacePath = QString("%1/?.lua").arg(m_luaSession["workspaceUrl"].toUrl().toLocalFile());
    const QString newPath = QString("%1;%2").arg(QString::fromStdString(currentPath), workspacePath);
    package["path"] = newPath.toStdString();
    // LuaDataProcess lib
    auto database = m_lua.create_table();
    database.set_function("list", [](const sol::this_state ts) { return LuaDataProcess::databaseList(ts); });
    database.set_function("write", [](const std::string &key, const sol::object &value) { LuaDataProcess::databaseWrite(key, value); });
    m_lua["database"] = database;

    auto datatable = m_lua.create_table();
    datatable.set_function("list", [](const sol::this_state ts) { return LuaDataProcess::datatableList(ts); });
    datatable.set_function("write", [](const std::string &key, const sol::object &value) { LuaDataProcess::datatableWrite(key, value); });
    datatable.set_function("export", [](const sol::optional<std::string> &fileName) { LuaDataProcess::datatableExport(fileName.value_or("")); });
    m_lua["datatable"] = datatable;
    // LuaIO lib
    auto io = m_lua.create_table();
    io.set_function("log", [this](const sol::variadic_args &args) { m_luaIO->log(args); });
    io.set_function("message", [this](const std::string &text) { m_luaIO->message(text); });
    io.set_function("speak", [](const std::string &text) { LuaIO::speak(text); });
    m_lua["io"] = io;
    connect(m_luaIO, &LuaIO::appendLog, this, &LuaInterpreter::appendLog);
    connect(m_luaIO, &LuaIO::newMessageDialog, this, [this](const QEventLoop *eventloop, const QString &text) {
        emit newMessageDialog(eventloop, m_luaSession["threadId"].toString(), text);
    });
    // LuaModbus lib
    auto modbusRtu = m_lua.create_table();
    modbusRtu.set_function("readHoldingRegisters",
                           [](const std::string &portName, const int slaveAddr, const int startAddr, const int quantity, const sol::optional<int> timeout) {
                               return LuaModbusRtu::readHoldingRegisters(portName, slaveAddr, startAddr, quantity, timeout.value_or(1000));
                           });
    modbusRtu.set_function("writeSingleRegister",
                           [](const std::string &portName, const int slaveAddr, const int regAddr, const std::string_view &data, const sol::optional<int> timeout) {
                               LuaModbusRtu::writeSingleRegister(portName, slaveAddr, regAddr, data, timeout.value_or(1000));
                           });
    modbusRtu.set_function("writeMultipleRegisters",
                           [](const std::string &portName, const int slaveAddr, const int startAddr, const std::string_view &data, const sol::optional<int> timeout) {
                               LuaModbusRtu::writeMultipleRegisters(portName, slaveAddr, startAddr, data, timeout.value_or(1000));
                           });
    m_lua["modbusRtu"] = modbusRtu;

    sol::table modbusAscii = m_lua.create_table();
    modbusAscii.set_function("readHoldingRegisters",
                             [](const std::string &portName, const int slaveAddr, const int startAddr, const int quantity, const sol::optional<int> timeout) {
                                 return LuaModbusAscii::readHoldingRegisters(portName, slaveAddr, startAddr, quantity, timeout.value_or(1000));
                             });
    modbusAscii.set_function("writeSingleRegister",
                             [](const std::string &portName, const int slaveAddr, const int regAddr, const std::string_view &data, const sol::optional<int> timeout) {
                                 LuaModbusAscii::writeSingleRegister(portName, slaveAddr, regAddr, data, timeout.value_or(1000));
                             });
    modbusAscii.set_function("writeMultipleRegisters",
                             [](const std::string &portName, const int slaveAddr, const int startAddr, const std::string_view &data, const sol::optional<int> timeout) {
                                 LuaModbusAscii::writeMultipleRegisters(portName, slaveAddr, startAddr, data, timeout.value_or(1000));
                             });
    m_lua["modbusAscii"] = modbusAscii;
    // LuaPort lib
    auto port = m_lua.create_table();
    port.set_function("list", [](const sol::this_state ts) { return LuaPort::list(ts); });
    port.set_function("info", [](const sol::this_state ts, const std::string &portName) { return LuaPort::info(ts, portName); });
    port.set_function("open", [](const std::string &portName) { LuaPort::open(portName); });
    port.set_function("close", [](const std::string &portName) { LuaPort::close(portName); });
    port.set_function("clear", [](const std::string &portName) { LuaPort::clear(portName); });
    port.set_function("write", [](const std::string &portName, const std::string_view &data, const sol::optional<std::string> &peerIp) {
        LuaPort::write(portName, data, peerIp.value_or(""));
    });
    port.set_function("read",
                      [](const sol::this_state ts, const std::string &portName, const sol::optional<int> length, const sol::optional<int> timeout,
                             const sol::optional<std::string> &peerIp) {
                          return LuaPort::read(ts, portName, length.value_or(0), timeout.value_or(0), peerIp.value_or(""));
                      });
    m_lua["port"] = port;
    // LuaSmtp lib
    auto smtp = m_lua.create_table();
    smtp.set_function("ehlo", [](const std::string &portName) { LuaSmtp::ehlo(portName); });
    smtp.set_function("authLogin", [](const std::string &portName, const std::string &username, const std::string &password) {
        LuaSmtp::authLogin(portName, username, password);
    });
    smtp.set_function("mail",
                      [](const std::string &portName, const std::string &from, const std::string &to, const std::string &subject, const std::string &body,
                             const sol::optional<std::string> &attachment) {
                          LuaSmtp::mail(portName, from, to, subject, body, attachment.value_or(""));
                      });
    m_lua["smtp"] = smtp;
    // LuaString lib
    auto string = m_lua["string"].get_or_create<sol::table>();
    string.set_function("toHex", [](const std::string_view &ba, const sol::optional<char> separator) { return LuaString::toHex(ba, separator.value_or('\0')); });
    string.set_function("fromHex", [](const std::string &str) { return LuaString::fromHex(str); });
    m_lua["string"] = string;
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

void LuaInterpreter::stackSet(lua_State *L, lua_Debug *ar) {
    sol::state_view lua(L);
    QVariantMap &session = *lua["session"].get<QVariantMap *>();
    auto *This = qvariant_cast<LuaInterpreter *>(session["this"]);
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
}

void LuaInterpreter::watchSet(lua_State *L, lua_Debug *ar) {
    lua_getstack(L, 0, ar);
    lua_getinfo(L, "S", ar);
    QUrl currentUrl;
    if (ar->source[0] == '@' && ar->source[1] != '\0') {
        currentUrl = QUrl::fromLocalFile(QString::fromUtf8(ar->source + 1));
    }
    sol::state_view lua(L);
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
                val = uni_cast<QString>(obj);
            }
            g_watchStandardItemModel->item(it.value(), 1)->setText(val);
            g_watchStandardItemModel->item(it.value(), 1)->setData(type, Qt::WhatsThisRole);
        }
    }
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
                // enabled check
                if (!g_breakpoints[currentUrl][currentLine]["enabled"].toBool()) return;
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
            stackSet(L, ar);
            // watch handle
            watchSet(L, ar);
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
                        // value not found
                        if (!updated) {
                            emit This->appendLog(QString("Hot update failed: variable '%1' not found").arg(expression), "error");
                        } else {
                            // watch handle
                            watchSet(L, ar);
                        }
                    });
            loop.exec();
        }
        if (QThread::currentThread()->isInterruptionRequested()) {
            luaL_error(L, "terminated");
        }
    }
}
