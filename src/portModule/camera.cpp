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
      m_portConfig(portConfig),
      m_portName(portConfig["portName"].toString()),
      m_charset(portConfig["charset"].toString()),
      m_process(portConfig["process"].toObject()),
      m_areaList(portConfig["areaList"].toArray()) {
}

int Camera::type() {
    return CAMERA;
}

QJsonObject Camera::config() {
    return m_portConfig;
}

bool Camera::open() {
    m_showPreview = true;
    emit refreshPort(m_portConfig["portName"].toString(), true);
    emit appendLog(QString("%1 opened").arg(m_portName), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 opened").arg(timestamp, m_portName);
    return true;
}

void Camera::close() {
    m_showPreview = false;
    emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(QString("%1 closed").arg(m_portName), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 closed").arg(timestamp, m_portName);
}

std::unordered_map<std::string, std::string> Camera::info() {
    return {};
}

QByteArray Camera::read(const int timeout, const int length, const QString &rxFormat) {
    // find camera
    m_camera = QCameraDevice();
    for (const QCameraDevice &camera: QMediaDevices::videoInputs()) {
        if (camera.description() == m_portName) {
            m_camera = camera;
            break;
        }
    }
    if (m_camera.isNull())
        return "camera not found";;
    // take picture
    QPixmap shot;
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
    QList<QPixmap> pixmapList{};
    QStringList resultList;
    for (const QJsonValue &value: m_areaList) {
        QJsonArray areaArray = value.toArray();
        const int x = areaArray[0].toInt();
        const int y = areaArray[1].toInt();
        const int width = areaArray[2].toInt();
        const int height = areaArray[3].toInt();
        const auto rect = QRect(x, y, width, height);
        const QPixmap cropped = shot.copy(rect);
        QPixmap processed{};
        const int processType = m_process["processType"].toInt();
        if (processType == RAW) {
            processed = cropped;
        } else {
            switch (processType) {
                case GAUSSIANBLUR: {
                    const int kernalSize = m_process["thresholdValue"].toInt();
                    processed = processGaussianBlur(cropped, kernalSize);
                    break;
                }
                case THRESHOLD: {
                    const int thresholdValue = m_process["thresholdValue"].toInt();
                    const int thresholdType = m_process["thresholdType"].toInt();
                    processed = processThreshold(cropped, thresholdValue, thresholdType);
                    break;
                }
                default: break;
            }
        }
        if (m_showPreview) pixmapList.append(processed);
        const QString text = ocr(processed, m_charset);
        resultList.append(text);
    }
    // return resultList.join("\x1E");
}
