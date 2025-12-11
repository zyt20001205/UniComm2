#ifndef UNICOMM_VISA_H
#define UNICOMM_VISA_H

#include <QJsonObject>
#include "visatype.h"

#include "basePort.h"

class Visa final : public BasePort {
    Q_OBJECT

public:
    explicit Visa(const QJsonObject &portConfig, QObject *parent = nullptr);

    int type() override;

    void reload(const QJsonObject &portConfig) override;

    std::unordered_map<std::string, std::string> info() override;

    bool open() override;

    void close() override;

    bool write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) override;

    QByteArray read(int timeout, int length, const QString &rxFormat) override;

signals:
    void connected();

    void disconnected();

private:
    bool handleWrite(const QByteArray &f_txData);

    QByteArray handleRead(int timeout, int length);

    void handleLog(const QString &mode, const QByteArray &data);

    ViSession m_visa{};
    // port config
    QString m_portName{};
    QString m_txFormat{};
    QString m_txSuffix{};
    QString m_rxFormat{};
    //
    bool m_syncMode = false;
    qint64 m_bufferSize = 0;
    QByteArray m_rxBuffer{};
};

#endif //UNICOMM_VISA_H
