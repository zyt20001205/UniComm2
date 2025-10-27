#include "portModule/portSetting.h"

#include <QCameraDevice>
#include <QComboBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMediaDevices>
#include <QMessageBox>
#include <QPushButton>
#include <QScreen>
#include <QSerialPortInfo>
#include <QSpinBox>
#include <visa.h>

#include "globals.h"
#include "portModule/areaSelection.h"

// PortSetting public
PortSetting::PortSetting(QSet<QString> portUsedName, QWidget *parent)
    : QDialog(parent),
      m_portUsedName(portUsedName),
      m_portSettingLayout(new QVBoxLayout(this)),
      m_portTypeWidget(new QWidget()),
      m_portTypeCombobox(new QComboBox()),
      m_serialPortNameWidget(new QWidget()),
      m_serialPortNameCombobox(new QComboBox()),
      m_serialPortBaudRateWidget(new QWidget()),
      m_serialPortBaudRateSpinBox(new QSpinBox()),
      m_serialPortDataBitsWidget(new QWidget()),
      m_serialPortDataBitsCombobox(new QComboBox()),
      m_serialPortParityWidget(new QWidget()),
      m_serialPortParityCombobox(new QComboBox()),
      m_serialPortStopBitsWidget(new QWidget()),
      m_serialPortStopBitsCombobox(new QComboBox()),
      m_visaNameWidget(new QWidget()),
      m_visaNameCombobox(new QComboBox()),
      m_tcpClientNameWidget(new QWidget()),
      m_tcpClientNameLineEdit(new QLineEdit()),
      m_tcpClientRemoteAddressWidget(new QWidget()),
      m_tcpClientRemoteAddressLineEdit(new QLineEdit()),
      m_tcpClientRemotePortWidget(new QWidget()),
      m_tcpClientRemotePortSpinBox(new QSpinBox()),
      m_tcpServerNameWidget(new QWidget()),
      m_tcpServerNameLineEdit(new QLineEdit()),
      m_tcpServerLocalAddressWidget(new QWidget()),
      m_tcpServerLocalAddressLineEdit(new QLineEdit()),
      m_tcpServerLocalPortWidget(new QWidget()),
      m_tcpServerLocalPortSpinBox(new QSpinBox()),
      m_udpSocketNameWidget(new QWidget()),
      m_udpSocketNameLineEdit(new QLineEdit()),
      m_udpSocketLocalAddressWidget(new QWidget()),
      m_udpSocketLocalAddressLineEdit(new QLineEdit()),
      m_udpSocketLocalPortWidget(new QWidget()),
      m_udpSocketLocalPortSpinBox(new QSpinBox()),
      m_udpSocketRemoteAddressWidget(new QWidget()),
      m_udpSocketRemoteAddressLineEdit(new QLineEdit()),
      m_udpSocketRemotePortWidget(new QWidget()),
      m_udpSocketRemotePortSpinBox(new QSpinBox()),
      m_screenNameWidget(new QWidget()),
      m_screenNameCombobox(new QComboBox()),
      m_cameraNameWidget(new QWidget()),
      m_cameraNameCombobox(new QComboBox()),
      m_areaSelectionWidget(new QWidget()),
      m_areaSelectionPushButton(new QPushButton(tr("Choose Capture Area"))),
      m_areaSelectionDialog(new AreaSelection(this)),
      m_txFormatWidget(new QWidget()),
      m_txFormatCombobox(new QComboBox()),
      m_txSuffixWidget(new QWidget()),
      m_txSuffixCombobox(new QComboBox()),
      m_rxFormatWidget(new QWidget()),
      m_rxFormatCombobox(new QComboBox()),
      m_portSettingSavePushButton(new QPushButton(tr("Save Setting"))) {
    // setting dialog & port type combobox
    {
        this->setFixedWidth(600);
        m_portSettingLayout->addWidget(m_portTypeWidget);
        const auto portTypeLayout = new QHBoxLayout(m_portTypeWidget); // NOLINT
        portTypeLayout->setContentsMargins(0, 0, 0, 0);
        const auto portTypeLabel = new QLabel("port type"); // NOLINT
        portTypeLayout->addWidget(portTypeLabel);
        portTypeLayout->addWidget(m_portTypeCombobox);
        m_portTypeCombobox->addItem(tr("Choose Port Type"));
        m_portTypeCombobox->addItem(tr("Serial Port"));
        m_portTypeCombobox->addItem(tr("Visa"));
        m_portTypeCombobox->addItem(tr("TCP Client"));
        m_portTypeCombobox->addItem(tr("TCP Server"));
        m_portTypeCombobox->addItem(tr("UDP Socket"));
        m_portTypeCombobox->addItem(tr("Screen"));
        m_portTypeCombobox->addItem(tr("Camera"));
        connect(m_portTypeCombobox, &QComboBox::currentIndexChanged, this, &PortSetting::portSettingTypeSwitch);
    }
    // serial port settings
    {
        m_portSettingLayout->addWidget(m_serialPortNameWidget);
        const auto serialPortNameLayout = new QHBoxLayout(m_serialPortNameWidget); // NOLINT
        serialPortNameLayout->setContentsMargins(0, 0, 0, 0);
        const auto serialPortNameLabel = new QLabel("port name"); // NOLINT
        serialPortNameLayout->addWidget(serialPortNameLabel);
        serialPortNameLayout->addWidget(m_serialPortNameCombobox);

        m_portSettingLayout->addWidget(m_serialPortBaudRateWidget);
        const auto serialPortBaudRateLayout = new QHBoxLayout(m_serialPortBaudRateWidget); // NOLINT
        serialPortBaudRateLayout->setContentsMargins(0, 0, 0, 0);
        const auto serialPortBaudRateLabel = new QLabel("baud rate"); // NOLINT
        serialPortBaudRateLayout->addWidget(serialPortBaudRateLabel);
        serialPortBaudRateLayout->addWidget(m_serialPortBaudRateSpinBox);
        m_serialPortBaudRateSpinBox->setRange(1, 5000000);

        m_portSettingLayout->addWidget(m_serialPortDataBitsWidget);
        const auto serialPortDataBitsLayout = new QHBoxLayout(m_serialPortDataBitsWidget); // NOLINT
        serialPortDataBitsLayout->setContentsMargins(0, 0, 0, 0);
        const auto serialPortDataBitsLabel = new QLabel("databits"); // NOLINT
        serialPortDataBitsLayout->addWidget(serialPortDataBitsLabel);
        serialPortDataBitsLayout->addWidget(m_serialPortDataBitsCombobox);
        m_serialPortDataBitsCombobox->addItem("5", 5);
        m_serialPortDataBitsCombobox->addItem("6", 6);
        m_serialPortDataBitsCombobox->addItem("7", 7);
        m_serialPortDataBitsCombobox->addItem("8", 8);

        m_portSettingLayout->addWidget(m_serialPortParityWidget);
        const auto serialPortParityLayout = new QHBoxLayout(m_serialPortParityWidget); // NOLINT
        serialPortParityLayout->setContentsMargins(0, 0, 0, 0);
        const auto serialPortParityLabel = new QLabel("parity"); // NOLINT
        serialPortParityLayout->addWidget(serialPortParityLabel);
        serialPortParityLayout->addWidget(m_serialPortParityCombobox);
        m_serialPortParityCombobox->addItem("no", 0);
        m_serialPortParityCombobox->addItem("even", 2);
        m_serialPortParityCombobox->addItem("odd", 3);
        m_serialPortParityCombobox->addItem("space", 4);
        m_serialPortParityCombobox->addItem("mark", 5);

        m_portSettingLayout->addWidget(m_serialPortStopBitsWidget);
        const auto serialPortStopBitsLayout = new QHBoxLayout(m_serialPortStopBitsWidget); // NOLINT
        serialPortStopBitsLayout->setContentsMargins(0, 0, 0, 0);
        const auto serialPortStopBitsLabel = new QLabel("stop bits"); // NOLINT
        serialPortStopBitsLayout->addWidget(serialPortStopBitsLabel);
        serialPortStopBitsLayout->addWidget(m_serialPortStopBitsCombobox);
        m_serialPortStopBitsCombobox->addItem("1", 1);
        m_serialPortStopBitsCombobox->addItem("1.5", 3);
        m_serialPortStopBitsCombobox->addItem("2", 2);
    }
    // visa settings
    {
        m_portSettingLayout->addWidget(m_visaNameWidget);
        const auto visaNameLayout = new QHBoxLayout(m_visaNameWidget); // NOLINT
        visaNameLayout->setContentsMargins(0, 0, 0, 0);
        const auto visaNameLabel = new QLabel("port name"); // NOLINT
        visaNameLayout->addWidget(visaNameLabel);
        visaNameLayout->addWidget(m_visaNameCombobox);
    }
    // tcp client settings
    {
        m_portSettingLayout->addWidget(m_tcpClientNameWidget);
        const auto tcpClientNameLayout = new QHBoxLayout(m_tcpClientNameWidget); // NOLINT
        tcpClientNameLayout->setContentsMargins(0, 0, 0, 0);
        const auto tcpClientNameLabel = new QLabel("port name"); // NOLINT
        tcpClientNameLayout->addWidget(tcpClientNameLabel);
        tcpClientNameLayout->addWidget(m_tcpClientNameLineEdit);

        m_portSettingLayout->addWidget(m_tcpClientRemoteAddressWidget);
        const auto tcpClientRemoteAddressLayout = new QHBoxLayout(m_tcpClientRemoteAddressWidget); // NOLINT
        tcpClientRemoteAddressLayout->setContentsMargins(0, 0, 0, 0);
        const auto tcpClientRemoteAddressLabel = new QLabel("remote adress"); // NOLINT
        tcpClientRemoteAddressLayout->addWidget(tcpClientRemoteAddressLabel);
        tcpClientRemoteAddressLayout->addWidget(m_tcpClientRemoteAddressLineEdit);

        m_portSettingLayout->addWidget(m_tcpClientRemotePortWidget);
        const auto tcpClientRemotePortLayout = new QHBoxLayout(m_tcpClientRemotePortWidget); // NOLINT
        tcpClientRemotePortLayout->setContentsMargins(0, 0, 0, 0);
        const auto tcpClientRemotePortLabel = new QLabel("remote port"); // NOLINT
        tcpClientRemotePortLayout->addWidget(tcpClientRemotePortLabel);
        tcpClientRemotePortLayout->addWidget(m_tcpClientRemotePortSpinBox);
        m_tcpClientRemotePortSpinBox->setRange(0, 65536);
    }
    // tcp server settings
    {
        m_portSettingLayout->addWidget(m_tcpServerNameWidget);
        const auto tcpServerNameLayout = new QHBoxLayout(m_tcpServerNameWidget); // NOLINT
        tcpServerNameLayout->setContentsMargins(0, 0, 0, 0);
        const auto tcpServerNameLabel = new QLabel("port name"); // NOLINT
        tcpServerNameLayout->addWidget(tcpServerNameLabel);
        tcpServerNameLayout->addWidget(m_tcpServerNameLineEdit);

        m_portSettingLayout->addWidget(m_tcpServerLocalAddressWidget);
        const auto tcpServerLocalAddressLayout = new QHBoxLayout(m_tcpServerLocalAddressWidget); // NOLINT
        tcpServerLocalAddressLayout->setContentsMargins(0, 0, 0, 0);
        const auto tcpServerLocalAddressLabel = new QLabel("local adress"); // NOLINT
        tcpServerLocalAddressLayout->addWidget(tcpServerLocalAddressLabel);
        tcpServerLocalAddressLayout->addWidget(m_tcpServerLocalAddressLineEdit);

        m_portSettingLayout->addWidget(m_tcpServerLocalPortWidget);
        const auto tcpServerLocalPortLayout = new QHBoxLayout(m_tcpServerLocalPortWidget); // NOLINT
        tcpServerLocalPortLayout->setContentsMargins(0, 0, 0, 0);
        const auto tcpServerLocalPortLabel = new QLabel("local port"); // NOLINT
        tcpServerLocalPortLayout->addWidget(tcpServerLocalPortLabel);
        tcpServerLocalPortLayout->addWidget(m_tcpServerLocalPortSpinBox);
        m_tcpServerLocalPortSpinBox->setRange(0, 65536);
    }
    // udp socket settings
    {
        m_portSettingLayout->addWidget(m_udpSocketNameWidget);
        const auto udpSocketNameLayout = new QHBoxLayout(m_udpSocketNameWidget); // NOLINT
        udpSocketNameLayout->setContentsMargins(0, 0, 0, 0);
        const auto udpSocketNameLabel = new QLabel("port name"); // NOLINT
        udpSocketNameLayout->addWidget(udpSocketNameLabel);
        udpSocketNameLayout->addWidget(m_udpSocketNameLineEdit);

        m_portSettingLayout->addWidget(m_udpSocketLocalAddressWidget);
        const auto udpSocketLocalAddressLayout = new QHBoxLayout(m_udpSocketLocalAddressWidget); // NOLINT
        udpSocketLocalAddressLayout->setContentsMargins(0, 0, 0, 0);
        const auto udpSocketLocalAddressLabel = new QLabel("local adress"); // NOLINT
        udpSocketLocalAddressLayout->addWidget(udpSocketLocalAddressLabel);
        udpSocketLocalAddressLayout->addWidget(m_udpSocketLocalAddressLineEdit);

        m_portSettingLayout->addWidget(m_udpSocketLocalPortWidget);
        const auto udpSocketLocalPortLayout = new QHBoxLayout(m_udpSocketLocalPortWidget); // NOLINT
        udpSocketLocalPortLayout->setContentsMargins(0, 0, 0, 0);
        const auto udpSocketLocalPortLabel = new QLabel("local port"); // NOLINT
        udpSocketLocalPortLayout->addWidget(udpSocketLocalPortLabel);
        udpSocketLocalPortLayout->addWidget(m_udpSocketLocalPortSpinBox);
        m_udpSocketLocalPortSpinBox->setRange(0, 65536);

        m_portSettingLayout->addWidget(m_udpSocketRemoteAddressWidget);
        const auto udpSocketRemoteAddressLayout = new QHBoxLayout(m_udpSocketRemoteAddressWidget); // NOLINT
        udpSocketRemoteAddressLayout->setContentsMargins(0, 0, 0, 0);
        const auto udpSocketRemoteAddressLabel = new QLabel("remote adress"); // NOLINT
        udpSocketRemoteAddressLayout->addWidget(udpSocketRemoteAddressLabel);
        udpSocketRemoteAddressLayout->addWidget(m_udpSocketRemoteAddressLineEdit);

        m_portSettingLayout->addWidget(m_udpSocketRemotePortWidget);
        const auto udpSocketRemotePortLayout = new QHBoxLayout(m_udpSocketRemotePortWidget); // NOLINT
        udpSocketRemotePortLayout->setContentsMargins(0, 0, 0, 0);
        const auto udpSocketRemotePortLabel = new QLabel("remote port"); // NOLINT
        udpSocketRemotePortLayout->addWidget(udpSocketRemotePortLabel);
        udpSocketRemotePortLayout->addWidget(m_udpSocketRemotePortSpinBox);
        m_udpSocketRemotePortSpinBox->setRange(0, 65536);
    }
    // screen & camera settings
    {
        m_portSettingLayout->addWidget(m_screenNameWidget);
        const auto screenLayout = new QHBoxLayout(m_screenNameWidget); // NOLINT
        screenLayout->setContentsMargins(0, 0, 0, 0);
        const auto screenNameLabel = new QLabel("screen name"); // NOLINT
        screenLayout->addWidget(screenNameLabel);
        screenLayout->addWidget(m_screenNameCombobox);

        m_portSettingLayout->addWidget(m_cameraNameWidget);
        const auto cameraLayout = new QHBoxLayout(m_cameraNameWidget); // NOLINT
        cameraLayout->setContentsMargins(0, 0, 0, 0);
        const auto cameraNameLabel = new QLabel("camera name"); // NOLINT
        cameraLayout->addWidget(cameraNameLabel);
        cameraLayout->addWidget(m_cameraNameCombobox);

        m_portSettingLayout->addWidget(m_areaSelectionWidget);
        const auto screenAreaLayout = new QHBoxLayout(m_areaSelectionWidget); // NOLINT
        screenAreaLayout->setContentsMargins(0, 0, 0, 0);
        const auto screenAreaLabel = new QLabel("capture area"); // NOLINT
        screenAreaLayout->addWidget(screenAreaLabel);
        screenAreaLayout->addWidget(m_areaSelectionPushButton);
    }
    // tx/rx settings
    {
        m_portSettingLayout->addWidget(m_txFormatWidget);
        const auto txFormatLayout = new QHBoxLayout(m_txFormatWidget); // NOLINT
        txFormatLayout->setContentsMargins(0, 0, 0, 0);
        const auto txFormatLabel = new QLabel("tx format"); // NOLINT
        txFormatLayout->addWidget(txFormatLabel);
        txFormatLayout->addWidget(m_txFormatCombobox);
        m_txFormatCombobox->addItems(QStringList{"raw", "hex", "ascii", "utf-8"});

        m_portSettingLayout->addWidget(m_txSuffixWidget);
        const auto txSuffixLayout = new QHBoxLayout(m_txSuffixWidget); // NOLINT
        txSuffixLayout->setContentsMargins(0, 0, 0, 0);
        const auto txSuffixLabel = new QLabel("tx suffix"); // NOLINT
        txSuffixLayout->addWidget(txSuffixLabel);
        txSuffixLayout->addWidget(m_txSuffixCombobox);
        m_txSuffixCombobox->addItems(QStringList{"null", "crlf", "crc16 modbus"});

        m_portSettingLayout->addWidget(m_rxFormatWidget);
        const auto rxFormatLayout = new QHBoxLayout(m_rxFormatWidget); // NOLINT
        rxFormatLayout->setContentsMargins(0, 0, 0, 0);
        const auto rxFormatLabel = new QLabel("rx format"); // NOLINT
        rxFormatLayout->addWidget(rxFormatLabel);
        rxFormatLayout->addWidget(m_rxFormatCombobox);
        m_rxFormatCombobox->addItems(QStringList{"raw", "hex", "ascii", "utf-8"});
    }
    // init setting save button
    {
        m_portSettingLayout->addWidget(m_portSettingSavePushButton);
        connect(m_portSettingSavePushButton, &QPushButton::clicked, this, [this] {
            portSettingSave(m_portTypeCombobox->currentIndex());
        });
    }
    this->portSettingHideAll();
    this->adjustSize();
}

void PortSetting::portSettingImport(const QJsonObject &portConfig) {
    const int portType = portConfig["portType"].toInt();
    m_portTypeCombobox->setCurrentIndex(portType);
    m_portTypeCombobox->setEnabled(false);
    switch (portType) {
        case SERIALPORT: {
            int i = m_serialPortNameCombobox->findData(portConfig["portName"].toString());
            m_serialPortNameCombobox->setCurrentIndex(i);
            m_serialPortBaudRateSpinBox->setValue(portConfig["baudRate"].toInt());
            i = m_serialPortDataBitsCombobox->findData(portConfig["dataBits"].toInt());
            m_serialPortDataBitsCombobox->setCurrentIndex(i);
            i = m_serialPortParityCombobox->findData(portConfig["parity"].toInt());
            m_serialPortParityCombobox->setCurrentIndex(i);
            i = m_serialPortStopBitsCombobox->findData(portConfig["stopBits"].toInt());
            m_serialPortStopBitsCombobox->setCurrentIndex(i);
            m_txFormatCombobox->setCurrentText(portConfig["txFormat"].toString());
            m_txSuffixCombobox->setCurrentText(portConfig["txSuffix"].toString());
            m_rxFormatCombobox->setCurrentText(portConfig["rxFormat"].toString());
            break;
        }
        case VISA: {
            const int i = m_visaNameCombobox->findData(portConfig["portName"].toString());
            m_visaNameCombobox->setCurrentIndex(i);
            m_txFormatCombobox->setCurrentText(portConfig["txFormat"].toString());
            m_txSuffixCombobox->setCurrentText(portConfig["txSuffix"].toString());
            m_rxFormatCombobox->setCurrentText(portConfig["rxFormat"].toString());
            break;
        }
        case TCPCLIENT: {
            m_tcpClientNameLineEdit->setText(portConfig["portName"].toString());
            m_tcpClientRemoteAddressLineEdit->setText(portConfig["tcpClientRemoteAddress"].toString());
            m_tcpClientRemotePortSpinBox->setValue(portConfig["tcpClientRemotePort"].toInt());
            m_txFormatCombobox->setCurrentText(portConfig["txFormat"].toString());
            m_txSuffixCombobox->setCurrentText(portConfig["txSuffix"].toString());
            m_rxFormatCombobox->setCurrentText(portConfig["rxFormat"].toString());
            break;
        }
        case TCPSERVER: {
            m_tcpServerNameLineEdit->setText(portConfig["portName"].toString());
            m_tcpServerLocalAddressLineEdit->setText(portConfig["tcpServerLocalAddress"].toString());
            m_tcpServerLocalPortSpinBox->setValue(portConfig["tcpServerLocalPort"].toInt());
            m_txFormatCombobox->setCurrentText(portConfig["txFormat"].toString());
            m_txSuffixCombobox->setCurrentText(portConfig["txSuffix"].toString());
            m_rxFormatCombobox->setCurrentText(portConfig["rxFormat"].toString());
            break;
        }
        case UDPSOCKET: {
            m_udpSocketNameLineEdit->setText(portConfig["portName"].toString());
            m_udpSocketLocalAddressLineEdit->setText(portConfig["udpSocketLocalAddress"].toString());
            m_udpSocketLocalPortSpinBox->setValue(portConfig["udpSocketLocalPort"].toInt());
            m_udpSocketRemoteAddressLineEdit->setText(portConfig["udpSocketRemoteAddress"].toString());
            m_udpSocketRemotePortSpinBox->setValue(portConfig["udpSocketRemotePort"].toInt());
            m_txFormatCombobox->setCurrentText(portConfig["txFormat"].toString());
            m_txSuffixCombobox->setCurrentText(portConfig["txSuffix"].toString());
            m_rxFormatCombobox->setCurrentText(portConfig["rxFormat"].toString());
            break;
        }
        case SCREEN: {
            m_screenNameCombobox->setCurrentText(portConfig["portName"].toString());
            m_areaSelectionDialog->show();
            m_areaSelectionDialog->reload(portConfig);
            m_areaSelectionDialog->captureRequest("screen", m_screenNameCombobox->currentText());
            break;
        }
        case CAMERA: {
            m_cameraNameCombobox->setCurrentText(portConfig["portName"].toString());
            m_areaSelectionDialog->show();
            m_areaSelectionDialog->reload(portConfig);
            m_areaSelectionDialog->captureRequest("camera", m_cameraNameCombobox->currentText());
            break;
        }
        default: {
            qDebug() << "unknown port type";
            break;
        }
    }
    this->adjustSize();
}

// PortSetting private
void PortSetting::portSettingHideAll() {
    // serial port setting widget
    m_serialPortNameWidget->hide();
    m_serialPortNameCombobox->clear();
    for (QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts(); const QSerialPortInfo &port: ports) {
        m_serialPortNameCombobox->addItem(port.portName() + " " + port.description(), port.portName());
    }
    m_serialPortBaudRateWidget->hide();
    m_serialPortDataBitsWidget->hide();
    m_serialPortParityWidget->hide();
    m_serialPortStopBitsWidget->hide();
    // visa setting widget
    m_visaNameWidget->hide();
    m_visaNameCombobox->clear();
    for (QStringList devices = visaListGet(); const QString &device : devices) {
        m_visaNameCombobox->addItem(device, device);
    }
    // tcp client setting widget
    m_tcpClientNameWidget->hide();
    m_tcpClientNameLineEdit->setText("Tcp Client");
    m_tcpClientRemoteAddressWidget->hide();
    m_tcpClientRemotePortWidget->hide();
    // tcp server setting widget
    m_tcpServerNameWidget->hide();
    m_tcpServerNameLineEdit->setText("Tcp Server");
    m_tcpServerLocalAddressWidget->hide();
    m_tcpServerLocalPortWidget->hide();
    // udp socket setting widget
    m_udpSocketNameWidget->hide();
    m_udpSocketNameLineEdit->setText("Udp Socket");
    m_udpSocketLocalAddressWidget->hide();
    m_udpSocketLocalPortWidget->hide();
    m_udpSocketRemoteAddressWidget->hide();
    m_udpSocketRemotePortWidget->hide();
    // screen/camera setting widget
    m_screenNameWidget->hide();
    m_screenNameCombobox->clear();
    for (const QList<QScreen *> screens = QGuiApplication::screens(); const QScreen *screen: screens) {
        m_screenNameCombobox->addItem(screen->name());
    }
    m_cameraNameWidget->hide();
    m_cameraNameCombobox->clear();
    for (const QList<QCameraDevice> cameras = QMediaDevices::videoInputs(); const QCameraDevice &camera: cameras) {
        m_cameraNameCombobox->addItem(camera.description());
    }
    m_areaSelectionWidget->hide();
    // rx/tx setting widget
    m_txFormatWidget->hide();
    m_txSuffixWidget->hide();
    m_rxFormatWidget->hide();
    // save button
    m_portSettingSavePushButton->hide();
};

void PortSetting::portSettingTypeSwitch(const int portType) {
    portSettingHideAll();
    switch (portType) {
        case SERIALPORT: {
            m_serialPortNameWidget->show();
            m_serialPortBaudRateSpinBox->setValue(115200);
            m_serialPortBaudRateWidget->show();
            m_serialPortDataBitsCombobox->setCurrentText("8");
            m_serialPortDataBitsWidget->show();
            m_serialPortParityWidget->show();
            m_serialPortStopBitsWidget->show();
            m_txFormatWidget->show();
            m_txSuffixWidget->show();
            m_rxFormatWidget->show();
            m_portSettingSavePushButton->show();
            break;
        }
        case VISA: {
            m_visaNameWidget->show();
            break;
        }
        case TCPCLIENT: {
            m_tcpClientNameWidget->show();
            m_tcpClientRemoteAddressWidget->show();
            m_tcpClientRemotePortWidget->show();
            m_txFormatWidget->show();
            m_txSuffixWidget->show();
            m_rxFormatWidget->show();
            m_portSettingSavePushButton->show();
            break;
        }
        case TCPSERVER: {
            m_tcpServerNameWidget->show();
            m_tcpServerLocalAddressWidget->show();
            m_tcpServerLocalPortWidget->show();
            m_txFormatWidget->show();
            m_txSuffixWidget->show();
            m_rxFormatWidget->show();
            m_portSettingSavePushButton->show();
            break;
        }
        case UDPSOCKET: {
            m_udpSocketNameWidget->show();
            m_udpSocketLocalAddressWidget->show();
            m_udpSocketLocalPortWidget->show();
            m_udpSocketRemoteAddressWidget->show();
            m_udpSocketRemotePortWidget->show();
            m_txFormatWidget->show();
            m_txSuffixWidget->show();
            m_rxFormatWidget->show();
            m_portSettingSavePushButton->show();
            break;
        }
        case SCREEN: {
            m_screenNameWidget->show();
            m_areaSelectionWidget->show();
            disconnect(m_areaSelectionPushButton, &QPushButton::clicked, this, nullptr);
            connect(m_areaSelectionPushButton, &QPushButton::clicked, this, [this] {
                m_areaSelectionDialog->show();
                m_areaSelectionDialog->captureRequest("screen", m_screenNameCombobox->currentText());
            });
            m_portSettingSavePushButton->show();
            break;
        }
        case CAMERA: {
            m_cameraNameWidget->show();
            m_areaSelectionWidget->show();
            disconnect(m_areaSelectionPushButton, &QPushButton::clicked, this, nullptr);
            connect(m_areaSelectionPushButton, &QPushButton::clicked, this, [this] {
                m_areaSelectionDialog->show();
                m_areaSelectionDialog->captureRequest("camera", m_cameraNameCombobox->currentText());
            });
            m_portSettingSavePushButton->show();
            break;
        }
        default: {
            break;
        }
    }
    this->adjustSize();
}

void PortSetting::portSettingSave(const int portType) {
    m_portConfig = {};
    switch (portType) {
        case SERIALPORT: {
            if (m_portUsedName.contains(m_serialPortNameCombobox->currentData().toString())) {
                QMessageBox::critical(this, tr("Error"), tr("Port name already exists."));
                return;
            }
            m_portConfig["portType"] = portType;
            m_portConfig["portName"] = m_serialPortNameCombobox->currentData().toString();
            m_portConfig["baudRate"] = m_serialPortBaudRateSpinBox->value();
            m_portConfig["dataBits"] = m_serialPortDataBitsCombobox->currentData().toInt();
            m_portConfig["parity"] = m_serialPortParityCombobox->currentData().toInt();
            m_portConfig["stopBits"] = m_serialPortStopBitsCombobox->currentData().toInt();
            m_portConfig["txFormat"] = m_txFormatCombobox->currentText();
            m_portConfig["txSuffix"] = m_txSuffixCombobox->currentText();
            m_portConfig["rxFormat"] = m_rxFormatCombobox->currentText();
            break;
        }
        case VISA: {
            if (m_portUsedName.contains(m_serialPortNameCombobox->currentData().toString())) {
                QMessageBox::critical(this, tr("Error"), tr("Port name already exists."));
                return;
            }
            m_portConfig["portType"] = portType;
            m_portConfig["portName"] = m_visaNameCombobox->currentData().toString();
            m_portConfig["txFormat"] = m_txFormatCombobox->currentText();
            m_portConfig["txSuffix"] = m_txSuffixCombobox->currentText();
            m_portConfig["rxFormat"] = m_rxFormatCombobox->currentText();
        }
        case TCPCLIENT: {
            if (m_portUsedName.contains(m_tcpClientNameLineEdit->text())) {
                QMessageBox::critical(this, tr("Error"), tr("Port name already exists."));
                return;
            }
            m_portConfig["portType"] = portType;
            m_portConfig["portName"] = m_tcpClientNameLineEdit->text();
            m_portConfig["tcpClientRemoteAddress"] = m_tcpClientRemoteAddressLineEdit->text();
            m_portConfig["tcpClientRemotePort"] = m_tcpClientRemotePortSpinBox->value();
            m_portConfig["txFormat"] = m_txFormatCombobox->currentText();
            m_portConfig["txSuffix"] = m_txSuffixCombobox->currentText();
            m_portConfig["rxFormat"] = m_rxFormatCombobox->currentText();
            break;
        }
        case TCPSERVER: {
            if (m_portUsedName.contains(m_tcpServerNameLineEdit->text())) {
                QMessageBox::critical(this, tr("Error"), tr("Port name already exists."));
                return;
            }
            m_portConfig["portType"] = portType;
            m_portConfig["portName"] = m_tcpServerNameLineEdit->text();
            m_portConfig["tcpServerLocalAddress"] = m_tcpServerLocalAddressLineEdit->text();
            m_portConfig["tcpServerLocalPort"] = m_tcpServerLocalPortSpinBox->value();
            m_portConfig["txFormat"] = m_txFormatCombobox->currentText();
            m_portConfig["txSuffix"] = m_txSuffixCombobox->currentText();
            m_portConfig["rxFormat"] = m_rxFormatCombobox->currentText();
            break;
        }
        case UDPSOCKET: {
            if (m_portUsedName.contains(m_udpSocketNameLineEdit->text())) {
                QMessageBox::critical(this, tr("Error"), tr("Port name already exists."));
                return;
            }
            m_portConfig["portType"] = portType;
            m_portConfig["portName"] = m_udpSocketNameLineEdit->text();
            m_portConfig["udpSocketLocalAddress"] = m_udpSocketLocalAddressLineEdit->text();
            m_portConfig["udpSocketLocalPort"] = m_udpSocketLocalPortSpinBox->value();
            m_portConfig["udpSocketRemoteAddress"] = m_udpSocketRemoteAddressLineEdit->text();
            m_portConfig["udpSocketRemotePort"] = m_udpSocketRemotePortSpinBox->value();
            m_portConfig["txFormat"] = m_txFormatCombobox->currentText();
            m_portConfig["txSuffix"] = m_txSuffixCombobox->currentText();
            m_portConfig["rxFormat"] = m_rxFormatCombobox->currentText();
            break;
        }
        case SCREEN: {
            if (m_portUsedName.contains(m_screenNameCombobox->currentText())) {
                QMessageBox::critical(this, tr("Error"), tr("Port name already exists."));
                return;
            }
            m_portConfig["portType"] = portType;
            m_portConfig["portName"] = m_screenNameCombobox->currentText();
            m_portConfig["dpr"] = m_areaSelectionDialog->dprExport();
            m_portConfig["charset"] = m_areaSelectionDialog->charsetExport();
            m_portConfig["process"] = m_areaSelectionDialog->processExport();
            m_portConfig["areaList"] = m_areaSelectionDialog->areaExport();
            break;
        }
        case CAMERA: {
            if (m_portUsedName.contains(m_cameraNameCombobox->currentText())) {
                QMessageBox::critical(this, tr("Error"), tr("Port name already exists."));
                return;
            }
            m_portConfig["portType"] = "camera";
            m_portConfig["portName"] = m_cameraNameCombobox->currentText();
            m_portConfig["dpr"] = m_areaSelectionDialog->dprExport();
            m_portConfig["charset"] = m_areaSelectionDialog->charsetExport();
            m_portConfig["process"] = m_areaSelectionDialog->processExport();
            m_portConfig["areaList"] = m_areaSelectionDialog->areaExport();
            break;
        }
        default: {
            qDebug() << "unknown port type";
            break;
        }
    }
    this->accept();
}

QJsonObject PortSetting::portSettingExport() {
    return m_portConfig;
}

QStringList PortSetting::visaListGet() {
    QStringList deviceList;

    ViFindList findList;
    ViUInt32 numInst;
    ViChar portName[VI_FIND_BUFLEN];

    ViStatus status = viOpenDefaultRM(&g_rm);
    if (status != VI_SUCCESS) {
        qDebug() << "fails to start visa resource manager";
        return deviceList;
    }

    status = viFindRsrc(g_rm, "?*INSTR", &findList, &numInst, portName);
    if (status != VI_SUCCESS) {
        qDebug() << "fails to search visa instruments";
        viClose(g_rm);
        return deviceList;
    }

    deviceList.append(QString(portName));

    for (ViUInt32 i = 1; i < numInst; i++) {
        status = viFindNext(findList, portName);
        if (status == VI_SUCCESS) {
            deviceList.append(QString(portName));
        }
    }

    // viClose(findList);
    // viClose(rm);

    return deviceList;
}
