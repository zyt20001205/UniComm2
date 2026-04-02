#ifndef UNICOMM_TCPSERVER_H
#define UNICOMM_TCPSERVER_H

#include <QJsonObject>

#include "basePort.h"

class RingBuffer;
class QTcpServer;
class QTcpSocket;

class TcpServer final : public BasePort {
    Q_OBJECT

public:
    explicit TcpServer(const QJsonObject &portConfig, QObject *parent = nullptr);

    ~TcpServer() override;

    [[nodiscard]] int type() override;

    [[nodiscard]] QJsonObject config() override;

    [[nodiscard]] bool open() override;

    void close() override;

    void clear() override;

    [[nodiscard]] QVariantHash info() override;

    [[nodiscard]] bool write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) override;

    [[nodiscard]] bool write(const QByteArray &txData, const QString &peerIp, const QString &txFormat, const QString &txSuffix) override;

    [[nodiscard]] QByteArray read(int length, int timeout, const QString &rxFormat) override;

    [[nodiscard]] QByteArray read(int length, int timeout, const QString &peerIp, const QString &rxFormat) override;

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

    [[nodiscard]] bool handleWrite(const QByteArray &f_txData, const QString &peerIp = QString());

    [[nodiscard]] QByteArray handleRead(int length, int timeout, const QString &peerIp);

    void handleLog(int type, const QByteArray &data, const QTcpSocket *tcpServerPeer);

    QTcpServer *m_tcpServer{};
    QJsonObject m_portConfig{};
    QHash<QString, QTcpSocket *> m_peerHash{};
    QHash<QString, RingBuffer *> m_bufferHash{};
};

#endif //UNICOMM_TCPSERVER_H
