#ifndef UNICOMM_QTUTILS_H
#define UNICOMM_QTUTILS_H

#include <QIcon>

class QUrl;

QByteArray fileHashCalc(const QString &fileInfo);

QByteArray fileHashCalc(const QUrl &fileInfo);

QByteArray stringHashCalc(const QString &content);

QIcon SvgIcon(const QString &svgPath, const QColor &color, const QSize &size = QSize(24, 24));

#endif //UNICOMM_QTUTILS_H