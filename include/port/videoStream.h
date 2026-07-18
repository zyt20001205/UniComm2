#ifndef UNICOMM_VIDEOSTREAM_H
#define UNICOMM_VIDEOSTREAM_H

#include <QJsonObject>

#include "port/basePort.h"
#include "port/module/imageProcess.h"

class QCamera;
class QEventLoop;
class QImageCapture;
class QMediaCaptureSession;
class QScreenCapture;
class QVideoSink;

class VideoStream final : public BasePort {
    Q_OBJECT

public:
    explicit VideoStream(const QJsonObject &portConfig, QObject *parent = nullptr);

    ~VideoStream() override;

    [[nodiscard]] int type() override;

    [[nodiscard]] QJsonObject config() override;

    [[nodiscard]] bool open() override;

    void close() override;

    void clear() override;

    [[nodiscard]] QVariantHash info() override;

    [[nodiscard]] bool write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) override;

    [[nodiscard]] QByteArray read(int length, int timeout, const QString &rxFormat) override;

private:
    QMediaCaptureSession *m_mediaCaptureSession{};
    QVideoSink *m_videoSink{};
    QScreenCapture *m_screenCapture{};
    QCamera *m_cameraCapture{};
    // port config
    QJsonObject m_portConfig{};
    ImageProcess m_imageProcess{};
};

#endif //UNICOMM_VIDEOSTREAM_H
