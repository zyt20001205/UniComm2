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
    void handleError();

    bool handleWrite(const QByteArray &f_txData);

    QByteArray handleRead(int timeout, int length);

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
    QByteArray m_rxBuffer{};
};

#endif //UNICOMM_UDPSOCKET_H
