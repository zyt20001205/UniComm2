#include "luaModule/luaPort.h"

#include "globals.h"
#include "portModule/basePort.h"
#include "portModule/portModule.h"
#include "utils/luaUtils.h"

int lua_portOpen(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 1)
        luaL_error(L, "unexpected number of arguments");
    // convert arguments
    const char *param1 = luaL_checkstring(L, 1);
    // start operation
    const QString portName = QString::fromUtf8(param1);
    if (!g_port->m_portHash.contains(portName)) {
        luaL_error(L, "port '%s' does not exist", portName.toUtf8().constData());
    } else {
        bool status;
        auto *portObject = g_port->m_portHash[portName];
        QMetaObject::invokeMethod(portObject, [&status, portObject] {
            status = portObject->open();
        }, Qt::BlockingQueuedConnection);
        lua_pushboolean(L, status);
        return 1;
    }
}

int lua_portClose(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 1)
        luaL_error(L, "unexpected number of arguments");
    // convert arguments
    const char *param1 = luaL_checkstring(L, 1);
    // start operation
    const QString portName = QString::fromUtf8(param1);
    if (!g_port->m_portHash.contains(portName)) {
        luaL_error(L, "port '%s' does not exist", portName.toUtf8().constData());
    } else {
        auto *portObject = g_port->m_portHash[portName];
        QMetaObject::invokeMethod(portObject, [portObject] {
            portObject->close();
        }, Qt::BlockingQueuedConnection);
        return 0;
    }
}

int lua_portInfo(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 1)
        luaL_error(L, "unexpected number of arguments");
    // convert arguments
    const char *param1 = luaL_checkstring(L, 1);
    // start operation
    const QString portName = QString::fromUtf8(param1);
    if (!g_port->m_portHash.contains(portName)) {
        luaL_error(L, "port '%s' does not exist", portName.toUtf8().constData());
    } else {
        QVariantMap infoMap;
        auto *portObject = g_port->m_portHash[portName];
        QMetaObject::invokeMethod(portObject, [&infoMap, portObject] {
            infoMap = portObject->info();
        }, Qt::BlockingQueuedConnection);
        lua_pushqvariant(L, infoMap);
        return 1;
    }
}

int lua_portWriteData(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 2 && lua_gettop(L) != 3)
        luaL_error(L, "unexpected number of arguments");
    // convert arguments
    const char *param1 = luaL_checkstring(L, 1);
    size_t len2;
    const char *param2 = luaL_checklstring(L, 2, &len2);
    const char *param3 = luaL_optstring(L, 3, nullptr);
    // start operation
    const QString portName = QString::fromUtf8(param1);
    if (!g_port->m_portHash.contains(portName)) {
        luaL_error(L, "port '%s' does not exist", portName.toUtf8().constData());
    } else {
        bool status = false;
        auto *portObject = g_port->m_portHash[portName];
        const QByteArray txData(param2, static_cast<qsizetype>(len2));
        if (param3) {
            const QString peerIp = QString::fromUtf8(param3);
            QMetaObject::invokeMethod(portObject, [&status, portObject, txData, peerIp] {
                status = portObject->writeData(txData, peerIp);
            }, Qt::BlockingQueuedConnection);
        } else {
            QMetaObject::invokeMethod(portObject, [&status, portObject, txData] {
                status = portObject->writeData(txData);
            }, Qt::BlockingQueuedConnection);
        }
        if (!status) {
            luaL_error(L, "write data failed");
        }
        return 0;
    }
}

int lua_portWriteText(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 2 && lua_gettop(L) != 3)
        luaL_error(L, "unexpected number of arguments");
    // convert arguments
    const char *param1 = luaL_checkstring(L, 1);
    const char *param2 = luaL_checkstring(L, 2);
    const char *param3 = luaL_optstring(L, 3, nullptr);
    // start operation
    const QString portName = QString::fromUtf8(param1);
    if (!g_port->m_portHash.contains(portName)) {
        luaL_error(L, "port '%s' does not exist", portName.toUtf8().constData());
    } else {
        bool status = false;
        auto *portObject = g_port->m_portHash[portName];
        const QString txText = QString::fromUtf8(param2);
        if (param3) {
            const QString peerIp = QString::fromUtf8(param3);
            QMetaObject::invokeMethod(portObject, [&status, portObject, txText, peerIp] {
                status = portObject->writeText(txText, peerIp);
            }, Qt::BlockingQueuedConnection);
        } else {
            QMetaObject::invokeMethod(portObject, [&status, portObject, txText] {
                status = portObject->writeText(txText);
            }, Qt::BlockingQueuedConnection);
        }
        if (!status) {
            luaL_error(L, "write text failed");
        }
        return 0;
    }
}

int lua_portReadData(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 1 && lua_gettop(L) != 2 && lua_gettop(L) != 3 && lua_gettop(L) != 4)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const char *param1 = luaL_checkstring(L, 1);
    const int param2 = static_cast<int>(luaL_optinteger(L, 2, 0));
    const int param3 = static_cast<int>(luaL_optinteger(L, 3, 0));
    const char *param4 = luaL_optstring(L, 4, nullptr);
    // start operation
    const QString portName = QString::fromUtf8(param1);
    if (!g_port->m_portHash.contains(portName)) {
        luaL_error(L, "port '%s' does not exist", portName.toUtf8().constData());
    } else {
        QByteArray rxData;
        auto *portObject = g_port->m_portHash[portName];
        const int timeout = param2;
        const int length = param3;
        if (param4) {
            const QString peerIp = QString::fromUtf8(param4);
            QMetaObject::invokeMethod(portObject, [&rxData, portObject, timeout, length, peerIp] {
                rxData = portObject->readData(timeout, length, peerIp);
            }, Qt::BlockingQueuedConnection);
        } else {
            QMetaObject::invokeMethod(portObject, [&rxData, portObject, timeout, length] {
                rxData = portObject->readData(timeout, length);
            }, Qt::BlockingQueuedConnection);
        }
        if (timeout != 0 && length != 0 && rxData.isEmpty()) {
            luaL_error(L, "port read data timeout");
            return 0;
        }
        lua_pushlstring(L, rxData.constData(), rxData.size());
        return 1;
    }
}

int lua_portReadText(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 1 && lua_gettop(L) != 2 && lua_gettop(L) != 3 && lua_gettop(L) != 4)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const char *param1 = luaL_checkstring(L, 1);
    const int param2 = static_cast<int>(luaL_optinteger(L, 2, 0));
    const int param3 = static_cast<int>(luaL_optinteger(L, 3, 0));
    const char *param4 = luaL_optstring(L, 4, nullptr);
    // start operation
    const QString portName = QString::fromUtf8(param1);
    if (!g_port->m_portHash.contains(portName)) {
        luaL_error(L, "port '%s' does not exist", portName.toUtf8().constData());
    } else {
        QString rxText;
        auto *portObject = g_port->m_portHash[portName];
        const int timeout = param2;
        const int length = param3;
        if (param4) {
            const QString peerIp = QString::fromUtf8(param4);
            QMetaObject::invokeMethod(portObject, [&rxText, portObject, timeout, length, peerIp] {
                rxText = portObject->readText(timeout, length, peerIp);
            }, Qt::BlockingQueuedConnection);
        } else {
            QMetaObject::invokeMethod(portObject, [&rxText, portObject, timeout, length] {
                rxText = portObject->readText(timeout, length);
            }, Qt::BlockingQueuedConnection);
        }
        if (timeout != 0 && length != 0 && rxText.isEmpty()) {
            luaL_error(L, "port read text timeout");
            return 0;
        }
        if (rxText.contains("\x1E")) {
            const QStringList rxTextList = rxText.split("\x1E");
            lua_createtable(L, rxTextList.size(), 0);
            for (int i = 0; i < rxTextList.size(); ++i) {
                const QString &text = rxTextList.at(i);
                lua_pushstring(L, text.toUtf8().constData());
                lua_rawseti(L, -2, i + 1);
            }
        } else {
            lua_pushstring(L, rxText.toUtf8().constData());
        }
        return 1;
    }
}
