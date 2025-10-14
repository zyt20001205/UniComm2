#ifndef UNICOMM_QTUTILS_H
#define UNICOMM_QTUTILS_H

#include <QByteArray>

class QUrl;

QByteArray fileHashCalc(const QString &fileInfo);

QByteArray fileHashCalc(const QUrl &fileInfo);

QByteArray stringHashCalc(const QString &content);

#endif //UNICOMM_QTUTILS_H