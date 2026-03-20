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

private:
    [[nodiscard]] bool handleWrite(const QByteArray &f_txData);

    [[nodiscard]] QByteArray handleRead(int length, int timeout);

    void handleLog(const QString &mode, const QByteArray &data);

    ViSession m_visa{};
    QJsonObject m_portConfig{};
    RingBuffer m_buffer;
};

#endif //UNICOMM_VISA_H
