#ifndef UNICOMM_UDPSOCKET_H
#define UNICOMM_UDPSOCKET_H

#include <QJsonObject>

#include "basePort.h"

class QUdpSocket;

class UdpSocket final : public BasePort {
    Q_OBJECT

public:
    explicit UdpSocket(const QJsonObject &portConfig, QObject *parent = nullptr);

    ~UdpSocket() override;

    int type() override;

    QJsonObject config() override;

    std::unordered_map<std::string, std::string> info() override;

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
    void handleReadyRead();

    void handleError();

    bool handleWrite(const QByteArray &f_txData);

    QByteArray handleRead(int timeout, int length);

    void handleLog(const QString &mode, const QByteArray &data);

    QUdpSocket *m_udpSocket{};
    // port config
    QJsonObject m_portConfig{};
    //
    bool m_syncMode = false;
    qint64 m_bufferSize = 0;
    QByteArray m_rxBuffer{};
};

#endif //UNICOMM_UDPSOCKET_H
