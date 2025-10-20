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

    void reload(const QJsonObject &portConfig) override;

    bool open() override;

    void close() override;

    QVariantMap info() override;

    QString readText(int timeout, int length) override;

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