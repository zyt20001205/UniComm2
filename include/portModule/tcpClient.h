#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QJsonObject>

#include "basePort.h"

class QTcpSocket;

class TcpClient final : public BasePort {
    Q_OBJECT

public:
    explicit TcpClient(const QJsonObject &portConfig, QObject *parent = nullptr);

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
    void handleConnected();

    void handleDisconnected();

    void handleError();

    void handleWrite(const QByteArray &f_txData);

    QByteArray handleRead(int timeout, int length);

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
    QByteArray m_rxBuffer{};
};

#endif //TCPCLIENT_H