#include "port/portSetting.h"

#include <QCamera>
#include <QCameraDevice>
#include <QHostInfo>
#include <QImageCapture>
#include <QJsonArray>
#include <QJsonObject>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QPainter>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QQuickWidget>
#include <QScreenCapture>
#include <QSerialPortInfo>
#include <QVBoxLayout>
#include <QVideoSink>
#include <visa.h>

#include "globals.h"
#include "core/globalManager.h"
#include "tesseract/baseapi.h"
#include "util/cvUtils.h"

// public
PortSetting::PortSetting(QWidget *parent)
    : QObject(parent),
      m_window(new QQuickView()),
      m_serialPortStandardItemModel(new QStandardItemModel(this)),
      m_visaStandardItemModel(new QStandardItemModel(this)),
      m_localHostStandardItemModel(new QStandardItemModel(this)),
      m_videoStreamStandardItemModel(new QStandardItemModel(this)),
      m_mediaCaptureSession(new QMediaCaptureSession(this)),
      m_roiModel(new RoiModel(this)),
      m_pipelineModel(new PipelineModel(this)),
      m_imageProvider(new ImageProvider()) {
}

PortSetting::~PortSetting() {
    // delete m_imageProvider;
}

void PortSetting::propertySet(const QVariantHash &objects) {
    m_window->setTitle(tr("Port Setting"));
    m_window->setTransientParent(g_mainWindow->windowHandle());
    m_window->engine()->addImageProvider("capture", m_imageProvider);

    m_window->rootContext()->setContextProperty("portSetting", this);
    m_window->rootContext()->setContextProperty("global", g_globalManager);
    m_window->rootContext()->setContextProperty("mainToolTip", objects["mainWindowToolTip"]);
    m_window->rootContext()->setContextProperty("serialPortStandardItemModel", m_serialPortStandardItemModel);
    m_window->rootContext()->setContextProperty("visaStandardItemModel", m_visaStandardItemModel);
    m_window->rootContext()->setContextProperty("localHostStandardItemModel", m_localHostStandardItemModel);
    m_window->rootContext()->setContextProperty("videoStreamStandardItemModel", m_videoStreamStandardItemModel);
    m_window->rootContext()->setContextProperty("roiModel", m_roiModel);
    m_window->rootContext()->setContextProperty("pipelineModel", m_pipelineModel);

    m_window->setResizeMode(QQuickView::SizeRootObjectToView);
    m_window->setSource(QUrl("qrc:/qml/port/portSetting.qml"));
    m_root = m_window->rootObject();
}

void PortSetting::propertyGet(const QVariantMap &objects) {
    m_swipeView = qvariant_cast<QObject *>(objects["swipeView"]);
    // serial port
    m_serialPortNameComboBox = qvariant_cast<QObject *>(objects["serialPortNameComboBox"]);
    m_serialPortBaudRateSpinBox = qvariant_cast<QObject *>(objects["serialPortBaudRateSpinBox"]);
    m_serialPortDataBitsComboBox = qvariant_cast<QObject *>(objects["serialPortDataBitsComboBox"]);
    m_serialPortParityComboBox = qvariant_cast<QObject *>(objects["serialPortParityComboBox"]);
    m_serialPortStopBitsComboBox = qvariant_cast<QObject *>(objects["serialPortStopBitsComboBox"]);
    // visa
    m_visaNameComboBox = qvariant_cast<QObject *>(objects["visaNameComboBox"]);
    // tcp client
    m_tcpClientNameTextField = qvariant_cast<QObject *>(objects["tcpClientNameTextField"]);
    m_tcpClientRemoteHostTextField = qvariant_cast<QObject *>(objects["tcpClientRemoteHostTextField"]);
    m_tcpClientRemotePortSpinBox = qvariant_cast<QObject *>(objects["tcpClientRemotePortSpinBox"]);
    // ssl client
    m_sslClientNameTextField = qvariant_cast<QObject *>(objects["sslClientNameTextField"]);
    m_sslClientRemoteHostTextField = qvariant_cast<QObject *>(objects["sslClientRemoteHostTextField"]);
    m_sslClientRemotePortSpinBox = qvariant_cast<QObject *>(objects["sslClientRemotePortSpinBox"]);
    // tcp server
    m_tcpServerNameTextField = qvariant_cast<QObject *>(objects["tcpServerNameTextField"]);
    m_tcpServerLocalHostComboBox = qvariant_cast<QObject *>(objects["tcpServerLocalHostComboBox"]);
    m_tcpServerLocalPortSpinBox = qvariant_cast<QObject *>(objects["tcpServerLocalPortSpinBox"]);
    // udp server
    m_udpSocketNameTextField = qvariant_cast<QObject *>(objects["udpSocketNameTextField"]);
    m_udpSocketLocalHostComboBox = qvariant_cast<QObject *>(objects["udpSocketLocalHostComboBox"]);
    m_udpSocketLocalPortSpinBox = qvariant_cast<QObject *>(objects["udpSocketLocalPortSpinBox"]);
    m_udpSocketRemoteHostTextField = qvariant_cast<QObject *>(objects["udpSocketRemoteHostTextField"]);
    m_udpSocketRemotePortSpinBox = qvariant_cast<QObject *>(objects["udpSocketRemotePortSpinBox"]);
    // video stream
    m_videoStreamNameComboBox = qvariant_cast<QObject *>(objects["videoStreamNameComboBox"]);
    // format
    m_txFormatComboBox = qvariant_cast<QObject *>(objects["txFormatComboBox"]);
    m_txSuffixComboBox = qvariant_cast<QObject *>(objects["txSuffixComboBox"]);
    m_rxFormatComboBox = qvariant_cast<QObject *>(objects["rxFormatComboBox"]);
    m_bufferSizeSpinBox = qvariant_cast<QObject *>(objects["bufferSizeSpinBox"]);
    // image
    m_videoSink = objects["videoSink"].value<QVideoSink *>();
    m_mediaCaptureSession->setVideoSink(m_videoSink);
    m_previewImage = qvariant_cast<QObject *>(objects["previewImage"]);
    m_recognitionComboBox = qvariant_cast<QObject *>(objects["recognitionComboBox"]);
}

void PortSetting::portSettingImport(const QJsonObject &portConfig) {
    serialPortRefresh();
    visaRefresh();
    localHostRefresh();
    videoStreamRefresh();
    processRefresh(portConfig);
    if (portConfig.isEmpty()) {
        m_root->setProperty("portType", 0);
        m_swipeView->setProperty("currentIndex", 0);
        m_oldPortName = "";
        // serial port
        if (m_serialPortNameComboBox->property("count").toInt()) {
            m_serialPortNameComboBox->setProperty("currentIndex", 0);
        }
        m_serialPortBaudRateSpinBox->setProperty("value", 115200);
        m_serialPortDataBitsComboBox->setProperty("currentValue", 8);
        m_serialPortParityComboBox->setProperty("currentValue", 0);
        m_serialPortStopBitsComboBox->setProperty("currentValue", 1);
        // visa
        if (m_visaNameComboBox->property("count").toInt()) {
            m_visaNameComboBox->setProperty("currentIndex", 0);
        }
        // tcp client
        m_tcpClientNameTextField->setProperty("text", "");
        m_tcpClientRemoteHostTextField->setProperty("text", "");
        m_tcpClientRemotePortSpinBox->setProperty("value", 0);
        // ssl client
        m_sslClientNameTextField->setProperty("text", "");
        m_sslClientRemoteHostTextField->setProperty("text", "");
        m_sslClientRemotePortSpinBox->setProperty("value", 0);
        // tcp server
        m_tcpServerNameTextField->setProperty("text", "");
        if (m_tcpServerLocalHostComboBox->property("count").toInt()) {
            m_tcpServerLocalHostComboBox->setProperty("currentIndex", 0);
        }
        m_tcpServerLocalPortSpinBox->setProperty("value", 0);
        // udp socket
        m_udpSocketNameTextField->setProperty("text", "");
        if (m_udpSocketLocalHostComboBox->property("count").toInt()) {
            m_udpSocketLocalHostComboBox->setProperty("currentIndex", 0);
        }
        m_udpSocketLocalPortSpinBox->setProperty("value", 0);
        m_udpSocketRemoteHostTextField->setProperty("text", "");
        m_udpSocketRemotePortSpinBox->setProperty("value", 0);
        // video stream
        if (m_videoStreamNameComboBox->property("count").toInt()) {
            m_videoStreamNameComboBox->setProperty("currentIndex", 0);
        }
        // format
        m_txFormatComboBox->setProperty("currentValue", "hex");
        m_txSuffixComboBox->setProperty("currentValue", "null");
        m_rxFormatComboBox->setProperty("currentValue", "hex");
        m_bufferSizeSpinBox->setProperty("value", 1024);
    } else {
        m_swipeView->setProperty("currentIndex", 1);
        m_oldPortName = portConfig["portName"].toString();
        const int portType = portConfig["portType"].toInt();
        m_root->setProperty("portType", portType);
        switch (portType) {
            case PortType::SerialPort: {
                m_serialPortNameComboBox->setProperty("currentValue", portConfig["portName"].toString());
                m_serialPortBaudRateSpinBox->setProperty("value", portConfig["baudRate"].toInt());
                m_serialPortDataBitsComboBox->setProperty("currentValue", portConfig["dataBits"].toInt());
                m_serialPortParityComboBox->setProperty("currentValue", portConfig["parity"].toInt());
                m_serialPortStopBitsComboBox->setProperty("currentValue", portConfig["stopBits"].toInt());
                m_txFormatComboBox->setProperty("currentValue", portConfig["txFormat"].toString());
                m_txSuffixComboBox->setProperty("currentValue", portConfig["txSuffix"].toString());
                m_rxFormatComboBox->setProperty("currentValue", portConfig["rxFormat"].toString());
                m_bufferSizeSpinBox->setProperty("value", portConfig["bufferSize"].toInt());
            }
            break;
            case PortType::Visa: {
                m_visaNameComboBox->setProperty("currentValue", portConfig["portName"].toString());
                m_txFormatComboBox->setProperty("currentValue", portConfig["txFormat"].toString());
                m_txSuffixComboBox->setProperty("currentValue", portConfig["txSuffix"].toString());
                m_rxFormatComboBox->setProperty("currentValue", portConfig["rxFormat"].toString());
                m_bufferSizeSpinBox->setProperty("value", portConfig["bufferSize"].toInt());
            }
            break;
            case PortType::TcpClient: {
                m_tcpClientNameTextField->setProperty("text", portConfig["portName"].toString());
                m_tcpClientRemoteHostTextField->setProperty("text", portConfig["remoteHost"].toString());
                m_tcpClientRemotePortSpinBox->setProperty("value", portConfig["remotePort"].toInt());
                m_txFormatComboBox->setProperty("currentValue", portConfig["txFormat"].toString());
                m_txSuffixComboBox->setProperty("currentValue", portConfig["txSuffix"].toString());
                m_rxFormatComboBox->setProperty("currentValue", portConfig["rxFormat"].toString());
                m_bufferSizeSpinBox->setProperty("value", portConfig["bufferSize"].toInt());
            }
            break;
            case PortType::SslClient: {
                m_sslClientNameTextField->setProperty("text", portConfig["portName"].toString());
                m_sslClientRemoteHostTextField->setProperty("text", portConfig["remoteHost"].toString());
                m_sslClientRemotePortSpinBox->setProperty("value", portConfig["remotePort"].toInt());
                m_txFormatComboBox->setProperty("currentValue", portConfig["txFormat"].toString());
                m_txSuffixComboBox->setProperty("currentValue", portConfig["txSuffix"].toString());
                m_rxFormatComboBox->setProperty("currentValue", portConfig["rxFormat"].toString());
                m_bufferSizeSpinBox->setProperty("value", portConfig["bufferSize"].toInt());
            }
            break;
            case PortType::TcpServer: {
                m_tcpServerNameTextField->setProperty("text", portConfig["portName"].toString());
                m_tcpServerLocalHostComboBox->setProperty("currentValue", portConfig["localHost"].toString());
                m_tcpServerLocalPortSpinBox->setProperty("value", portConfig["localPort"].toInt());
                m_txFormatComboBox->setProperty("currentValue", portConfig["txFormat"].toString());
                m_txSuffixComboBox->setProperty("currentValue", portConfig["txSuffix"].toString());
                m_rxFormatComboBox->setProperty("currentValue", portConfig["rxFormat"].toString());
                m_bufferSizeSpinBox->setProperty("value", portConfig["bufferSize"].toInt());
            }
            break;
            case PortType::UdpSocket: {
                m_udpSocketNameTextField->setProperty("text", portConfig["portName"].toString());
                m_udpSocketLocalHostComboBox->setProperty("currentValue", portConfig["localHost"].toString());
                m_udpSocketLocalPortSpinBox->setProperty("value", portConfig["localPort"].toInt());
                m_udpSocketRemoteHostTextField->setProperty("text", portConfig["remoteHost"].toString());
                m_udpSocketRemotePortSpinBox->setProperty("value", portConfig["remotePort"].toInt());
                m_txFormatComboBox->setProperty("currentValue", portConfig["txFormat"].toString());
                m_txSuffixComboBox->setProperty("currentValue", portConfig["txSuffix"].toString());
                m_rxFormatComboBox->setProperty("currentValue", portConfig["rxFormat"].toString());
                m_bufferSizeSpinBox->setProperty("value", portConfig["bufferSize"].toInt());
            }
            break;
            case PortType::VideoStream: {
                m_videoStreamNameComboBox->setProperty("currentValue", portConfig["portName"].toString());
            }
            break;
            default: break;
        }
    }
    m_window->resize(600, 500);
    m_window->show();
}

void PortSetting::portSettingExport() {
    const int portType = m_root->property("portType").toInt();
    QJsonObject portConfig{};
    switch (portType) {
        case PortType::SerialPort: {
            portConfig = {
                {"portType", portType},
                {"portName", m_serialPortNameComboBox->property("currentValue").toString()},
                {"baudRate", m_serialPortBaudRateSpinBox->property("value").toInt()},
                {"dataBits", m_serialPortDataBitsComboBox->property("currentValue").toInt()},
                {"parity", m_serialPortParityComboBox->property("currentValue").toInt()},
                {"stopBits", m_serialPortStopBitsComboBox->property("currentValue").toInt()},
                {"txFormat", m_txFormatComboBox->property("currentValue").toString()},
                {"txSuffix", m_txSuffixComboBox->property("currentValue").toString()},
                {"rxFormat", m_rxFormatComboBox->property("currentValue").toString()},
                {"bufferSize", m_bufferSizeSpinBox->property("value").toInt()}
            };
        }
        break;
        case PortType::Visa: {
            portConfig = {
                {"portType", portType},
                {"portName", m_visaNameComboBox->property("currentValue").toString()},
                {"txFormat", m_txFormatComboBox->property("currentValue").toString()},
                {"txSuffix", m_txSuffixComboBox->property("currentValue").toString()},
                {"rxFormat", m_rxFormatComboBox->property("currentValue").toString()},
                {"bufferSize", m_bufferSizeSpinBox->property("value").toInt()}
            };
        }
        break;
        case PortType::TcpClient: {
            portConfig = {
                {"portType", portType},
                {"portName", m_tcpClientNameTextField->property("text").toString()},
                {"remoteHost", m_tcpClientRemoteHostTextField->property("text").toString()},
                {"remotePort", m_tcpClientRemotePortSpinBox->property("value").toInt()},
                {"txFormat", m_txFormatComboBox->property("currentValue").toString()},
                {"txSuffix", m_txSuffixComboBox->property("currentValue").toString()},
                {"rxFormat", m_rxFormatComboBox->property("currentValue").toString()},
                {"bufferSize", m_bufferSizeSpinBox->property("value").toInt()}
            };
        }
        break;
        case PortType::SslClient: {
            portConfig = {
                {"portType", portType},
                {"portName", m_sslClientNameTextField->property("text").toString()},
                {"remoteHost", m_sslClientRemoteHostTextField->property("text").toString()},
                {"remotePort", m_sslClientRemotePortSpinBox->property("value").toInt()},
                {"txFormat", m_txFormatComboBox->property("currentValue").toString()},
                {"txSuffix", m_txSuffixComboBox->property("currentValue").toString()},
                {"rxFormat", m_rxFormatComboBox->property("currentValue").toString()},
                {"bufferSize", m_bufferSizeSpinBox->property("value").toInt()}
            };
        }
        break;
        case PortType::TcpServer: {
            portConfig = {
                {"portType", portType},
                {"portName", m_tcpServerNameTextField->property("text").toString()},
                {"localHost", m_tcpServerLocalHostComboBox->property("currentValue").toString()},
                {"localPort", m_tcpServerLocalPortSpinBox->property("value").toInt()},
                {"txFormat", m_txFormatComboBox->property("currentValue").toString()},
                {"txSuffix", m_txSuffixComboBox->property("currentValue").toString()},
                {"rxFormat", m_rxFormatComboBox->property("currentValue").toString()},
                {"bufferSize", m_bufferSizeSpinBox->property("value").toInt()}
            };
        }
        break;
        case PortType::UdpSocket: {
            portConfig = {
                {"portType", portType},
                {"portName", m_udpSocketNameTextField->property("text").toString()},
                {"localHost", m_udpSocketLocalHostComboBox->property("currentValue").toString()},
                {"localPort", m_udpSocketLocalPortSpinBox->property("value").toInt()},
                {"remoteHost", m_udpSocketRemoteHostTextField->property("text").toString()},
                {"remotePort", m_udpSocketRemotePortSpinBox->property("value").toInt()},
                {"txFormat", m_txFormatComboBox->property("currentValue").toString()},
                {"txSuffix", m_txSuffixComboBox->property("currentValue").toString()},
                {"rxFormat", m_rxFormatComboBox->property("currentValue").toString()},
                {"bufferSize", m_bufferSizeSpinBox->property("value").toInt()}
            };
        }
        break;
        case PortType::VideoStream: {
            if (m_screenCapture) {
                m_mediaCaptureSession->setScreenCapture(nullptr);
                m_screenCapture->stop();
                m_screenCapture->deleteLater();
                m_screenCapture = nullptr;
            } else if (m_cameraCapture) {
                m_mediaCaptureSession->setCamera(nullptr);
                m_cameraCapture->stop();
                m_cameraCapture->deleteLater();
                m_cameraCapture = nullptr;
            }
            QJsonArray roiArray{};
            for (int i = 0; i < m_roiModel->rowCount(); ++i) {
                const QJsonArray roi = QJsonArray::fromVariantList(m_roiModel->item(i, 0)->data(Qt::WhatsThisRole).toList());
                roiArray.append(roi);
            }
            QJsonArray pipelineArray{};
            for (int i = 0; i < m_pipelineModel->rowCount(); ++i) {
                const QJsonObject session = QJsonObject::fromVariantHash(m_pipelineModel->item(i, 0)->data(Qt::WhatsThisRole).toHash());
                pipelineArray.append(session);
            }
            portConfig = {
                {"portType", portType},
                {"portName", m_videoStreamNameComboBox->property("currentValue").toString()},
                {"roi", roiArray},
                {"pipeline", pipelineArray},
                {
                    "recognition", QJsonObject{
                        {"mode", m_recognitionComboBox->property("currentIndex").toInt()}
                    }
                }
            };
        }
        break;
        default: break;
    }
    if (m_oldPortName.isEmpty()) {
        emit insertPort(-1, portConfig);
    } else {
        emit editPort(m_oldPortName, portConfig);
    }
    m_window->hide();
}

void PortSetting::dialogResize(const int width, const int height) const {
    m_window->resize(width, height);
}

void PortSetting::videoCapture() {
    m_window->resize(1600, 900);
    if (m_screenCapture) {
        m_mediaCaptureSession->setScreenCapture(nullptr);
        m_screenCapture->stop();
        m_screenCapture->deleteLater();
        m_screenCapture = nullptr;
    } else if (m_cameraCapture) {
        m_mediaCaptureSession->setCamera(nullptr);
        m_cameraCapture->stop();
        m_cameraCapture->deleteLater();
        m_cameraCapture = nullptr;
    }
    const auto &portName = m_videoStreamNameComboBox->property("currentValue").toString();
    for (QScreen *screen: QGuiApplication::screens()) {
        if (portName == screen->name()) {
            m_screenCapture = new QScreenCapture(this);
            m_screenCapture->setScreen(screen);
            m_mediaCaptureSession->setScreenCapture(m_screenCapture);
            m_screenCapture->start();
            return;
        }
    }
    for (const QCameraDevice &camera: QMediaDevices::videoInputs()) {
        if (portName == camera.description()) {
            m_cameraCapture = new QCamera(camera, this);
            m_mediaCaptureSession->setCamera(m_cameraCapture);
            m_cameraCapture->start();
            return;
        }
    }
}

void PortSetting::previewLoad(const int index, const int mode) const {
    if (index == -1 || !m_roiModel->item(index, 0)) {
        m_previewImage->setProperty("source", "qrc:/icon/null.svg");
        return;
    }
    const QJsonArray roi = QJsonArray::fromVariantList(m_roiModel->item(index, 0)->data(Qt::WhatsThisRole).toList());
    QJsonArray pipeline{};
    for (int i = 0; i < m_pipelineModel->rowCount(); ++i) {
        const QJsonObject session = QJsonObject::fromVariantHash(m_pipelineModel->item(i, 0)->data(Qt::WhatsThisRole).toHash());
        pipeline.append(session);
    }
    m_imageProvider->preview(m_videoSink, roi, pipeline);
    m_previewImage->setProperty("source", "image://capture/" + QString::number(QDateTime::currentMSecsSinceEpoch()));

    qDebug() << m_imageProvider->recognition(mode);


}

void PortSetting::roiInsert(const QVariantList &roi) const {
    QString text{};
    if (roi.size() == 4) {
        const int x = roi[0].toInt();
        const int y = roi[1].toInt();
        const int w = roi[2].toInt();
        const int h = roi[3].toInt();
        text = QString::number(x) + " " + QString::number(y) + " " + QString::number(w) + " " + QString::number(h);
    } else if (roi.size() == 8) {
        text = "Quadrilateral";
    }
    auto *item = new QStandardItem(text); // NOLINT
    m_roiModel->appendRow(item);
    item->setData(roi, Qt::WhatsThisRole);
    QMetaObject::invokeMethod(m_root, "indicatorReload");
}

void PortSetting::roiRemove(const int index) const {
    m_roiModel->removeRow(index);
    QMetaObject::invokeMethod(m_root, "indicatorReload");
}

void PortSetting::roiSwap(const int src, const int dst) const {
    const auto tmp = m_roiModel->takeRow(src);
    m_roiModel->insertRow(dst, tmp);
    QMetaObject::invokeMethod(m_root, "roiReload");
    QMetaObject::invokeMethod(m_root, "indicatorReload");
}

void PortSetting::pipelineInsert(const QVariantHash &session) const {
    const int type = session["type"].toInt();
    switch (type) {
        case ImagePipeline::Scale: {
            auto *item = new QStandardItem("Scale"); // NOLINT
            m_pipelineModel->appendRow(item);
            item->setData(session, Qt::WhatsThisRole);
        }
        break;
        case ImagePipeline::Threshold: {
            auto *item = new QStandardItem("Threshold"); // NOLINT
            m_pipelineModel->appendRow(item);
            item->setData(session, Qt::WhatsThisRole);
        }
        break;
        default: break;
    }
}

void PortSetting::pipelineRemove(const int index) const {
    m_pipelineModel->removeRow(index);
}

void PortSetting::pipelineSwap(const int src, const int dst) const {
    const auto tmp = m_pipelineModel->takeRow(src);
    m_pipelineModel->insertRow(dst, tmp);
    QMetaObject::invokeMethod(m_root, "pipelineReload");
}

// private
void PortSetting::serialPortRefresh() const {
    m_serialPortStandardItemModel->clear();
    for (QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts(); const QSerialPortInfo &port: ports) {
        const QString portName = port.portName();
        auto *item = new QStandardItem(portName + " " + port.description()); // NOLINT
        item->setData(portName, Qt::WhatsThisRole);
        m_serialPortStandardItemModel->appendRow(item);
    }
}

void PortSetting::visaRefresh() const {
    ViFindList findList;
    ViUInt32 numInst;
    ViChar portName[VI_FIND_BUFLEN];
    // resource manager check
    ViStatus status = viOpenDefaultRM(&g_rm);
    if (status != VI_SUCCESS) {
        qDebug() << "failed to start visa resource manager";
        return;
    }
    // device check
    status = viFindRsrc(g_rm, "?*INSTR", &findList, &numInst, portName);
    if (status != VI_SUCCESS) {
        qDebug() << "failed to find visa instruments";
        viClose(g_rm);
        return;
    }
    // standard item model construct
    m_visaStandardItemModel->clear();
    // first device
    auto *firstItem = new QStandardItem(QString(portName)); // NOLINT
    firstItem->setData(QString(portName), Qt::WhatsThisRole);
    m_visaStandardItemModel->appendRow(firstItem);
    // following device
    for (ViUInt32 i = 1; i < numInst; i++) {
        status = viFindNext(findList, portName);
        if (status == VI_SUCCESS) {
            auto *followingItem = new QStandardItem(QString(portName)); // NOLINT
            followingItem->setData(QString(portName), Qt::WhatsThisRole);
            m_visaStandardItemModel->appendRow(followingItem);
        }
    }
    // free resource
    viClose(findList);
    viClose(g_rm);
}

void PortSetting::localHostRefresh() const {
    m_localHostStandardItemModel->clear();
    for (const QHostAddress &address: QHostInfo::fromName(QHostInfo::localHostName()).addresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            const QString portName = address.toString();
            auto *item = new QStandardItem(portName); // NOLINT
            item->setData(portName, Qt::WhatsThisRole);
            m_localHostStandardItemModel->appendRow(item);
        } else if (address.protocol() == QAbstractSocket::IPv6Protocol) {
            auto portName = address.toString();
            // const QString scopeId = address.scopeId();
            // if (!scopeId.isEmpty()) portName.remove("%"+scopeId);
            auto *item = new QStandardItem(portName); // NOLINT
            item->setData(portName, Qt::WhatsThisRole);
            m_localHostStandardItemModel->appendRow(item);
        }
    }
}

void PortSetting::videoStreamRefresh() const {
    m_videoStreamStandardItemModel->clear();
    for (const QScreen *screen: QGuiApplication::screens()) {
        const QString portName = screen->name();
        auto *item = new QStandardItem(portName); // NOLINT
        item->setData(portName, Qt::WhatsThisRole);
        m_videoStreamStandardItemModel->appendRow(item);
    }
    for (const QCameraDevice &camera: QMediaDevices::videoInputs()) {
        const QString portName = camera.description();
        auto *item = new QStandardItem(portName); // NOLINT
        item->setData(portName, Qt::WhatsThisRole);
        m_videoStreamStandardItemModel->appendRow(item);
    }
}

void PortSetting::processRefresh(const QJsonObject &portConfig) const {
    m_roiModel->clear();
    m_pipelineModel->clear();
    QMetaObject::invokeMethod(m_root, "indicatorReload");
    m_recognitionComboBox->setProperty("currentIndex", 0);
    if (!portConfig.isEmpty()) {
        // roi
        for (const auto &value: portConfig["roi"].toArray()) {
            QJsonArray roi = value.toArray();
            roiInsert(roi.toVariantList());
        }
        for (const auto &value: portConfig["pipeline"].toArray()) {
            QVariantHash session = value.toObject().toVariantHash();
            pipelineInsert(session);
        }
    }
}

// public
ImageProvider::ImageProvider()
    : QQuickImageProvider(Pixmap) {
    m_ocrEngine = new tesseract::TessBaseAPI();
    const QByteArray charsetBytes = "eng";
    const char *charsetChar = charsetBytes.constData();
    m_ocrEngine->Init(nullptr, charsetChar);
}

ImageProvider::~ImageProvider() {
    if (m_ocrEngine) {
        m_ocrEngine->End();
        delete m_ocrEngine;
    }
}

QPixmap ImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) {
    return m_preview;
}

void ImageProvider::preview(const QVideoSink *videoSink, const QJsonArray &roi, const QJsonArray &pipeline) {
    const auto videoFrame = videoSink->videoFrame();
    const auto image = videoFrame.toImage();
    QPixmap cropped{};
    if (roi.size() == 4) {
        const auto rect = QRect(roi[0].toInt(), roi[1].toInt(), roi[2].toInt(), roi[3].toInt());
        cropped = QPixmap::fromImage(image.copy(rect));
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
        QImage result(w, h, image.format());
        QPainter painter(&result);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.setTransform(transform);
        painter.drawImage(0, 0, image);
        cropped = QPixmap::fromImage(result);
    }
    m_preview = pipelineProcess(cropped, pipeline);
}

QString ImageProvider::recognition(const int mode) const {
    QString result{};
    switch (mode) {
        case Recognition::OCR: {
            const QImage image = m_preview.toImage().convertToFormat(QImage::Format_Grayscale8);
            m_ocrEngine->SetImage(image.bits(), image.width(), image.height(), 1, image.bytesPerLine());
            char *_result = m_ocrEngine->GetUTF8Text();
            result = QString::fromUtf8(_result).trimmed();
            delete _result;
        }
            break;
        case Recognition::CornerShiTomasi: {
            const QPoint point = goodFeaturesToTrack(m_preview);
            result = point.x() < 0 || point.y() < 0 ? "null" : QString("%1,%2").arg(point.x()).arg(point.y());
        }
            break;
        case Recognition::CornerHarris: {
            const QPoint point = harris(m_preview);
            result = point.x() < 0 || point.y() < 0 ? "null" : QString("%1,%2").arg(point.x()).arg(point.y());
        }
            break;
        default: break;
    }
    return result;
}

// public
RoiModel::RoiModel(QObject *parent)
    : QStandardItemModel(parent) {
    connect(this, &QAbstractItemModel::rowsInserted, this, &RoiModel::emptyChanged);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &RoiModel::emptyChanged);
    connect(this, &QAbstractItemModel::modelReset, this, &RoiModel::emptyChanged);
}

// public
PipelineModel::PipelineModel(QObject *parent)
    : QStandardItemModel(parent) {
    connect(this, &QAbstractItemModel::rowsInserted, this, &PipelineModel::emptyChanged);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &PipelineModel::emptyChanged);
    connect(this, &QAbstractItemModel::modelReset, this, &PipelineModel::emptyChanged);
}
