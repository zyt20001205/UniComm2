#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include <QJsonObject>
#include <QUdpSocket>

#include "basePort.h"

class UdpSocket final : public BasePort {
    Q_OBJECT

public:
    explicit UdpSocket(const QJsonObject &portConfig, QObject *parent = nullptr);

    void reload(const QJsonObject &portConfig) override;

    QHash<QString, QVariant> info() override;

    bool open() override;

    void close() override;

    void writeText(const QString &txText) override;

    void writeData(const QByteArray &txData) override;

    QString readText(int timeout, int length) override;

    QByteArray readData(int timeout, int length) override;

signals:
    void connected();

    void disconnected();

    void readyRead();

    void errorOccurred(const QString &error);

private:
    void handleError();

    void handleWrite(const QByteArray &f_txData);

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

#endif //UDPSOCKET_H
