#include "port/videoStream.h"

#include <QCamera>
#include <QImageCapture>
#include <QJsonArray>
#include <QLabel>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QPainter>
#include <QScreenCapture>
#include <QThread>
#include <QVideoSink>

#include "globals.h"
#include "util/cvUtils.h"

// public
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
    return PortType::VideoStream;
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
        // const QByteArray whitelistBytes = m_portConfig["whitelist"].toString().toUtf8();
        // const char *whitelistChar = whitelistBytes.constData();
        // m_ocrEngine->SetVariable("tessedit_char_whitelist", whitelistChar);
        // m_ocrEngine->SetVariable("load_system_dawg", "0");
        // m_ocrEngine->SetVariable("load_freq_dawg", "0");
    }
    // port open
    if (m_screenCapture) m_screenCapture->start();
    else if (m_cameraCapture) m_cameraCapture->start();
    else {
        emit refreshPort(m_portConfig["portName"].toString(), false);
        emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), "open failed");
        return false;
    }
    emit refreshPort(m_portConfig["portName"].toString(), true);
    emit appendLog(LogLevel::Info, QString("[%1]").arg(m_portConfig["portName"].toString()), "opened");
    return true;
}

void VideoStream::close() {
    // port close
    if (m_screenCapture) m_screenCapture->stop();
    else if (m_cameraCapture) m_cameraCapture->stop();
    clear();
    emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(LogLevel::Info, QString("[%1]").arg(m_portConfig["portName"].toString()), "closed");
}

void VideoStream::clear() {
}

QVariantHash VideoStream::info() {
    return {};
}

// TODO: register commands to video stream
bool VideoStream::write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) {
    bool status = false;
    if (m_screenCapture) status = m_screenCapture->isActive();
    else if (m_cameraCapture) status = m_cameraCapture->isActive();
    // check port status
    if (!status) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), "not opened");
        return {};
    }
    const auto rawFrame = m_videoSink->videoFrame();
    const auto rawImage = rawFrame.toImage();
    if (rawImage.isNull()) return {};

    const auto command = QString::fromUtf8(txData);
    if (command == "raw") {
        const QString fileName = g_workspaceUrl.toLocalFile() + "/raw.png";
        return rawImage.save(fileName);
    }
    if (command == "processed") {
        return {};
    }
    return {};
}

QByteArray VideoStream::read(const int length, const int timeout, const QString &rxFormat) {
    bool status = false;
    if (m_screenCapture) status = m_screenCapture->isActive();
    else if (m_cameraCapture) status = m_cameraCapture->isActive();
    // check port status
    if (!status) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), "not opened");
        return {};
    }
    const auto rawFrame = m_videoSink->videoFrame();
    const auto rawImage = rawFrame.toImage();
    if (rawImage.isNull()) return {};

    QStringList resultList{};
    for (const QJsonValue &value: m_portConfig["roi"].toArray()) {
        // roi
        QPixmap cropped{};
        QJsonArray roi = value.toArray();
        if (roi.size() == 4) {
            const auto rect = QRect(roi[0].toInt(), roi[1].toInt(), roi[2].toInt(), roi[3].toInt());
            cropped = QPixmap::fromImage(rawImage.copy(rect));
        } else if (roi.size() == 8) {
            // src poly
            QPolygon src({
                QPoint(roi[0].toInt(), roi[1].toInt()),
                QPoint(roi[2].toInt(), roi[3].toInt()),
                QPoint(roi[4].toInt(), roi[5].toInt()),
                QPoint(roi[6].toInt(), roi[7].toInt())
            });
            // dst poly
            const int w = qRound(qMax(QLineF(src[0], src[1]).length(), QLineF(src[2], src[3]).length()));
            const int h = qRound(qMax(QLineF(src[0], src[3]).length(), QLineF(src[1], src[2]).length()));
            const QPolygon dst({
                QPoint(0, 0),
                QPoint(w, 0),
                QPoint(w, h),
                QPoint(0, h)
            });
            // perform transform
            QTransform transform;
            QTransform::quadToQuad(src, dst, transform);
            QImage result(w, h, rawImage.format());
            QPainter painter(&result);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            painter.setTransform(transform);
            painter.drawImage(0, 0, rawImage);
            cropped = QPixmap::fromImage(result);
        }
        // pipeline
        const QPixmap processed = pipelineProcess(cropped, m_portConfig["pipeline"].toArray());
        // for testing
        // QMetaObject::invokeMethod(g_mainWindow, [processed] {
        //     auto *label = new QLabel(g_mainWindow); // NOLINT
        //     label->setWindowTitle("test");
        //     label->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
        //     label->setPixmap(processed);
        //     label->show();
        // });
        const QImage image = processed.toImage().convertToFormat(QImage::Format_Grayscale8);
        m_ocrEngine->SetImage(image.bits(), image.width(), image.height(), 1, image.bytesPerLine());
        // recognition
        const auto recognition = m_portConfig["recognition"].toObject();
        const auto mode = recognition["mode"].toInt();
        switch (mode) {
            case Recognition::OCR: {
                char *result = m_ocrEngine->GetUTF8Text();
                QString text = QString::fromUtf8(result);
                text = text.trimmed();
                resultList.append(text.isEmpty() ? "null" : text);
                delete[] result;
            }
            break;
            case Recognition::CornerShiTomasi: {
                const QPoint point = goodFeaturesToTrack(processed);
                resultList.append(point.x() < 0 || point.y() < 0 ? "null" : QString("%1,%2").arg(point.x()).arg(point.y()));
            }
            break;
            case Recognition::CornerHarris: {
                const QPoint point = cornerHarris(processed);
                resultList.append(point.x() < 0 || point.y() < 0 ? "null" : QString("%1,%2").arg(point.x()).arg(point.y()));
            }
            break;
            default: break;
        }
    }
    return resultList.join("\x1E").toUtf8();
}
