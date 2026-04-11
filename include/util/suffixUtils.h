#ifndef UNICOMM_SUFFIX_H
#define UNICOMM_SUFFIX_H

#include <QString>

QByteArray modbusCRC(const QByteArray& data);

QByteArray modbusLRC(const QByteArray& data);

#endif //UNICOMM_SUFFIX_H
