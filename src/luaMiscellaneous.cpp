#include "../include/luaMiscellaneous.h"

QString lua_toqstring(lua_State *L, const int idx) {
    switch (lua_type(L, idx)) {
        case LUA_TNIL:
            return QString("\\");
        case LUA_TBOOLEAN:
            return lua_toboolean(L, idx) ? "true" : "false";
        case LUA_TLIGHTUSERDATA:
            return QString("WIP");
        case LUA_TNUMBER:
            return lua_tostring(L, idx);
        case LUA_TSTRING:
            return lua_tostring(L, idx);
        case LUA_TTABLE:
            return QString("{...}");
        case LUA_TFUNCTION:
            return QString("\\");
        case LUA_TUSERDATA:
            return QString("WIP");
        case LUA_TTHREAD:
            return QString("\\");
        default:
            return QString("?");
    }
}

void lua_pushqstring(lua_State *L, const int idx, const QString &value) {
    switch (lua_type(L, idx)) {
        case LUA_TBOOLEAN: {
            if (value == "true" || value == "1") {
                lua_pushboolean(L, 1);
            } else {
                lua_pushboolean(L, 0);
            }
        }
        break;
        case LUA_TNUMBER: {
            if (value.contains(".") || value.contains("e")) {
                lua_pushnumber(L, value.toDouble());
            } else {
                lua_pushinteger(L, value.toInt());
            }
        }
        break;
        case LUA_TSTRING: {
            lua_pushstring(L, value.toUtf8().constData());
        }
        break;
        default: break;
    }
}

int lua_print(lua_State *L) {
    const int n = lua_gettop(L);
    QString message;
    for (int i = 1; i <= n; i++) {
        size_t len = 0;
        const char *s = luaL_tolstring(L, i, &len);
        if (i > 1) message += " ";
        if (s) message += QString::fromUtf8(s, static_cast<int>(len));
        lua_pop(L, 1);
    }
    if (!message.isEmpty()) g_log->logAppend(message, "info");
    return 0;
}

int lua_sleep(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 1)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const int param = static_cast<int>(luaL_checkinteger(L, 1));
    // start operation
    const int millisecond = param;
    QThread::msleep(millisecond);
    return 0;
}

int lua_input(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 0)
        luaL_error(L, "unexpected number of arguments");
    // start operation
    bool ok = false;
    QString input;
    QMetaObject::invokeMethod(qApp, [&ok, &input] {
        QWidget *parent = QApplication::activeWindow();
        input = QInputDialog::getText(parent, "Input Dialog", "input:", QLineEdit::Normal, QString(), &ok);
    }, Qt::BlockingQueuedConnection);
    if (!ok)
        return 0;
    lua_pushstring(L, input.toUtf8().constData());
    return 1;
}
