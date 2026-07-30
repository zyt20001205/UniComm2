#ifndef UNICOMM_SSLSERVER_H
#define UNICOMM_SSLSERVER_H

#include <QJsonObject>

#include "basePort.h"

class RingBuffer;
class QSslServer;
class QSslSocket;

class SslServer final : public BasePort {
    Q_OBJECT

public:
    explicit SslServer(const QJsonObject &portConfig, QObject *parent = nullptr);

    ~SslServer() override;

    [[nodiscard]] int type() override;

    [[nodiscard]] QJsonObject config() override;

    [[nodiscard]] QVariantHash info() override;

    [[nodiscard]] bool open() override;

    void close() override;

    void disconnectPeer(const QString &peerIp);

    void clear() override;

    void monitor(bool enabled) override;

    [[nodiscard]] bool write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) override;

    [[nodiscard]] bool write(const QByteArray &txData, const QString &peerIp, const QString &txFormat, const QString &txSuffix) override;

    [[nodiscard]] QByteArray read(int length, int timeout, const QString &rxFormat) override;

    [[nodiscard]] QByteArray read(int length, int timeout, const QString &peerIp, const QString &rxFormat) override;

    [[nodiscard]] QByteArray readUntil(const QByteArray &text, int timeout, const QString &rxFormat) override;

    [[nodiscard]] QByteArray readUntil(const QByteArray &text, int timeout, const QString &peerIp, const QString &rxFormat) override;

signals:
    void connected(const QString &peerIp);

    void disconnected(const QString &peerIp);

    void readyRead(const QString &peerIp);

private:
    [[nodiscard]] bool configureSsl();

    void handleNewConnection();

    void handleServerError();

    void handleConnected(QSslSocket *sslServerPeer);

    void handleDisconnected(QSslSocket *sslServerPeer);

    void handleReadyRead(QSslSocket *sslServerPeer);

    void handleError(const QSslSocket *sslServerPeer);

    [[nodiscard]] bool handleWrite(const QByteArray &f_txData, const QString &peerIp = QString());

    [[nodiscard]] QByteArray handleRead(int length, int timeout, const QString &peerIp);

    [[nodiscard]] QByteArray handleReadUntil(const QByteArray &text, int timeout, const QString &peerIp);

    void handleLog(int type, const QByteArray &data, const QSslSocket *sslServerPeer);

    QSslServer *m_sslServer{};
    QJsonObject m_portConfig{};
    QHash<QString, QSslSocket *> m_peerHash{};
    QHash<QString, RingBuffer *> m_bufferHash{};
};

#endif //UNICOMM_SSLSERVER_H
