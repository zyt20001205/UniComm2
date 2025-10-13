#ifndef UNICOMM_UTILS_H
#define UNICOMM_UTILS_H

#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QPixmap>
#include <QString>
#include <baseapi.h>
#include <lua.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

QByteArray fileHashCalc(const QString &fileInfo);

QByteArray fileHashCalc(const QUrl &fileInfo);

QByteArray stringHashCalc(const QString &content);

QString lua_toqstring(lua_State *L, int idx);

void lua_pushqstring(lua_State *L, int idx, const QString &value);

QString ocr(const QPixmap &pixmap, const QString &charset);

QPixmap processGaussianBlur(const QPixmap &pixmap, int size);

QPixmap processThreshold(const QPixmap &pixmap, int thresh, int type);

#endif //UNICOMM_UTILS_H
