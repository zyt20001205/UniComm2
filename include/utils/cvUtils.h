#ifndef UNICOMM_UTILS_H
#define UNICOMM_UTILS_H

#include <QPixmap>
#include <QString>

QString ocr(const QPixmap &pixmap, const QString &charset, const QString &whitelist);

QPixmap processGaussianBlur(const QPixmap &pixmap, int size);

QPixmap processThreshold(const QPixmap &pixmap, int thresh, int type);

#endif //UNICOMM_UTILS_H
