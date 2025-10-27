#ifndef UNICOMM_TCPCLIENT_H
#define UNICOMM_TCPCLIENT_H

#include <QJsonObject>

#include "basePort.h"

class QTcpSocket;

class TcpClient final : public BasePort {
    Q_OBJECT

public:
    explicit TcpClient(const QJsonObject &portConfig, QObject *parent = nullptr);

    void reload(const QJsonObject &portConfig) override;

    QVariantMap info() override;

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
    void handleConnected();

    void handleDisconnected();

    void handleReadyRead();

    void handleError();

    bool handleWrite(const QByteArray &f_txData);

    QByteArray handleRead(int timeout, int length);

    void handleLog(const QString &mode, const QByteArray &data);

    QTcpSocket *m_tcpClient{};
    // port config
    QString m_portName{};
    QString m_tcpClientRemoteAddress{};
    int m_tcpClientRemotePort{};
    QString m_tcpClientLocalAddress{};
    int m_tcpClientLocalPort{};
    QString m_txFormat{};
    QString m_txSuffix{};
    QString m_rxFormat{};
    //
    bool m_syncMode = false;
    qint64 m_bufferSize = 0;
    QByteArray m_rxBuffer{};
};

#endif //UNICOMM_TCPCLIENT_H
