#ifndef UNICOMM_VISA_H
#define UNICOMM_VISA_H

#include <QJsonObject>
#include <visatype.h>

#include "basePort.h"
#include "utils/qtUtils.h"

class Visa final : public BasePort {
    Q_OBJECT

public:
    explicit Visa(const QJsonObject &portConfig, QObject *parent = nullptr);

    ~Visa() override;

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

private:
    bool handleWrite(const QByteArray &f_txData);

    QByteArray handleRead(int length, int timeout);

    void handleLog(const QString &mode, const QByteArray &data);

    ViSession m_visa{};
    QJsonObject m_portConfig{};
    RingBuffer m_buffer;
};

#endif //UNICOMM_VISA_H
