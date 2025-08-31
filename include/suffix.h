#ifndef SUFFIX_H
#define SUFFIX_H

#include <QByteArray>

QByteArray crc8Maxim(const QByteArray& data);

QByteArray crc16Modbus(const QByteArray& data);

#endif //SUFFIX_H
