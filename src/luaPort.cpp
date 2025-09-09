#include "../include/luaPort.h"

int lua_portOpen(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 1)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const int param1 = static_cast<int>(luaL_optinteger(L, 1, -1));
    // start operation
    const int index = param1;
    auto *portObject = g_port->portObject(index);
    bool status;
    QMetaObject::invokeMethod(portObject, [&status, portObject] {
        status = portObject->open();
    }, Qt::BlockingQueuedConnection);
    lua_pushboolean(L, status);
    return 1;
}

int lua_portClose(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 1)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const int param1 = static_cast<int>(luaL_optinteger(L, 1, -1));
    // start operation
    const int index = param1;
    auto *portObject = g_port->portObject(index);
    QMetaObject::invokeMethod(portObject, [portObject] {
        portObject->close();
    }, Qt::BlockingQueuedConnection);
    return 0;
}

int lua_portInfo(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 1)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const int param1 = static_cast<int>(luaL_optinteger(L, 1, -1));
    // start operation
    const int index = param1;
    auto *portObject = g_port->portObject(index);
    QString info;
    QMetaObject::invokeMethod(portObject, [&info, portObject] {
        info = portObject->info();
    }, Qt::BlockingQueuedConnection);
    g_log->logAppend(info, "info");
    return 0;
}

int lua_portWriteText(lua_State *L) {
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
    auto *portObject = g_port->portObject(index);
    if (param3) {
        const QString peerIp = QString::fromUtf8(param3);
        QMetaObject::invokeMethod(portObject, [portObject, txText, peerIp] {
            portObject->writeText(txText, peerIp);
        }, Qt::BlockingQueuedConnection);
    } else {
        QMetaObject::invokeMethod(portObject, [portObject, txText] {
            portObject->writeText(txText);
        }, Qt::BlockingQueuedConnection);
    }
    return 0;
}

int lua_portWriteData(lua_State *L) {
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
    auto *portObject = g_port->portObject(index);
    if (param3) {
        const QString peerIp = QString::fromUtf8(param3);
        QMetaObject::invokeMethod(portObject, [portObject, txData, peerIp] {
            portObject->writeData(txData, peerIp);
        }, Qt::BlockingQueuedConnection);
    } else {
        QMetaObject::invokeMethod(portObject, [portObject, txData] {
            portObject->writeData(txData);
        }, Qt::BlockingQueuedConnection);
    }
    return 0;
}

int lua_portReadText(lua_State *L) {
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
    auto *portObject = g_port->portObject(index);
    QMetaObject::invokeMethod(portObject, [&rxText, portObject, timeout] {
        rxText = portObject->readText(timeout);
    }, Qt::BlockingQueuedConnection);
    if (rxText == "timeout") {
        luaL_error(L, "port read data timeout");
        return 0;
    }
    lua_pushstring(L, rxText.toUtf8().constData());
    return 1;
}

int lua_portReadData(lua_State *L) {
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
    auto *portObject = g_port->portObject(index);
    QMetaObject::invokeMethod(portObject, [&rxData, portObject, timeout] {
        rxData = portObject->readData(timeout);
    }, Qt::BlockingQueuedConnection);
    if (rxData == "timeout") {
        luaL_error(L, "port read data timeout");
        return 0;
    }
    lua_pushlstring(L, rxData.constData(), rxData.size());
    return 1;
}