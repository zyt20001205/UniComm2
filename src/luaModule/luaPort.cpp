#include "luaModule/luaPort.h"

#include "globals.h"
#include "portModule/basePort.h"
#include "portModule/portModule.h"
#include "utils/luaUtils.h"

int lua_portList(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 0)
        luaL_error(L, "unexpected number of arguments");
    // convert arguments
    // start operation
    QVariantList variantList{};
    QMetaObject::invokeMethod(g_mainWindow, [&variantList] {
        variantList = g_port->portList();
    }, Qt::BlockingQueuedConnection);
    lua_pushqvariant(L, variantList);
    return 1;
}

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
        QVariantMap infoMap{};
        auto *portObject = g_port->m_portHash[portName];
        QMetaObject::invokeMethod(portObject, [&infoMap, portObject] {
            infoMap = portObject->info();
        }, Qt::BlockingQueuedConnection);
        lua_pushqvariant(L, infoMap);
        return 1;
    }
}

int lua_portWrite(lua_State *L) {
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
                status = portObject->write(txData, peerIp, "", "");
            }, Qt::BlockingQueuedConnection);
        } else {
            QMetaObject::invokeMethod(portObject, [&status, portObject, txData] {
                status = portObject->write(txData, "", "");
            }, Qt::BlockingQueuedConnection);
        }
        if (!status) {
            luaL_error(L, "port write failed");
        }
        return 0;
    }
}

int lua_portRead(lua_State *L) {
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
                rxData = portObject->read(timeout, length, peerIp, "");
            }, Qt::BlockingQueuedConnection);
        } else {
            QMetaObject::invokeMethod(portObject, [&rxData, portObject, timeout, length] {
                rxData = portObject->read(timeout, length, "");
            }, Qt::BlockingQueuedConnection);
        }
        if (timeout != 0 && length != 0 && rxData.isEmpty()) {
            luaL_error(L, "port read timeout");
            return 0;
        }
        lua_pushlstring(L, rxData.constData(), rxData.size());
        return 1;
    }
}
