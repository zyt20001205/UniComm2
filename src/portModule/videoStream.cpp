#include "portModule/videoStream.h"

#include <QCamera>
#include <QEventLoop>
#include <QImageCapture>
#include <QJsonArray>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QScreenCapture>

#include "globals.h"
#include "utils/cvUtils.h"

// VideoStream public
VideoStream::VideoStream(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portConfig(portConfig) {
}

VideoStream::~VideoStream() {
    if (m_ocrEngine) {
        m_ocrEngine->End();
        delete m_ocrEngine;
    }
}

int VideoStream::type() {
    return VIDEOSTREAM;
}

QJsonObject VideoStream::config() {
    return m_portConfig;
}

bool VideoStream::open() {
    // port init
    if (m_mediaCaptureSession == nullptr) {
        m_mediaCaptureSession = new QMediaCaptureSession(this);
        const auto &portName = m_portConfig["portName"].toString();
        for (QScreen *screen: QGuiApplication::screens()) {
            if (portName == screen->name()) {
                m_screenCapture = new QScreenCapture(this);
                m_screenCapture->setScreen(screen);
                m_mediaCaptureSession->setScreenCapture(m_screenCapture);
                break;
            }
        }
        for (const QCameraDevice &camera: QMediaDevices::videoInputs()) {
            if (portName == camera.description()) {
                m_cameraCapture = new QCamera(camera, this);
                m_mediaCaptureSession->setCamera(m_cameraCapture);
                break;
            }
        }
    }
    if (m_ocrEngine == nullptr) {
        m_ocrEngine = new tesseract::TessBaseAPI();
        const QByteArray charsetBytes = "eng";
        // const QByteArray charsetBytes = charset.toUtf8();
        const char *charsetChar = charsetBytes.constData();
        m_ocrEngine->Init(nullptr, charsetChar);
        const QByteArray whitelistBytes = m_portConfig["whitelist"].toString().toUtf8();
        const char *whitelistChar = whitelistBytes.constData();
        m_ocrEngine->SetVariable("tessedit_char_whitelist", whitelistChar);
        // m_ocrEngine->SetVariable("load_system_dawg", "0");
        // m_ocrEngine->SetVariable("load_freq_dawg", "0");
    }
    // port open
    if (m_screenCapture) m_screenCapture->start();
    else if (m_cameraCapture) m_cameraCapture->start();
    else {
        emit refreshPort(m_portConfig["portName"].toString(), false);
        emit appendLog(QString("%1 open failed").arg(m_portConfig["portName"].toString()), "info");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 open failed").arg(timestamp, m_portConfig["portName"].toString());
        return false;
    }
    emit refreshPort(m_portConfig["portName"].toString(), true);
    emit appendLog(QString("%1 opened").arg(m_portConfig["portName"].toString()), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 opened").arg(timestamp, m_portConfig["portName"].toString());
    return true;
}

void VideoStream::close() {
    // port close
    if (m_screenCapture) m_screenCapture->stop();
    else if (m_cameraCapture) m_cameraCapture->stop();
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
    bool status = false;
    if (m_screenCapture) status = m_screenCapture->isActive();
    else if (m_cameraCapture) status = m_cameraCapture->isActive();
    qDebug() << status;
    return {};
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
