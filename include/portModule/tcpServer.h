#ifndef UNICOMM_TCPSERVER_H
#define UNICOMM_TCPSERVER_H

#include <QJsonObject>

#include "basePort.h"

class QTcpServer;
class QTcpSocket;

class TcpServer final : public BasePort {
    Q_OBJECT

public:
    explicit TcpServer(const QJsonObject &portConfig, QObject *parent = nullptr);

    void reload(const QJsonObject &portConfig) override;

    bool open() override;

    void close() override;

    QHash<QString, QVariant> info() override;

    bool writeText(const QString &txText) override;

    bool writeText(const QString &txText, const QString &peerIp) override;

    bool writeData(const QByteArray &txData) override;

    bool writeData(const QByteArray &txData, const QString &peerIp) override;

    QString readText(int timeout, int length) override;

    QString readText(int timeout, int length, const QString &peerIp) override;

    QByteArray readData(int timeout, int length) override;

    QByteArray readData(int timeout, int length, const QString &peerIp) override;

signals:
    void newConnection();

    void acceptError(const QString &error);

    void disconnected(qintptr socketDescriptor);

    void readyRead();

    void errorOccurred(const QString &error);

private:
    void handleNewConnection();

    void handleServerError();

    void handleConnected(QTcpSocket *tcpServerPeer);

    void handleDisconnected(QTcpSocket *tcpServerPeer);

    void handleReadyRead(QTcpSocket *tcpServerPeer);

    void handleError(QTcpSocket *tcpServerPeer);

    bool handleWrite(const QByteArray &f_txData, const QString &peerIp = QString());

    QByteArray handleRead(int timeout, int length, QTcpSocket *tcpServerPeer);

    void handleLog(const QString &mode, const QByteArray &data, const QTcpSocket *tcpServerPeer);

    QTcpServer *m_tcpServer{};
    // port config
    QString m_portName{};
    QString m_tcpServerLocalAddress{};
    int m_tcpServerLocalPort{};
    QHash<QString, QTcpSocket *> m_tcpServerPeerHash{};
    QString m_txFormat{};
    QString m_txSuffix{};
    QString m_rxFormat{};
    //
    bool m_syncMode = false;
    qint64 m_bufferSize = 0;
    QByteArray m_rxBuffer;
};

#endif //UNICOMM_TCPSERVER_H
