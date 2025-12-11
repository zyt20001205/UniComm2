#ifndef UNICOMM_UDPSOCKET_H
#define UNICOMM_UDPSOCKET_H

#include <QJsonObject>

#include "basePort.h"

class QUdpSocket;

class UdpSocket final : public BasePort {
    Q_OBJECT

public:
    explicit UdpSocket(const QJsonObject &portConfig, QObject *parent = nullptr);

    void reload(const QJsonObject &portConfig) override;

    std::unordered_map<std::string, std::string> info() override;

    bool open() override;

    void close() override;

    bool write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) override;

    QByteArray read(int timeout, int length, const QString &rxFormat) override;

signals:
    void connected();

    void disconnected();

    void readyRead();

    void errorOccurred(const QString &error);

private:
    void handleReadyRead();

    void handleError();

    bool handleWrite(const QByteArray &f_txData);

    QByteArray handleRead(int timeout, int length);

    void handleLog(const QString &mode, const QByteArray &data);

    QUdpSocket *m_udpSocket{};
    // port config
    QString m_portName{};
    QString m_udpSocketLocalAddress{};
    int m_udpSocketLocalPort{};
    QString m_udpSocketRemoteAddress{};
    int m_udpSocketRemotePort{};
    QString m_txFormat{};
    QString m_txSuffix{};
    QString m_rxFormat{};
    //
    bool m_syncMode = false;
    qint64 m_bufferSize = 0;
    QByteArray m_rxBuffer{};
};

#endif //UNICOMM_UDPSOCKET_H
