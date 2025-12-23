#include "portModule/portSetting/portSetting.h"

#include <QCamera>
#include <QCameraDevice>
#include <QHostInfo>
#include <QImageCapture>
#include <QJsonObject>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QQmlContext>
#include <QSerialPortInfo>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <visa.h>

#include "globals.h"
#include "portModule/areaSelection.h"

// PortSetting public
PortSetting::PortSetting(QWidget *parent)
    : QWidget(parent),
      m_portSettingDialog(new QDialog(this)),
      m_serialPortStandardItemModel(new QStandardItemModel(this)),
      m_visaStandardItemModel(new QStandardItemModel(this)),
      m_localHostStandardItemModel(new QStandardItemModel(this)),
      m_screenStandardItemModel(new QStandardItemModel(this)),
      m_cameraStandardItemModel(new QStandardItemModel(this)),
      m_imageProvider(new ImageProvider()) {
    propertySet();
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
    widget->rootContext()->setContextProperty("screenStandardItemModel", m_screenStandardItemModel);
    widget->rootContext()->setContextProperty("cameraStandardItemModel", m_cameraStandardItemModel);
    widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    widget->setSource(QUrl("qrc:/qml/portModule/portSetting/portSetting.qml"));
}

void PortSetting::propertyGet(const QVariantMap &objects) {
    m_swipeView = qvariant_cast<QObject *>(objects["swipeView"]);
    m_tumbler = qvariant_cast<QObject *>(objects["tumbler"]);
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
    // screen
    m_screenNameComboBox = qvariant_cast<QObject *>(objects["screenNameComboBox"]);
    // camera
    m_cameraNameComboBox = qvariant_cast<QObject *>(objects["cameraNameComboBox"]);
    // format
    m_txFormatComboBox = qvariant_cast<QObject *>(objects["txFormatComboBox"]);
    m_txSuffixComboBox = qvariant_cast<QObject *>(objects["txSuffixComboBox"]);
    m_rxFormatComboBox = qvariant_cast<QObject *>(objects["rxFormatComboBox"]);
}

void PortSetting::portSettingImport(const QJsonObject &portConfig) {
    serialPortRefresh();
    visaRefresh();
    localHostRefresh();
    screenRefresh();
    cameraRefresh();
    if (portConfig.isEmpty()) {
        m_swipeView->setProperty("currentIndex", 0);
        m_oldPortName = "";
        m_tumbler->setProperty("currentIndex", 0);
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
        // screen
        if (m_screenNameComboBox->property("count").toInt()) {
            m_screenNameComboBox->setProperty("currentIndex", 0);
        }
        // screen
        if (m_cameraNameComboBox->property("count").toInt()) {
            m_cameraNameComboBox->setProperty("currentIndex", 0);
        }
        // format
        m_txFormatComboBox->setProperty("currentValue", "hex");
        m_txSuffixComboBox->setProperty("currentValue", "null");
        m_rxFormatComboBox->setProperty("currentValue", "hex");
    } else {
        m_swipeView->setProperty("currentIndex", 1);
        m_oldPortName = portConfig["portName"].toString();
        const int portType = portConfig["portType"].toInt();
        m_tumbler->setProperty("currentIndex", portType);
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
            case SCREEN: {
                m_screenNameComboBox->setProperty("currentValue", portConfig["portName"].toString());
            }
            break;
            case CAMERA: {
                m_cameraNameComboBox->setProperty("currentValue", portConfig["portName"].toString());
            }
            break;
            default: break;
        }
    }
    m_portSettingDialog->resize(600, 500);
    m_portSettingDialog->show();
}

void PortSetting::portSettingExport() {
    const int portType = m_tumbler->property("currentIndex").toInt();
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
        case TCPSERVER: {
            portConfig = {
                {"portType", portType},
                {"portName", m_tcpClientNameTextField->property("text").toString()},
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
        case SCREEN: {
            portConfig = {
                {"portType", portType},
                {"portName", m_screenNameComboBox->property("currentValue").toString()},
            };
        }
        break;
        case CAMERA: {
            portConfig = {
                {"portType", portType},
                {"portName", m_cameraNameComboBox->property("currentValue").toString()},
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

void PortSetting::screenCapture() const {
    const auto &portName = m_screenNameComboBox->property("currentValue").toString();
    m_imageProvider->screenCapture(portName);
}

void PortSetting::cameraCapture() const {
    const auto &portName = m_cameraNameComboBox->property("currentValue").toString();
    m_imageProvider->cameraCapture(portName);
}

void PortSetting::dialogResize(const int width, const int height) const {
    m_portSettingDialog->resize(width, height);
}

// PortSetting private
void PortSetting::serialPortRefresh() const {
    m_serialPortStandardItemModel->clear();
    for (QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts(); const QSerialPortInfo &port: ports) {
        auto *item = new QStandardItem(port.portName() + " " + port.description());
        item->setData(port.portName(), Qt::WhatsThisRole);
        m_serialPortStandardItemModel->appendRow(item);
    }
}

void PortSetting::visaRefresh() const {
    // QStringList deviceList;
    //
    // ViFindList findList;
    // ViUInt32 numInst;
    // ViChar portName[VI_FIND_BUFLEN];
    //
    // ViStatus status = viOpenDefaultRM(&g_rm);
    // if (status != VI_SUCCESS) {
    //     qDebug() << "fails to start visa resource manager";
    //     return deviceList;
    // }
    //
    // status = viFindRsrc(g_rm, "?*INSTR", &findList, &numInst, portName);
    // if (status != VI_SUCCESS) {
    //     qDebug() << "fails to search visa instruments";
    //     viClose(g_rm);
    //     return deviceList;
    // }
    //
    // deviceList.append(QString(portName));
    //
    // for (ViUInt32 i = 1; i < numInst; i++) {
    //     status = viFindNext(findList, portName);
    //     if (status == VI_SUCCESS) {
    //         deviceList.append(QString(portName));
    //     }
    // }
    //
    // // viClose(findList);
    // // viClose(rm);
    //
    // return deviceList;
}

void PortSetting::localHostRefresh() const {
    m_localHostStandardItemModel->clear();
    for (const QHostAddress &address: QHostInfo::fromName(QHostInfo::localHostName()).addresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            auto *item = new QStandardItem(address.toString());
            item->setData(address.toString(), Qt::WhatsThisRole);
            m_localHostStandardItemModel->appendRow(item);
        }
    }
}

void PortSetting::screenRefresh() const {
    m_screenStandardItemModel->clear();
    for (const QScreen *screen: QGuiApplication::screens()) {
        auto *item = new QStandardItem(screen->name());
        item->setData(screen->name(), Qt::WhatsThisRole);
        m_screenStandardItemModel->appendRow(item);
    }
}

void PortSetting::cameraRefresh() const {
    m_cameraStandardItemModel->clear();
    for (const QCameraDevice &camera: QMediaDevices::videoInputs()) {
        auto *item = new QStandardItem(camera.description());
        item->setData(camera.description(), Qt::WhatsThisRole);
        m_cameraStandardItemModel->appendRow(item);
    }
}

ImageProvider::ImageProvider()
    : QQuickImageProvider(Pixmap) {
}

QPixmap ImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) {
    if (m_capture.isNull()) return {};
    return m_capture;
}

void ImageProvider::screenCapture(const QString &portName) {
    QScreen *screen = nullptr;
    for (QScreen *s: QGuiApplication::screens()) {
        if (s->name() == portName) {
            screen = s;
            break;
        }
    }
    if (!screen) return;
    // capture
    m_capture = screen->grabWindow(0);
}

void ImageProvider::cameraCapture(const QString &portName) {
    QCameraDevice cameraDevice;
    for (const QCameraDevice &c: QMediaDevices::videoInputs()) {
        if (c.description() == portName) {
            cameraDevice = c;
            break;
        }
    }
    if (cameraDevice.isNull()) return;
    // capture
    const auto camera = new QCamera(cameraDevice, this);
    QMediaCaptureSession captureSession;
    captureSession.setCamera(camera);
    QImageCapture imageCapture;
    captureSession.setImageCapture(&imageCapture);
    QEventLoop loop;
    connect(&imageCapture, &QImageCapture::imageCaptured, this, [this, &loop](int, const QImage &img) {
        m_capture = QPixmap::fromImage(img);
        loop.quit();
    });
    camera->start();
    imageCapture.capture();
    loop.exec();
    camera->stop();
    delete camera;
}
