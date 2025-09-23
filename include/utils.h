#ifndef UTILS_H
#define UTILS_H

#include <QDebug>
#include <QPixmap>
#include <QString>
#include <baseapi.h>
#include <lua.hpp>

QString lua_toqstring(lua_State *L, int idx);

void lua_pushqstring(lua_State *L, int idx, const QString &value);

QString ocr(const QPixmap &pixmap, const QString &charset);

#endif //UTILS_H
