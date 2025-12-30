#include "portModule/videoStream.h"

#include <QCamera>
#include <QImageCapture>
#include <QJsonArray>
#include <QLabel>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QScreenCapture>
#include <QThread>
#include <QVideoSink>

#include "globals.h"
#include "utils/cvUtils.h"

// VideoStream public
VideoStream::VideoStream(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portConfig(portConfig) {
}

VideoStream::~VideoStream() {
    close();
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
        m_videoSink = new QVideoSink(this);
        m_mediaCaptureSession->setVideoSink(m_videoSink);
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
    // check port status
    if (!status) {
        emit appendLog(QString("%1 is not opened").arg(m_portConfig["portName"].toString()), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portConfig["portName"].toString());
        return {};
    }
    const auto rawFrame = m_videoSink->videoFrame();
    const auto rawImage = rawFrame.toImage();
    const auto rawPixmap = QPixmap::fromImage(rawImage);
    if (rawPixmap.isNull()) return {};

    QStringList resultList{};
    for (const QJsonValue &value: m_portConfig["roi"].toArray()) {
        QJsonArray roi = value.toArray();
        const auto rect = QRect(roi[0].toInt(), roi[1].toInt(), roi[2].toInt(), roi[3].toInt());
        const QPixmap cropped = rawPixmap.copy(rect);
        const QPixmap processed = processPipeline(cropped, m_portConfig["pipeline"].toArray());

        // for testing
        // QMetaObject::invokeMethod(g_mainWindow, [processed] {
        //     auto *label = new QLabel(g_mainWindow); // NOLINT
        //     label->setWindowTitle("test");
        //     label->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
        //     label->setPixmap(processed);
        //     label->show();
        // });

        const QImage image = processed.toImage().convertToFormat(QImage::Format_RGB888);
        m_ocrEngine->SetImage(image.bits(), image.width(), image.height(), 3, image.bytesPerLine());
        char *result = m_ocrEngine->GetUTF8Text();
        QString text = QString::fromUtf8(result);
        text = text.trimmed();
        resultList.append(text.isEmpty() ? "null" : text);
        delete result;
    }
    return resultList.join("\x1E").toUtf8();
}
