#ifndef UNICOMM_VIDEOSTREAM_H
#define UNICOMM_VIDEOSTREAM_H

#include <QJsonObject>
#include <tesseract/baseapi.h>

#include "portModule/basePort.h"

class QCamera;
class QMediaCaptureSession;
class QScreenCapture;

class VideoStream final : public BasePort {
    Q_OBJECT

public:
    explicit VideoStream(const QJsonObject &portConfig, QObject *parent = nullptr);

    ~VideoStream() override;

    int type() override;

    QJsonObject config() override;

    bool open() override;

    void close() override;

    std::unordered_map<std::string, std::string> info() override;

    QByteArray read(int timeout, int length, const QString &rxFormat) override;

private:
    QMediaCaptureSession *m_mediaCaptureSession{};
    QScreenCapture *m_screenCapture{};
    QCamera *m_cameraCapture{};
    tesseract::TessBaseAPI *m_ocrEngine{};
    // port config
    QJsonObject m_portConfig{};
};

#endif //UNICOMM_VIDEOSTREAM_H
