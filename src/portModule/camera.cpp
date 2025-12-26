#include "portModule/camera.h"

#include <QCamera>
#include <QEventLoop>
#include <QImageCapture>
#include <QMediaCaptureSession>
#include <QMediaDevices>

#include "globals.h"
#include "utils/cvUtils.h"

// Camera public
Camera::Camera(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portConfig(portConfig) {
}

int Camera::type() {
    return CAMERA;
}

QJsonObject Camera::config() {
    return m_portConfig;
}

bool Camera::open() {
    emit refreshPort(m_portConfig["portName"].toString(), true);
    emit appendLog(QString("%1 opened").arg(m_portConfig["portName"].toString()), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 opened").arg(timestamp, m_portConfig["portName"].toString());
    return true;
}

void Camera::close() {
    emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(QString("%1 closed").arg(m_portConfig["portName"].toString()), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 closed").arg(timestamp, m_portConfig["portName"].toString());
}

std::unordered_map<std::string, std::string> Camera::info() {
    return {};
}

QByteArray Camera::read(const int timeout, const int length, const QString &rxFormat) {
    // find camera
    m_camera = QCameraDevice();
    for (const QCameraDevice &camera: QMediaDevices::videoInputs()) {
        if (camera.description() == m_portConfig["portName"].toString()) {
            m_camera = camera;
            break;
        }
    }
    if (m_camera.isNull()) return "camera not found";;
    QPixmap shot{};
    const auto camera = new QCamera(m_camera, this);
    QMediaCaptureSession captureSession;
    captureSession.setCamera(camera);
    QImageCapture imageCapture;
    captureSession.setImageCapture(&imageCapture);
    QEventLoop loop;
    connect(&imageCapture, &QImageCapture::imageCaptured, this, [&shot, &loop](int, const QImage &img) {
        shot = QPixmap::fromImage(img);
        loop.quit();
    });
    camera->start();
    imageCapture.capture();
    loop.exec();
    camera->stop();
    delete camera;
    QStringList resultList{};
    for (const QJsonValue &value: m_portConfig["roi"].toArray()) {
        QJsonArray roi = value.toArray();
        const int x = roi[0].toInt();
        const int y = roi[1].toInt();
        const int width = roi[2].toInt();
        const int height = roi[3].toInt();
        const auto rect = QRect(x, y, width, height);
        const QPixmap cropped = shot.copy(rect);
        const QString text = ocr(cropped, "eng", m_portConfig["whitelist"].toString());
        resultList.append(text);
    }
    return resultList.join("\x1E").toUtf8();
}
