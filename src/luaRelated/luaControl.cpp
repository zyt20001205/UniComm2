#include "luaRelated/luaControl.h"

int lua_leftClick(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 2)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const int param1 = static_cast<int>(luaL_checkinteger(L, 1));
    const int param2 = static_cast<int>(luaL_checkinteger(L, 2));
    // start operation
    const int x = param1;
    const int y = param2;
    SetCursorPos(x, y);
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(2, inputs, sizeof(INPUT));
    return 0;
}

int lua_leftDoubleClick(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 2)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const int param1 = static_cast<int>(luaL_checkinteger(L, 1));
    const int param2 = static_cast<int>(luaL_checkinteger(L, 2));
    // start operation
    const int x = param1;
    const int y = param2;
    SetCursorPos(x, y);
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(2, inputs, sizeof(INPUT));
    QThread::msleep(GetDoubleClickTime() / 2);
    SendInput(2, inputs, sizeof(INPUT));
    return 0;
}

int lua_rightClick(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 2)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const int param1 = static_cast<int>(luaL_checkinteger(L, 1));
    const int param2 = static_cast<int>(luaL_checkinteger(L, 2));
    // start operation
    const int x = param1;
    const int y = param2;
    SetCursorPos(x, y);
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    SendInput(2, inputs, sizeof(INPUT));
    return 0;
}

int lua_rightDoubleClick(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 2)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const int param1 = static_cast<int>(luaL_checkinteger(L, 1));
    const int param2 = static_cast<int>(luaL_checkinteger(L, 2));
    // start operation
    const int x = param1;
    const int y = param2;
    SetCursorPos(x, y);
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    SendInput(2, inputs, sizeof(INPUT));
    QThread::msleep(GetDoubleClickTime() / 2);
    SendInput(2, inputs, sizeof(INPUT));
    return 0;
}

int lua_keyPress(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 1)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const char* param1 = lua_tostring(L, 1);
    // start operation
    const WORD vk = static_cast<unsigned char>(param1[0]);
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = vk;
    inputs[1] = inputs[0];
    inputs[1].ki.dwFlags |= KEYEVENTF_KEYUP;
    SendInput(2, inputs, sizeof(INPUT));
    return 0;
}