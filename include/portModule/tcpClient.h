#ifndef UNICOMM_TCPCLIENT_H
#define UNICOMM_TCPCLIENT_H

#include <QJsonObject>

#include "basePort.h"
#include "utils/ringBuffer.h"

class QTcpSocket;

class TcpClient final : public BasePort {
    Q_OBJECT

public:
    explicit TcpClient(const QJsonObject &portConfig, QObject *parent = nullptr);

    ~TcpClient() override;

    [[nodiscard]] int type() override;

    [[nodiscard]] QJsonObject config() override;

    [[nodiscard]] QVariantHash info() override;

    [[nodiscard]] bool open() override;

    void close() override;

    void clear() override;

    [[nodiscard]] bool write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) override;

    [[nodiscard]] QByteArray read(int length, int timeout, const QString &rxFormat) override;

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

    [[nodiscard]] bool handleWrite(const QByteArray &f_txData);

    [[nodiscard]] QByteArray handleRead(int length, int timeout);

    void handleLog(const QString &mode, const QByteArray &data);

    QTcpSocket *m_tcpClient{};
    QJsonObject m_portConfig{};
    QString m_tcpClientLocalHost{};
    int m_tcpClientLocalPort{};
    RingBuffer m_buffer;

};

#endif //UNICOMM_TCPCLIENT_H
