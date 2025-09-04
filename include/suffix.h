#ifndef SUFFIX_H
#define SUFFIX_H

#include <QByteArray>
#include <QString>

QByteArray crc8Maxim(const QByteArray& data);

QByteArray modbusCRC(const QByteArray& data);

QString modbusLRC(const QString& text);

#endif //SUFFIX_H
