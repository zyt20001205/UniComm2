#ifndef UNICOMM_LUAMISCELLANEOUS_H
#define UNICOMM_LUAMISCELLANEOUS_H

#include <QApplication>
#include <QString>
#include <QtTextToSpeech/QTextToSpeech>
#include <QThread>
#include <lua.hpp>

int lua_exec(lua_State *L);

int lua_stop(lua_State *L);

int lua_wait(lua_State *L);

int lua_input(lua_State *L);

int lua_print(lua_State *L);

int lua_sleep(lua_State *L);

int lua_speak(lua_State *L);

#endif //UNICOMM_LUAMISCELLANEOUS_H
