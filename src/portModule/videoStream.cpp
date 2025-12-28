#include "portModule/videoStream.h"

#include <QEventLoop>
#include <QImageCapture>
#include <QJsonArray>
#include <QMediaCaptureSession>
#include <QMediaDevices>

#include "globals.h"
#include "utils/cvUtils.h"

// VideoStream public
VideoStream::VideoStream(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portConfig(portConfig) {
}

int VideoStream::type() {
    return VIDEOSTREAM;
}

QJsonObject VideoStream::config() {
    return m_portConfig;
}

bool VideoStream::open() {
    emit refreshPort(m_portConfig["portName"].toString(), true);
    emit appendLog(QString("%1 opened").arg(m_portConfig["portName"].toString()), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 opened").arg(timestamp, m_portConfig["portName"].toString());
    return true;
}

void VideoStream::close() {
    emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(QString("%1 closed").arg(m_portConfig["portName"].toString()), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 closed").arg(timestamp, m_portConfig["portName"].toString());
}

std::unordered_map<std::string, std::string> VideoStream::info() {
    return {};
}

QByteArray VideoStream::read(const int timeout, const int length, const QString &rxFormat) {
    // // find videoStream
    // m_videoStream = QVideoStreamDevice();
    // for (const QVideoStreamDevice &videoStream: QMediaDevices::videoInputs()) {
    //     if (videoStream.description() == m_portConfig["portName"].toString()) {
    //         m_videoStream = videoStream;
    //         break;
    //     }
    // }
    // if (m_videoStream.isNull()) return "videoStream not found";;
    // QPixmap shot{};
    // const auto videoStream = new QVideoStream(m_videoStream, this);
    // QMediaCaptureSession captureSession;
    // captureSession.setVideoStream(videoStream);
    // QImageCapture imageCapture;
    // captureSession.setImageCapture(&imageCapture);
    // QEventLoop loop;
    // connect(&imageCapture, &QImageCapture::imageCaptured, this, [&shot, &loop](int, const QImage &img) {
    //     shot = QPixmap::fromImage(img);
    //     loop.quit();
    // });
    // videoStream->start();
    // imageCapture.capture();
    // loop.exec();
    // videoStream->stop();
    // delete videoStream;
    // QStringList resultList{};
    // for (const QJsonValue &value: m_portConfig["roi"].toArray()) {
    //     QJsonArray roi = value.toArray();
    //     const int x = roi[0].toInt();
    //     const int y = roi[1].toInt();
    //     const int width = roi[2].toInt();
    //     const int height = roi[3].toInt();
    //     const auto rect = QRect(x, y, width, height);
    //     const QPixmap cropped = shot.copy(rect);
    //     const QString text = ocr(cropped, "eng", m_portConfig["whitelist"].toString());
    //     resultList.append(text);
    // }
    // return resultList.join("\x1E").toUtf8();
}
