#include "luaInterpreter.h"

#include "debug.h"
#include "luaControl.h"
#include "luaDataProcess.h"
#include "luaPort.h"
#include "luaMiscellaneous.h"
#include "luaModbus.h"
#include "globals.h"
#include "log.h"
#include "script.h"
#include "utils.h"

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
    lua_register(L, "stop", lua_stop);
    lua_register(L, "wait", lua_wait);
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
            *ptrHolder = nullptr;
        }
        // close interpreter
        m_debugData.reset();
        lua_close(L);
        L = nullptr;
    }
}

void LuaInterpreter::run(const QString &script) const {
    // set terminate hook
    lua_sethook(L, luaTerminateHook, LUA_MASKCOUNT, 100);
    // lua exec preparation
    QMetaObject::invokeMethod(g_mainWindow, [this] {
        g_script->markerShow(m_scriptUrl, MARKER_ARROW);
        g_script->markerShow(m_scriptUrl, MARKER_ERROR);
    }, Qt::QueuedConnection);
    // lua exec
    const QString filePath = "@" + m_scriptUrl.toLocalFile();
    const int load_result = luaL_loadbuffer(L, script.toUtf8().constData(), script.size(), filePath.toUtf8().constData());
    if (load_result == LUA_OK) {
        const int pcall_result = lua_pcall(L, 0, LUA_MULTRET, 0);
        if (pcall_result == LUA_OK) {
            QMetaObject::invokeMethod(g_mainWindow, [this] {
                g_script->markerShow(m_scriptUrl, MARKER_ARROW);
                g_script->markerShow(m_scriptUrl, MARKER_ERROR);
            }, Qt::QueuedConnection);
        } else {
            handleError();
        }
    } else {
        handleError();
    }
    // remove terminate hook
    lua_sethook(L, nullptr, 0, 0);
}

void LuaInterpreter::debug(const QString &script, const DebugData &debugData) {
    // set debug hook
    lua_sethook(L, &luaDebugHook, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0);
    // save debug session
    const auto ptrHolder = static_cast<void **>(lua_getextraspace(L));
    m_debugData.reset(new DebugData(debugData));
    *ptrHolder = m_debugData.data();
    // lua debug preparation
    QMetaObject::invokeMethod(g_mainWindow, [this] {
        g_script->markerShow(m_scriptUrl, MARKER_ARROW);
        g_script->markerShow(m_scriptUrl, MARKER_ERROR);
    }, Qt::QueuedConnection);
    // lua debug
    const QString filePath = "@" + m_scriptUrl.toLocalFile();
    const int load_result = luaL_loadbuffer(L, script.toUtf8().constData(), script.size(), filePath.toUtf8().constData());
    if (load_result == LUA_OK) {
        const int pcall_result = lua_pcall(L, 0, LUA_MULTRET, 0);
        if (pcall_result == LUA_OK) {
            QMetaObject::invokeMethod(g_mainWindow, [this] {
                g_script->markerShow(m_scriptUrl, MARKER_ARROW);
                g_script->markerShow(m_scriptUrl, MARKER_ERROR);
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

void LuaInterpreter::debugStateSet(const int state) const {
    m_debugData->state = state;
    if (state == DEBUG_STEPOVER || state == DEBUG_STEPOUT) {
        m_debugData->baseDepth = m_debugData->depth;
    }
}

void LuaInterpreter::hotUpdate(const QString &varScope, const QString &varName, const QString &varValue) const {
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
void LuaInterpreter::luaTerminateHook(lua_State *L, lua_Debug *ar) {
    (void) ar;
    // check if thread interruption is requested
    if (QThread::currentThread()->isInterruptionRequested()) {
        luaL_error(L, "terminated");
    }
}

void LuaInterpreter::luaDebugHook(lua_State *L, lua_Debug *ar) {
    // check if thread interruption is requested
    if (QThread::currentThread()->isInterruptionRequested()) {
        luaL_error(L, "terminated");
    }
    const auto ptrHolder = static_cast<void **>(lua_getextraspace(L));
    auto debugData = static_cast<DebugData *>(*ptrHolder);
    // clear highlight
    QMetaObject::invokeMethod(g_mainWindow, [debugData] {
        g_script->markerShow(debugData->currentUrl, MARKER_ARROW);
    }, Qt::BlockingQueuedConnection);
    if (ar->event == LUA_HOOKCALL) {
        debugData->depth += 1;
    } else if (ar->event == LUA_HOOKRET) {
        debugData->depth -= 1;
    } else if (ar->event == LUA_HOOKLINE) {
        // get file info
        lua_getinfo(L, "Sl", ar);
        QUrl currentUrl;
        if (ar->source[0] == '@' && ar->source[1] != '\0') {
            currentUrl = QUrl::fromLocalFile(QString::fromUtf8(ar->source + 1));
        }
        // debug state machine
        if (debugData->state == DEBUG_RUN && g_breakpoints.contains(currentUrl)) {
            if (g_breakpoints[currentUrl].contains(ar->currentline)) {
                QString expression = g_breakpoints[currentUrl][ar->currentline]["expr"].toString();
                const int base = lua_gettop(L);
                if (expression.isEmpty()) {
                    debugData->state = DEBUG_PAUSE;
                } else {
                    if (!expression.trimmed().startsWith("return ")) expression = "return " + expression;
                    // create env table
                    lua_newtable(L);
                    const int env = lua_gettop(L);
                    // load locals into env table
                    const char *name = nullptr;
                    int i = 1;
                    while ((name = lua_getlocal(L, ar, i++)) != nullptr) {
                        lua_setfield(L, env, name);
                    }
                    // load expression
                    if (const int load_result = luaL_loadstring(L, expression.toUtf8().constData()); load_result == LUA_OK) {
                        // overwrite _ENV with env table
                        lua_pushvalue(L, env);
                        lua_setupvalue(L, -2, 1);
                        // judge expression
                        const int pcall_result = lua_pcall(L, 0, 1, 0);
                        if (pcall_result == LUA_OK) {
                            const bool result = lua_toboolean(L, -1);
                            lua_pop(L, 1);
                            if (result) debugData->state = DEBUG_PAUSE;
                        } else {
                            const QString error = lua_tostring(L, -1);
                            QMetaObject::invokeMethod(g_mainWindow, [error] {
                                g_log->logAppend(error, "error");
                            }, Qt::BlockingQueuedConnection);
                            lua_pop(L, 1);
                        }
                    } else {
                        const QString error = lua_tostring(L, -1);
                        QMetaObject::invokeMethod(g_mainWindow, [error] {
                            g_log->logAppend(error, "error");
                        }, Qt::BlockingQueuedConnection);
                        lua_pop(L, 1);
                    }
                    lua_settop(L, base);
                }
            }
        } else if (debugData->state == DEBUG_STEPOVER && debugData->depth == debugData->baseDepth) debugData->state = DEBUG_PAUSE;
        else if (debugData->state == DEBUG_STEPOUT && debugData->depth < debugData->baseDepth) debugData->state = DEBUG_PAUSE;
        else if (debugData->state == DEBUG_STEPINTO) debugData->state = DEBUG_PAUSE;
        else if (debugData->state == DEBUG_RUNTOCURSOR && g_cursorPosition["url"].toUrl() == currentUrl && g_cursorPosition["line"].toInt() == ar->currentline)
            debugData->state = DEBUG_PAUSE;
        if (debugData->state == DEBUG_PAUSE) {
            // src handle
            if (currentUrl != debugData->currentUrl) {
                QMetaObject::invokeMethod(g_mainWindow, [currentUrl] {
                    g_script->scriptOpen(currentUrl);
                }, Qt::BlockingQueuedConnection);
                debugData->currentUrl = currentUrl;
            }
            // line handle
            QMetaObject::invokeMethod(g_mainWindow, [debugData, ar] {
                g_script->markerShow(debugData->currentUrl, MARKER_ARROW, ar->currentline);
            }, Qt::BlockingQueuedConnection);
            // var tree
            {
                // init var tree
                auto *varTree = new QStandardItemModel(); // NOLINT
                varTree->setHorizontalHeaderLabels({tr("Name"), tr("Type"), tr("Value")});
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
                varTree->appendRow(localVar);
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
                varTree->appendRow(upVar);
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
                // send to debug module
                QMetaObject::invokeMethod(g_mainWindow, [debugData, varTree] {
                    g_debug->varReturn(debugData->threadId, varTree);
                }, Qt::BlockingQueuedConnection);
            }
            // call table
            {
                // init call table
                auto *callTable = new QStandardItemModel(); // NOLINT
                callTable->setHorizontalHeaderLabels({tr("Script"), tr("Line"), tr("Name"), tr("View")});
                // get call stack
                int level = 0;
                while (lua_getstack(L, level, ar)) {
                    lua_getinfo(L, "nSl", ar);
                    const QUrl scriptUrl = QUrl::fromLocalFile(QString::fromUtf8(ar->source + 1));
                    callTable->insertRow(0);
                    callTable->setItem(0, 0, new QStandardItem(scriptUrl.fileName()));
                    callTable->setItem(0, 1, new QStandardItem(QString::number(ar->currentline)));
                    callTable->setItem(0, 2, new QStandardItem(ar->name ? ar->name : "?"));
                    auto *viewItem = new QStandardItem(QIcon(":/icon/arrowRight.svg"), ""); // NOLINT
                    viewItem->setData(QVariant(scriptUrl), Qt::UserRole + 1);
                    viewItem->setData(QVariant(ar->currentline), Qt::UserRole + 2);
                    callTable->setItem(0, 3, viewItem);
                    level++;
                }
                // send to debug module
                QMetaObject::invokeMethod(g_mainWindow, [debugData, callTable] {
                    g_debug->callReturn(debugData->threadId, callTable);
                }, Qt::BlockingQueuedConnection);
            }
            // hold thread
            QEventLoop loop;
            // interruption check
            QTimer timer;
            timer.setInterval(100);
            connect(&timer, &QTimer::timeout, [&loop] {
                if (QThread::currentThread()->isInterruptionRequested()) loop.quit();
            });
            timer.start();
            // debug state check
            connect(g_debug, &Debug::resume, &loop, [debugData, &loop](const QString &threadId) {
                if (threadId == debugData->threadId) loop.quit();
            });
            loop.exec();
        }
    }
}

void LuaInterpreter::handleError() const {
    const QString error = lua_tostring(L, -1);
    int line = -1;
    static const QRegularExpression re(R"(:(\d+):)");
    if (const auto match = re.match(error); match.hasMatch()) line = match.captured(1).toInt();
    QMetaObject::invokeMethod(g_mainWindow, [this, line, error] {
        g_script->markerShow(m_scriptUrl, MARKER_ERROR, line);
        g_log->logAppend(error, "error");
    }, Qt::QueuedConnection);
    lua_pop(L, 1);
}