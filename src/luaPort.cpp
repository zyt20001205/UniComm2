#include "../include/luaPort.h"

#include "../include/globals.h"
#include "../include/log.h"
#include "../include/port.h"

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
    QMetaObject::invokeMethod(portObject, [portObject, &status] {
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
    QHash<QString, QVariant> infoHash;
    QMetaObject::invokeMethod(portObject, [&infoHash, portObject] {
        infoHash = portObject->info();
    }, Qt::BlockingQueuedConnection);
    lua_createtable(L, 0, static_cast<int>(infoHash.size()));
    for (auto it = infoHash.constBegin(); it != infoHash.constEnd(); ++it) {
        const QString &key = it.key();
        const QVariant &value = it.value();
        lua_pushstring(L, key.toUtf8().constData());
        switch (value.typeId()) {
            case QMetaType::Bool:
                lua_pushboolean(L, value.toBool());
                break;
            case QMetaType::QString:
                lua_pushstring(L, value.toString().toUtf8().constData());
                break;
            default:
                break;
        }
        lua_settable(L, -3);
    }
    return 1;
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
    if (lua_gettop(L) > 3)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const int param1 = static_cast<int>(luaL_optinteger(L, 1, -1));
    const int param2 = static_cast<int>(luaL_optinteger(L, 2, 0));
    const int param3 = static_cast<int>(luaL_optinteger(L, 3, 0));
    // start operation
    const int index = param1;
    const int timeout = param2;
    const int length = param3;
    QString rxText;
    auto *portObject = g_port->portObject(index);
    QMetaObject::invokeMethod(portObject, [&rxText, portObject, timeout, length] {
        rxText = portObject->readText(timeout, length);
    }, Qt::BlockingQueuedConnection);
    if (rxText == "timeout") {
        luaL_error(L, "port read data timeout");
        return 0;
    }
    if (rxText.contains("\x1E")) {
        const QStringList rxTextList = rxText.split("\x1E");
        lua_createtable(L, rxTextList.size(), 0);
        for (int i = 0; i < rxTextList.size(); ++i) {
            const QString& text = rxTextList.at(i);
            lua_pushstring(L, text.toUtf8().constData());
            lua_rawseti(L, -2, i + 1);
        }
    } else {
        lua_pushstring(L, rxText.toUtf8().constData());
    }
    return 1;
}

int lua_portReadData(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 3)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const int param1 = static_cast<int>(luaL_optinteger(L, 1, -1));
    const int param2 = static_cast<int>(luaL_optinteger(L, 2, 0));
    const int param3 = static_cast<int>(luaL_optinteger(L, 3, 0));
    // start operation
    const int index = param1;
    const int timeout = param2;
    const int length = param3;
    QByteArray rxData;
    auto *portObject = g_port->portObject(index);
    QMetaObject::invokeMethod(portObject, [&rxData, portObject, timeout, length] {
        rxData = portObject->readData(timeout, length);
    }, Qt::BlockingQueuedConnection);
    if (rxData == "timeout") {
        luaL_error(L, "port read data timeout");
        return 0;
    }
    lua_pushlstring(L, rxData.constData(), rxData.size());
    return 1;
}