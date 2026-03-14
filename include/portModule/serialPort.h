#ifndef UNICOMM_SERIALPORT_H
#define UNICOMM_SERIALPORT_H

#include <QJsonObject>

#include "basePort.h"
#include "utils/qtUtils.h"

class QSerialPort;

class SerialPort final : public BasePort {
    Q_OBJECT

public:
    explicit SerialPort(const QJsonObject &portConfig, QObject *parent = nullptr);

    ~SerialPort() override;

    int type() override;

    QJsonObject config() override;

    std::unordered_map<std::string, std::string> info() override;

    bool open() override;

    void close() override;

    void clear() override;

    bool write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) override;

    QByteArray read(int length, int timeout, const QString &rxFormat) override;

signals:
    void connected();

    void disconnected();

    void readyRead();

    void errorOccurred(const QString &error);

private:
    void handleReadyRead();

    void handleError();

    bool handleWrite(const QByteArray &f_txData);

    QByteArray handleRead(int length, int timeout);

    void handleLog(const QString &mode, const QByteArray &data);

    QSerialPort *m_serialPort{};
    QJsonObject m_portConfig{};
    RingBuffer m_buffer;
};

#endif //UNICOMM_SERIALPORT_H
