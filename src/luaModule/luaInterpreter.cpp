#include "luaModule/luaInterpreter.h"

#include <QStandardItemModel>
#include <QTimer>
#include <sol/sol.hpp>

#include "globals.h"
#include "luaModule/luaControl.h"
#include "luaModule/luaDataProcess.h"
#include "luaModule/luaIO.h"
#include "luaModule/luaPort.h"
#include "luaModule/luaModbusRtu.h"
#include "luaModule/luaThread.h"
#include "scriptModule/scriptModule.h"
#include "utils/luaUtils.h"

// LuaInterpreter public
LuaInterpreter::LuaInterpreter(const QVariantMap &luaSession, QObject *parent)
    : QObject(parent),
      m_luaSession(luaSession),
      m_luaIO(new LuaIO(this)),
      m_luaModbusRtu(new LuaModbusRtu(this)),
      m_luaPort(new LuaPort(this)),
      m_luaThread(new LuaThread(this)) {
    // standard lib
    m_lua.open_libraries();
    // add workspace to search path
    sol::table package = m_lua["package"];
    const std::string current_path = package["path"];
    const QString workspacePath = QString("%1/?.lua").arg(m_luaSession["workspaceUrl"].toUrl().toLocalFile());
    const QString new_path = QString("%1;%2").arg(QString::fromStdString(current_path), workspacePath);
    package["path"] = new_path.toStdString();
    // LuaIO lib
    sol::table io = m_lua.create_table();
    io.set_function("log", [this](const sol::variadic_args &args) { m_luaIO->log(args); });
    io.set_function("message", [this](const std::string &text) { m_luaIO->message(text); });
    io.set_function("speak", [](const std::string &text) { LuaIO::speak(text); });
    m_lua["io"] = io;
    connect(m_luaIO, &LuaIO::appendLog, this, &LuaInterpreter::appendLog);
    connect(m_luaIO, &LuaIO::newMessageDialog, this, [this](const QString &text, const QEventLoop *eventloop) {
        emit newMessageDialog(m_luaSession["threadId"].toString(), text, eventloop);
    });
    // LuaModbusRtu lib
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
    // LuaPort lib
    sol::table port = m_lua.create_table();
    port.set_function("list", [this] { return sol::as_table(m_luaPort->list()); });
    port.set_function("info", [this](const std::string &portName) { return sol::as_table(m_luaPort->info(portName)); });
    port.set_function("open", [this](const std::string &portName) { m_luaPort->open(portName); });
    port.set_function("close", [this](const std::string &portName) { m_luaPort->close(portName); });
    port.set_function("write", [this](const std::string &portName, const std::string_view &data, const sol::optional<std::string> &peerIp) {
        m_luaPort->write(portName, data, peerIp.value_or(""));
    });
    port.set_function("read", [this](const std::string &portName, const sol::optional<int> timeout, const sol::optional<int> length, const sol::optional<std::string> &peerIp) {
        return m_luaPort->read(portName, timeout.value_or(0), length.value_or(0), peerIp.value_or(""));
    });
    m_lua["port"] = port;
    connect(m_luaPort, &LuaPort::listPort, this, &LuaInterpreter::listPort);
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
        // // register database class
        // lua_newtable(L);
        // lua_pushcfunction(L, lua_databaseList);
        // lua_setfield(L, -2, "list");
        // lua_pushcfunction(L, lua_databaseWrite);
        // lua_setfield(L, -2, "write");
        // lua_pushcfunction(L, lua_databaseClear);
        // lua_setfield(L, -2, "clear");
        // lua_setglobal(L, "database");
        // // register datatable class
        // lua_newtable(L);
        // lua_pushcfunction(L, lua_datatableList);
        // lua_setfield(L, -2, "list");
        // lua_pushcfunction(L, lua_datatableWrite);
        // lua_setfield(L, -2, "write");
        // lua_pushcfunction(L, lua_datatableClear);
        // lua_setfield(L, -2, "clear");
        // lua_pushcfunction(L, lua_datatableExport);
        // lua_setfield(L, -2, "export");
        // lua_setglobal(L, "datatable");
        // // register dataplot class
        // lua_newtable(L);
        // lua_pushcfunction(L, lua_dataplotAppend);
        // lua_setfield(L, -2, "append");
        // lua_setglobal(L, "dataplot");
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

void LuaInterpreter::hotUpdate(const QString &varScope, const QString &varName, const QString &varValue) const {
    lua_State *L = m_lua.lua_state();
    lua_Debug ar;
    if (varScope == "local") {
        // local table
        if (varName.contains(".")) {
            QStringList path = varName.split('.');

            if (lua_getstack(L, 0, &ar)) {
                int i = 1;
                QString firstname;
                while ((firstname = lua_getlocal(L, &ar, i)) != nullptr) {
                    if (path.first() == firstname) {
                        break;
                    }
                    lua_pop(L, 1);
                    i++;
                }

                for (int j = 1; j + 1 < path.size(); ++j) {
                    const QString &part = path[j];
                    lua_pushstring(L, part.toUtf8().constData());
                    lua_gettable(L, -2);
                    lua_remove(L, -2);
                }

                const QByteArray lastname = path.last().toUtf8();
                lua_pushstring(L, lastname.constData());
                lua_gettable(L, -2);
                lua_pushqstring(L, -1, varValue);
                lua_remove(L, -2);
                lua_setfield(L, -2, lastname.constData());
                lua_pop(L, 1);

                qDebug() << "local table" << varName << "updated to" << varValue;
            }
        } else {
            // local boolean/number/string
            if (lua_getstack(L, 0, &ar)) {
                int i = 1;
                QString name;
                while ((name = lua_getlocal(L, &ar, i)) != nullptr) {
                    if (varName == name) {
                        lua_pushqstring(L, -1, varValue);
                        lua_setlocal(L, &ar, i);
                        lua_pop(L, 1);
                        qDebug() << "local variable" << varName << "updated to" << varValue;
                        break;
                    }
                    lua_pop(L, 1);
                    i++;
                }
            }
        }
    } else {
        if (lua_getstack(L, 0, &ar)) {
            lua_getinfo(L, "f", &ar);
            int i = 1;
            // up table
            if (varName.contains('.')) {
                QStringList path = varName.split('.');
                QString firstname;
                while ((firstname = lua_getupvalue(L, -1, i)) != nullptr) {
                    if (path.first() == firstname) {
                        break;
                    }
                    lua_pop(L, 1);
                    i++;
                }
                for (int j = 1; j + 1 < path.size(); ++j) {
                    const QString &part = path[j];
                    lua_pushstring(L, part.toUtf8().constData());
                    lua_gettable(L, -2);
                    lua_remove(L, -2);
                }
                const QByteArray lastname = path.last().toUtf8();
                lua_pushstring(L, lastname.constData());
                lua_gettable(L, -2);
                lua_pushqstring(L, -1, varValue);
                lua_remove(L, -2);
                lua_setfield(L, -2, lastname.constData());
                lua_pop(L, 1);
                qDebug() << "up table" << varName << "updated to" << varValue;
                lua_pop(L, 1);
            } else {
                // up boolean/number/string
                QString name;
                while ((name = lua_getupvalue(L, -1, i)) != nullptr) {
                    if (varName == name) {
                        lua_pushqstring(L, -1, varValue);
                        lua_setupvalue(L, -3, i);
                        qDebug() << "up variable" << varName << "updated to" << varValue;
                        break;
                    }
                    lua_pop(L, 1);
                    i++;
                }
                lua_pop(L, 1);
            }
        }
    }
}

// LuaInterpreter private
void LuaInterpreter::luaRunHook(lua_State *L, lua_Debug *ar) {
    // check if thread interruption is requested
    if (QThread::currentThread()->isInterruptionRequested()) {
        luaL_error(L, "terminated");
    }
}

void LuaInterpreter::luaDebugHook(lua_State *L, lua_Debug *ar) {
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
                        // load locals into env table
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
            // TODO: var handle
            {
                // var tree
                // {
                //     // init var tree
                //     auto *varTree = new QStandardItemModel(); // NOLINT
                //     varTree->setHorizontalHeaderLabels({tr("Name"), tr("Type"), tr("Value")});
                //     // table recursion lambda
                //     auto appendTable = [](lua_State *L, QStandardItem *parentNameItem, const QString &parentVarname, const QString &parentVarScope, const int tableIndex,
                //                           auto &&self) -> void {
                //         lua_pushnil(L);
                //         while (lua_next(L, tableIndex) != 0) {
                //             lua_pushvalue(L, -2);
                //             QString varName = lua_tostring(L, -1);
                //             lua_pop(L, 1);
                //             QString varType = lua_typename(L, lua_type(L, -1));
                //             lua_pushvalue(L, -1);
                //             QString varValue = lua_toqstring(L, -1);
                //             lua_pop(L, 1);
                //
                //             auto *localNameItem = new QStandardItem(varName); // NOLINT
                //             localNameItem->setEditable(false);
                //             auto *localTypeItem = new QStandardItem(varType); // NOLINT
                //             localTypeItem->setEditable(false);
                //             QStandardItem *valueItem = new QStandardItem(varValue); // NOLINT
                //
                //             if (lua_type(L, -1) == LUA_TBOOLEAN || lua_type(L, -1) == LUA_TNUMBER || lua_type(L, -1) == LUA_TSTRING) {
                //                 valueItem->setData(parentVarScope, Qt::UserRole + 1);
                //                 valueItem->setData(parentVarname + "." + varName, Qt::UserRole + 2);
                //             } else if (lua_type(L, -1) == LUA_TTABLE) {
                //                 valueItem->setEditable(false);
                //                 self(L, localNameItem, parentVarname + "." + varName, parentVarScope, lua_gettop(L), self);
                //             } else {
                //                 valueItem->setEditable(false);
                //             }
                //             parentNameItem->appendRow({localNameItem, localTypeItem, valueItem});
                //             lua_pop(L, 1);
                //         }
                //     };
                //     // local var
                //     auto *localVar = new QStandardItem("local"); // NOLINT
                //     localVar->setEditable(false);
                //     varTree->appendRow(localVar);
                //     int i = 1;
                //     QString localVarName;
                //     while ((localVarName = lua_getlocal(L, ar, i)) != nullptr) {
                //         if (localVarName[0] != '(') {
                //             QString localVarType = lua_typename(L, lua_type(L, -1));
                //             QString localVarValue = lua_toqstring(L, -1);
                //
                //             QStandardItem *localNameItem = new QStandardItem(localVarName); // NOLINT
                //             localNameItem->setEditable(false);
                //             QStandardItem *localTypeItem = new QStandardItem(localVarType); // NOLINT
                //             localTypeItem->setEditable(false);
                //             QStandardItem *localValueItem = new QStandardItem(localVarValue); // NOLINT
                //
                //             if (lua_type(L, -1) == LUA_TBOOLEAN || lua_type(L, -1) == LUA_TNUMBER || lua_type(L, -1) == LUA_TSTRING) {
                //                 localValueItem->setData("local", Qt::UserRole + 1);
                //                 localValueItem->setData(localVarName, Qt::UserRole + 2);
                //             } else if (lua_type(L, -1) == LUA_TTABLE) {
                //                 localValueItem->setEditable(false);
                //                 appendTable(L, localNameItem, localVarName, "local", lua_gettop(L), appendTable);
                //             } else {
                //                 localValueItem->setEditable(false);
                //             }
                //             localVar->appendRow({localNameItem, localTypeItem, localValueItem});
                //         }
                //         lua_pop(L, 1);
                //         i++;
                //     }
                //     // up var
                //     auto *upVar = new QStandardItem("up"); // NOLINT
                //     upVar->setEditable(false);
                //     varTree->appendRow(upVar);
                //     lua_getinfo(L, "f", ar);
                //     i = 1;
                //     QString upVarName;
                //     while ((upVarName = lua_getupvalue(L, -1, i)) != nullptr) {
                //         if (upVarName[0] != '(' && upVarName[0] != '_') {
                //             QString upVarType = lua_typename(L, lua_type(L, -1));
                //             QString upVarValue = lua_toqstring(L, -1);
                //
                //             QStandardItem *upNameItem = new QStandardItem(upVarName); // NOLINT
                //             upNameItem->setEditable(false);
                //             QStandardItem *upTypeItem = new QStandardItem(upVarType); // NOLINT
                //             upTypeItem->setEditable(false);
                //             QStandardItem *upValueItem = new QStandardItem(upVarValue); // NOLINT
                //
                //             if (lua_type(L, -1) == LUA_TBOOLEAN || lua_type(L, -1) == LUA_TNUMBER || lua_type(L, -1) == LUA_TSTRING) {
                //                 upValueItem->setData("up", Qt::UserRole + 1);
                //                 upValueItem->setData(upVarName, Qt::UserRole + 2);
                //             } else if (lua_type(L, -1) == LUA_TTABLE) {
                //                 upValueItem->setEditable(false);
                //                 appendTable(L, upNameItem, upVarName, "up", lua_gettop(L), appendTable);
                //             } else {
                //                 upValueItem->setEditable(false);
                //             }
                //             upVar->appendRow({upNameItem, upTypeItem, upValueItem});
                //         }
                //         lua_pop(L, 1);
                //         i++;
                //     }
                //     lua_pop(L, 1);
                //     // send to debug module
                //     QMetaObject::invokeMethod(g_mainWindow, [debugData, varTree] {
                //         g_debug->varReturn(debugData->threadId, varTree);
                //     }, Qt::BlockingQueuedConnection);
                // }
            }
            // hold thread
            QEventLoop loop;
            connect(This, &LuaInterpreter::quitLoop, &loop, &QEventLoop::quit);
            loop.exec();
        }
        if (QThread::currentThread()->isInterruptionRequested()) {
            luaL_error(L, "terminated");
        }
    }
}
