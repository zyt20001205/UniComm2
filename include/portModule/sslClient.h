#ifndef UNICOMM_SSLCLIENT_H
#define UNICOMM_SSLCLIENT_H

#include <QJsonObject>

#include "basePort.h"
#include "utils/qtUtils.h"

class QSslSocket;

class SslClient final : public BasePort {
    Q_OBJECT

public:
    explicit SslClient(const QJsonObject &portConfig, QObject *parent = nullptr);

    ~SslClient() override;

    int type() override;

    QJsonObject config() override;

    std::unordered_map<std::string, std::string> info() override;

    bool open() override;

    void close() override;

    void clear() override;

    bool write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) override;

    QByteArray read(int length, int timeout, const QString &rxFormat) override;

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

    QByteArray handleRead(int length, int timeout);

    void handleLog(const QString &mode, const QByteArray &data);

    QSslSocket *m_sslClient{};
    QJsonObject m_portConfig{};
    QString m_sslClientLocalHost{};
    int m_sslClientLocalPort{};
    RingBuffer m_buffer;
};

#endif //UNICOMM_SSLCLIENT_H
