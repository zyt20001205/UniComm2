#ifndef UNICOMM_QTUTILS_H
#define UNICOMM_QTUTILS_H

#include <QString>

class QUrl;

QByteArray fileHashCalc(const QString &fileInfo);

QByteArray fileHashCalc(const QUrl &fileInfo);

QString md2plain(const QString &markdown);

#endif //UNICOMM_QTUTILS_H
