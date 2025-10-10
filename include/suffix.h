#ifndef UNICOMM_SUFFIX_H
#define UNICOMM_SUFFIX_H

#include <QByteArray>
#include <QString>

QByteArray crc8Maxim(const QByteArray& data);

QByteArray modbusCRC(const QByteArray& data);

QString modbusLRC(const QString& text);

#endif //UNICOMM_SUFFIX_H
