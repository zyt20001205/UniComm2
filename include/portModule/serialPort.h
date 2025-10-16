#ifndef UNICOMM_SERIALPORT_H
#define UNICOMM_SERIALPORT_H

#include <QJsonObject>

#include "basePort.h"

class QSerialPort;

class SerialPort final : public BasePort {
    Q_OBJECT

public:
    explicit SerialPort(const QJsonObject &portConfig, QObject *parent = nullptr);

    void reload(const QJsonObject &portConfig) override;

    QHash<QString, QVariant> info() override;

    bool open() override;

    void close() override;

    bool writeText(const QString &txText) override;

    bool writeData(const QByteArray &txData) override;

    QString readText(int timeout, int length) override;

    QByteArray readData(int timeout, int length) override;

signals:
    void connected();

    void disconnected();

    void readyRead();

    void errorOccurred(const QString &error);

private:
    bool handleWrite(const QByteArray &f_txData);

    QByteArray handleRead(int timeout, int length);

    void handleReadyRead();

    void handleError();

    void handleLog(const QString &mode, const QByteArray &data);

    QSerialPort *m_serialPort{};
    // port config
    QString m_portName{};
    int m_baudRate{};
    int m_dataBits{};
    int m_parity{};
    int m_stopBits{};
    QString m_txFormat{};
    QString m_txSuffix{};
    QString m_rxFormat{};
    //
    bool m_syncMode = false;
    qint64 m_bufferSize = 0;
    QByteArray m_rxBuffer{};
};

#endif //UNICOMM_SERIALPORT_H
