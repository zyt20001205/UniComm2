#include "portModule/portSetting.h"

#include <QCamera>
#include <QCameraDevice>
#include <QHostInfo>
#include <QImageCapture>
#include <QJsonArray>
#include <QJsonObject>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QQmlContext>
#include <QQuickItem>
#include <QScreenCapture>
#include <QSerialPortInfo>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QVideoSink>
#include <visa.h>

#include "globals.h"
#include "utils/cvUtils.h"

// PortSetting public
PortSetting::PortSetting(QWidget *parent)
    : QWidget(parent),
      m_portSettingDialog(new QDialog(this)),
      m_serialPortStandardItemModel(new QStandardItemModel(this)),
      m_visaStandardItemModel(new QStandardItemModel(this)),
      m_localHostStandardItemModel(new QStandardItemModel(this)),
      m_videoStreamStandardItemModel(new QStandardItemModel(this)),
      m_mediaCaptureSession(new QMediaCaptureSession(this)),
      m_roiStandardItemModel(new QStandardItemModel(this)),
      m_pipelineStandardItemModel(new QStandardItemModel(this)),
      m_imageProvider(new ImageProvider()) {
    propertySet();
}

PortSetting::~PortSetting() {
    // delete m_imageProvider;
}

void PortSetting::propertySet() {
    m_portSettingDialog->setWindowTitle(tr("Port Setting"));
    auto *layout = new QVBoxLayout(m_portSettingDialog); // NOLINT
    auto *widget = new QQuickWidget(); // NOLINT
    layout->addWidget(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    QQmlEngine *engine = widget->engine();
    engine->addImageProvider("capture", m_imageProvider);
    widget->rootContext()->setContextProperty("portSetting", this);
    widget->rootContext()->setContextProperty("serialPortStandardItemModel", m_serialPortStandardItemModel);
    widget->rootContext()->setContextProperty("visaStandardItemModel", m_visaStandardItemModel);
    widget->rootContext()->setContextProperty("localHostStandardItemModel", m_localHostStandardItemModel);
    widget->rootContext()->setContextProperty("videoStreamStandardItemModel", m_videoStreamStandardItemModel);
    widget->rootContext()->setContextProperty("roiStandardItemModel", m_roiStandardItemModel);
    widget->rootContext()->setContextProperty("pipelineStandardItemModel", m_pipelineStandardItemModel);
    widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    widget->setSource(QUrl("qrc:/qml/portModule/portSetting.qml"));
    m_rootItem = widget->rootObject();
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
    // image
    m_videoSink = objects["videoSink"].value<QVideoSink *>();
    m_mediaCaptureSession->setVideoSink(m_videoSink);
    m_previewImage = qvariant_cast<QObject *>(objects["previewImage"]);
    m_whitelistSwitch = qvariant_cast<QObject *>(objects["whitelistSwitch"]);
    m_whitelistTextField = qvariant_cast<QObject *>(objects["whitelistTextField"]);
}

void PortSetting::portSettingImport(const QJsonObject &portConfig) {
    serialPortRefresh();
    visaRefresh();
    localHostRefresh();
    videoStreamRefresh();
    processRefresh(portConfig);
    if (portConfig.isEmpty()) {
        m_rootItem->setProperty("portType", 0);
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
    } else {
        m_swipeView->setProperty("currentIndex", 1);
        m_oldPortName = portConfig["portName"].toString();
        const int portType = portConfig["portType"].toInt();
        m_rootItem->setProperty("portType", portType);
        switch (portType) {
            case SERIALPORT: {
                m_serialPortNameComboBox->setProperty("currentValue", portConfig["portName"].toString());
                m_serialPortBaudRateSpinBox->setProperty("value", portConfig["baudRate"].toInt());
                m_serialPortDataBitsComboBox->setProperty("currentValue", portConfig["dataBits"].toInt());
                m_serialPortParityComboBox->setProperty("currentValue", portConfig["parity"].toInt());
                m_serialPortStopBitsComboBox->setProperty("currentValue", portConfig["stopBits"].toInt());
                m_txFormatComboBox->setProperty("currentValue", portConfig["txFormat"].toString());
                m_txSuffixComboBox->setProperty("currentValue", portConfig["txSuffix"].toString());
                m_rxFormatComboBox->setProperty("currentValue", portConfig["rxFormat"].toString());
            }
            break;
            case VISA: {
                m_visaNameComboBox->setProperty("currentValue", portConfig["portName"].toString());
                m_txFormatComboBox->setProperty("currentValue", portConfig["txFormat"].toString());
                m_txSuffixComboBox->setProperty("currentValue", portConfig["txSuffix"].toString());
                m_rxFormatComboBox->setProperty("currentValue", portConfig["rxFormat"].toString());
            }
            break;
            case TCPCLIENT: {
                m_tcpClientNameTextField->setProperty("text", portConfig["portName"].toString());
                m_tcpClientRemoteHostTextField->setProperty("text", portConfig["remoteHost"].toString());
                m_tcpClientRemotePortSpinBox->setProperty("value", portConfig["remotePort"].toInt());
                m_txFormatComboBox->setProperty("currentValue", portConfig["txFormat"].toString());
                m_txSuffixComboBox->setProperty("currentValue", portConfig["txSuffix"].toString());
                m_rxFormatComboBox->setProperty("currentValue", portConfig["rxFormat"].toString());
            }
            break;
            case SSLCLIENT: {
                m_sslClientNameTextField->setProperty("text", portConfig["portName"].toString());
                m_sslClientRemoteHostTextField->setProperty("text", portConfig["remoteHost"].toString());
                m_sslClientRemotePortSpinBox->setProperty("value", portConfig["remotePort"].toInt());
                m_txFormatComboBox->setProperty("currentValue", portConfig["txFormat"].toString());
                m_txSuffixComboBox->setProperty("currentValue", portConfig["txSuffix"].toString());
                m_rxFormatComboBox->setProperty("currentValue", portConfig["rxFormat"].toString());
            }
            break;
            case TCPSERVER: {
                m_tcpServerNameTextField->setProperty("text", portConfig["portName"].toString());
                m_tcpServerLocalHostComboBox->setProperty("currentValue", portConfig["localHost"].toString());
                m_tcpServerLocalPortSpinBox->setProperty("value", portConfig["localPort"].toInt());
                m_txFormatComboBox->setProperty("currentValue", portConfig["txFormat"].toString());
                m_txSuffixComboBox->setProperty("currentValue", portConfig["txSuffix"].toString());
                m_rxFormatComboBox->setProperty("currentValue", portConfig["rxFormat"].toString());
            }
            break;
            case UDPSOCKET: {
                m_udpSocketNameTextField->setProperty("text", portConfig["portName"].toString());
                m_udpSocketLocalHostComboBox->setProperty("currentValue", portConfig["localHost"].toString());
                m_udpSocketLocalPortSpinBox->setProperty("value", portConfig["localPort"].toInt());
                m_udpSocketRemoteHostTextField->setProperty("text", portConfig["remoteHost"].toString());
                m_udpSocketRemotePortSpinBox->setProperty("value", portConfig["remotePort"].toInt());
                m_txFormatComboBox->setProperty("currentValue", portConfig["txFormat"].toString());
                m_txSuffixComboBox->setProperty("currentValue", portConfig["txSuffix"].toString());
                m_rxFormatComboBox->setProperty("currentValue", portConfig["rxFormat"].toString());
            }
            break;
            case VIDEOSTREAM: {
                m_videoStreamNameComboBox->setProperty("currentValue", portConfig["portName"].toString());
            }
            break;
            default: break;
        }
    }
    m_portSettingDialog->resize(600, 500);
    m_portSettingDialog->show();
}

void PortSetting::portSettingExport() {
    const int portType = m_rootItem->property("portType").toInt();
    QJsonObject portConfig{};
    switch (portType) {
        case SERIALPORT: {
            portConfig = {
                {"portType", portType},
                {"portName", m_serialPortNameComboBox->property("currentValue").toString()},
                {"baudRate", m_serialPortBaudRateSpinBox->property("value").toInt()},
                {"dataBits", m_serialPortDataBitsComboBox->property("currentValue").toInt()},
                {"parity", m_serialPortParityComboBox->property("currentValue").toInt()},
                {"stopBits", m_serialPortStopBitsComboBox->property("currentValue").toInt()},
                {"txFormat", m_txFormatComboBox->property("currentValue").toString()},
                {"txSuffix", m_txSuffixComboBox->property("currentValue").toString()},
                {"rxFormat", m_rxFormatComboBox->property("currentValue").toString()}
            };
        }
        break;
        case VISA: {
            portConfig = {
                {"portType", portType},
                {"portName", m_visaNameComboBox->property("currentValue").toString()},
                {"txFormat", m_txFormatComboBox->property("currentValue").toString()},
                {"txSuffix", m_txSuffixComboBox->property("currentValue").toString()},
                {"rxFormat", m_rxFormatComboBox->property("currentValue").toString()}
            };
        }
        break;
        case TCPCLIENT: {
            portConfig = {
                {"portType", portType},
                {"portName", m_tcpClientNameTextField->property("text").toString()},
                {"remoteHost", m_tcpClientRemoteHostTextField->property("text").toString()},
                {"remotePort", m_tcpClientRemotePortSpinBox->property("value").toInt()},
                {"txFormat", m_txFormatComboBox->property("currentValue").toString()},
                {"txSuffix", m_txSuffixComboBox->property("currentValue").toString()},
                {"rxFormat", m_rxFormatComboBox->property("currentValue").toString()}
            };
        }
        break;
        case SSLCLIENT: {
            portConfig = {
                {"portType", portType},
                {"portName", m_sslClientNameTextField->property("text").toString()},
                {"remoteHost", m_sslClientRemoteHostTextField->property("text").toString()},
                {"remotePort", m_sslClientRemotePortSpinBox->property("value").toInt()},
                {"txFormat", m_txFormatComboBox->property("currentValue").toString()},
                {"txSuffix", m_txSuffixComboBox->property("currentValue").toString()},
                {"rxFormat", m_rxFormatComboBox->property("currentValue").toString()}
            };
        }
        break;
        case TCPSERVER: {
            portConfig = {
                {"portType", portType},
                {"portName", m_tcpServerNameTextField->property("text").toString()},
                {"localHost", m_tcpServerLocalHostComboBox->property("currentValue").toString()},
                {"localPort", m_tcpServerLocalPortSpinBox->property("value").toInt()},
                {"txFormat", m_txFormatComboBox->property("currentValue").toString()},
                {"txSuffix", m_txSuffixComboBox->property("currentValue").toString()},
                {"rxFormat", m_rxFormatComboBox->property("currentValue").toString()}
            };
        }
        break;
        case UDPSOCKET: {
            portConfig = {
                {"portType", portType},
                {"portName", m_udpSocketNameTextField->property("text").toString()},
                {"localHost", m_udpSocketLocalHostComboBox->property("currentValue").toString()},
                {"localPort", m_udpSocketLocalPortSpinBox->property("value").toInt()},
                {"remoteHost", m_udpSocketRemoteHostTextField->property("text").toString()},
                {"remotePort", m_udpSocketRemotePortSpinBox->property("value").toInt()},
                {"txFormat", m_txFormatComboBox->property("currentValue").toString()},
                {"txSuffix", m_txSuffixComboBox->property("currentValue").toString()},
                {"rxFormat", m_rxFormatComboBox->property("currentValue").toString()}
            };
        }
        break;
        case VIDEOSTREAM: {
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
            for (int i = 0; i < m_roiStandardItemModel->rowCount(); ++i) {
                const QJsonArray roi = QJsonArray::fromVariantList(m_roiStandardItemModel->item(i, 0)->data(Qt::WhatsThisRole).toList());
                roiArray.append(roi);
            }
            QJsonArray pipelineArray{};
            for (int i = 0; i < m_pipelineStandardItemModel->rowCount(); ++i) {
                const QJsonObject session = QJsonObject::fromVariantHash(m_pipelineStandardItemModel->item(i, 0)->data(Qt::WhatsThisRole).toHash());
                pipelineArray.append(session);
            }
            portConfig = {
                {"portType", portType},
                {"portName", m_videoStreamNameComboBox->property("currentValue").toString()},
                {"roi", roiArray},
                {"pipeline", pipelineArray},
                {"whitelist", m_whitelistTextField->property("text").toString()}
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
    m_portSettingDialog->hide();
}

void PortSetting::dialogResize(const int width, const int height) const {
    m_portSettingDialog->resize(width, height);
}

void PortSetting::videoCapture() {
    m_portSettingDialog->resize(1600, 900);
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

void PortSetting::previewLoad(const int index) const {
    const QJsonArray roi = QJsonArray::fromVariantList(m_roiStandardItemModel->item(index, 0)->data(Qt::WhatsThisRole).toList());
    QJsonArray pipeline{};
    for (int i = 0; i < m_pipelineStandardItemModel->rowCount(); ++i) {
        const QJsonObject session = QJsonObject::fromVariantHash(m_pipelineStandardItemModel->item(i, 0)->data(Qt::WhatsThisRole).toHash());
        pipeline.append(session);
    }
    m_imageProvider->preview(m_videoSink, roi, pipeline);
    m_previewImage->setProperty("source", "image://capture/" + QString::number(QDateTime::currentMSecsSinceEpoch()));
    m_previewImage->setProperty("currentIndex", index);
}

void PortSetting::roiInsert(const int x, const int y, const int w, const int h) const {
    auto *item = new QStandardItem(QString::number(x) + " " + QString::number(y) + " " + QString::number(w) + " " + QString::number(h)); // NOLINT
    const QVariantList position = {x, y, w, h};
    m_roiStandardItemModel->appendRow(item);
    item->setData(position, Qt::WhatsThisRole);
    QMetaObject::invokeMethod(m_rootItem, "indicatorReload");
}

void PortSetting::roiRemove(const int index) const {
    m_roiStandardItemModel->removeRow(index);
    QMetaObject::invokeMethod(m_rootItem, "indicatorReload");
}

void PortSetting::roiSwap(const int src, const int dst) const {
    const auto tmp = m_roiStandardItemModel->takeRow(src);
    m_roiStandardItemModel->insertRow(dst, tmp);
    QMetaObject::invokeMethod(m_rootItem, "roiReload");
    QMetaObject::invokeMethod(m_rootItem, "indicatorReload");
}

void PortSetting::pipelineInsert(const QVariantHash &session) const {
    const int type = session["type"].toInt();
    switch (type) {
        case SCALE: {
            auto *item = new QStandardItem(tr("Scale")); // NOLINT
            m_pipelineStandardItemModel->appendRow(item);
            item->setData(session, Qt::WhatsThisRole);
        }
        break;
        case THRESHOLD: {
            auto *item = new QStandardItem(tr("Threshold")); // NOLINT
            m_pipelineStandardItemModel->appendRow(item);
            item->setData(session, Qt::WhatsThisRole);
        }
        break;
        default: break;
    }
}

void PortSetting::pipelineRemove(const int index) const {
    m_pipelineStandardItemModel->removeRow(index);
}

void PortSetting::pipelineSwap(const int src, const int dst) const {
    const auto tmp = m_pipelineStandardItemModel->takeRow(src);
    m_pipelineStandardItemModel->insertRow(dst, tmp);
    QMetaObject::invokeMethod(m_rootItem, "pipelineReload");
}

// PortSetting private
void PortSetting::serialPortRefresh() const {
    m_serialPortStandardItemModel->clear();
    for (QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts(); const QSerialPortInfo &port: ports) {
        const QString portName = port.portName();
        auto *item = new QStandardItem(portName + " " + port.description());
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
            auto *item = new QStandardItem(portName);
            item->setData(portName, Qt::WhatsThisRole);
            m_localHostStandardItemModel->appendRow(item);
        } else if (address.protocol() == QAbstractSocket::IPv6Protocol) {
            auto portName = address.toString();
            // const QString scopeId = address.scopeId();
            // if (!scopeId.isEmpty()) portName.remove("%"+scopeId);
            auto *item = new QStandardItem(portName);
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
    m_roiStandardItemModel->clear();
    m_pipelineStandardItemModel->clear();
    QMetaObject::invokeMethod(m_rootItem, "indicatorReload");
    m_whitelistSwitch->setProperty("checked", false);
    m_whitelistTextField->setProperty("text", "");
    if (!portConfig.isEmpty()) {
        // roi
        for (const QJsonValue &value: portConfig["roi"].toArray()) {
            QJsonArray roi = value.toArray();
            roiInsert(roi[0].toInt(), roi[1].toInt(), roi[2].toInt(), roi[3].toInt());
        }
        for (const QJsonValue &value: portConfig["pipeline"].toArray()) {
            QVariantHash session = value.toObject().toVariantHash();
            pipelineInsert(session);
        }
        // whitelist
        const QString whitelist = portConfig["whitelist"].toString();
        if (!whitelist.isEmpty()) {
            m_whitelistSwitch->setProperty("checked", true);
            m_whitelistTextField->setProperty("text", whitelist);
        }
    }
}

// ImageProvider public
ImageProvider::ImageProvider()
    : QQuickImageProvider(Pixmap) {
}

QPixmap ImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) {
    if (m_preview.isNull()) return {};
    return m_preview;
}

void ImageProvider::preview(const QVideoSink* videoSink, const QJsonArray &roi, const QJsonArray &pipeline) {
    const auto videoFrame = videoSink->videoFrame();
    const auto image = videoFrame.toImage();
    const auto pixmap = QPixmap::fromImage(image);
    const auto rect = QRect(roi[0].toInt(), roi[1].toInt(), roi[2].toInt(), roi[3].toInt());
    const QPixmap cropped = pixmap.copy(rect);
    m_preview = processPipeline(cropped, pipeline);
}
