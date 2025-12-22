#include "portModule/portSetting/portSetting.h"

#include <QCameraDevice>
#include <QHostInfo>
#include <QJsonObject>
#include <QMediaDevices>
#include <QQmlContext>
#include <QScreen>
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
      m_localHostStandardItemModel(new QStandardItemModel(this)),
      m_visaStandardItemModel(new QStandardItemModel(this)) {
    propertySet();
}

void PortSetting::propertySet() {
    m_portSettingDialog->setWindowTitle(tr("Port Setting"));
    auto *layout = new QVBoxLayout(m_portSettingDialog); // NOLINT
    auto *widget = new QQuickWidget(); // NOLINT
    layout->addWidget(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    widget->rootContext()->setContextProperty("portSetting", this);
    widget->rootContext()->setContextProperty("serialPortStandardItemModel", m_serialPortStandardItemModel);
    widget->rootContext()->setContextProperty("visaStandardItemModel", m_visaStandardItemModel);
    widget->rootContext()->setContextProperty("localHostStandardItemModel", m_localHostStandardItemModel);
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
    // format
    m_txFormatComboBox = qvariant_cast<QObject *>(objects["txFormatComboBox"]);
    m_txSuffixComboBox = qvariant_cast<QObject *>(objects["txSuffixComboBox"]);
    m_rxFormatComboBox = qvariant_cast<QObject *>(objects["rxFormatComboBox"]);
}

void PortSetting::portSettingImport(const QJsonObject &portConfig) {
    serialPortRefresh();
    visaRefresh();
    localHostRefresh();
    m_swipeView->setProperty("currentIndex", 0);
    if (portConfig.isEmpty()) {
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
        // format
        m_txFormatComboBox->setProperty("currentValue", "hex");
        m_txSuffixComboBox->setProperty("currentValue", "null");
        m_rxFormatComboBox->setProperty("currentValue", "hex");
    } else {
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
            }
            break;
            case CAMERA: {
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
                {"portType", portType}
            };
        }
        break;
        case CAMERA: {
            portConfig = {
                {"portType", portType}
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
    for (const QHostAddress &address : QHostInfo::fromName(QHostInfo::localHostName()).addresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            auto *item = new QStandardItem(address.toString());
            item->setData(address.toString(), Qt::WhatsThisRole);
            m_localHostStandardItemModel->appendRow(item);
        }
    }
}

//     for (const QList<QScreen *> screens = QGuiApplication::screens(); const QScreen *screen: screens) {
//         m_screenNameCombobox->addItem(screen->name());
//     }
//     for (const QList<QCameraDevice> cameras = QMediaDevices::videoInputs(); const QCameraDevice &camera: cameras) {
//         m_cameraNameCombobox->addItem(camera.description());
//     }
