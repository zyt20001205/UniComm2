#ifndef UNICOMM_VIDEOSTREAM_H
#define UNICOMM_VIDEOSTREAM_H

#include <QJsonObject>

#include "portModule/basePort.h"

class VideoStream final : public BasePort {
    Q_OBJECT

public:
    explicit VideoStream(const QJsonObject &portConfig, QObject *parent = nullptr);

    int type() override;

    QJsonObject config() override;

    bool open() override;

    void close() override;

    std::unordered_map<std::string, std::string> info() override;

    QByteArray read(int timeout, int length, const QString &rxFormat) override;

private:
    // port config
    QJsonObject m_portConfig{};
};

#endif //UNICOMM_VIDEOSTREAM_H
