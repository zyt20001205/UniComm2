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

    int type() override;

    QJsonObject config() override;

    void reload(const QJsonObject &portConfig) override;

    bool open() override;

    void close() override;

    std::unordered_map<std::string, std::string> info() override;

    bool write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) override;

    bool write(const QByteArray &txData, const QString &peerIp, const QString &txFormat, const QString &txSuffix) override;

    QByteArray read(int timeout, int length, const QString &rxFormat) override;

    QByteArray read(int timeout, int length, const QString &peerIp, const QString &rxFormat) override;

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
    QJsonObject m_portConfig{};
    QHash<QString, QTcpSocket *> m_tcpServerPeerHash{};
    //
    bool m_syncMode = false;
    qint64 m_bufferSize = 0;
    QByteArray m_rxBuffer;
};

#endif //UNICOMM_TCPSERVER_H
