#ifndef UNICOMM_CAMERA_H
#define UNICOMM_CAMERA_H

#include <QCameraDevice>
#include <QJsonArray>
#include <QJsonObject>

#include "portModule/basePort.h"

class Camera final : public BasePort {
    Q_OBJECT

public:
    explicit Camera(const QJsonObject &portConfig, QObject *parent = nullptr);

    int type() override;

    QJsonObject config() override;

    bool open() override;

    void close() override;

    std::unordered_map<std::string, std::string> info() override;

    QByteArray read(int timeout, int length, const QString &rxFormat) override;

private:
    QCameraDevice m_camera{};
    // port config
    QString m_portName{};
    QString m_charset{};
    QJsonObject m_process{};
    QJsonArray m_areaList{};
    //
    bool m_showPreview = false;
};

#endif //UNICOMM_CAMERA_H