#include "portModule/portModule.h"

#include "globals.h"
#include "suffix.h"
#include "utils.h"
#include "portModule/basePort.h"
#include "portModule/serialPort.h"

// PortModule public
PortModule::PortModule(QWidget *parent)
    : QDockWidget("port", parent),
      m_portConfig(g_config["portConfig"].toArray()),
      m_tabWidget(new QTabWidget()),
      m_previewDialog(new QDialog(this)),
      m_previewLayout(new QVBoxLayout(m_previewDialog)) {
    // port widget gui init
    {
        setWidget(m_tabWidget);
        connect(m_tabWidget, &QTabWidget::currentChanged, this, &PortModule::portSelected);
        m_tabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_tabWidget->tabBar(), &QTabBar::customContextMenuRequested, this, [this](const QPoint &pos) {
            const int index = m_tabWidget->tabBar()->tabAt(pos);
            portMenu(index, pos);
        });
        m_tabWidget->setMovable(true);
        connect(m_tabWidget->tabBar(), &QTabBar::tabMoved, this, &PortModule::portSwap);
        auto *addButton = new QPushButton(m_tabWidget);
        addButton->setIcon(QIcon(":/icon/add.svg"));
        m_tabWidget->setCornerWidget(addButton, Qt::TopRightCorner);
        connect(addButton, &QPushButton::clicked, this, [this] { portSettingLoad(-1); });
        // init port tab
        if (const auto portCount = m_portConfig.size(); portCount == 0) {
            auto welcomePage = new QWidget(); // NOLINT
            auto welcomeLayout = new QVBoxLayout(welcomePage); // NOLINT
            auto welcomeLabel = new QLabel("welcome"); // NOLINT
            welcomeLayout->addWidget(welcomeLabel);
            m_tabWidget->addTab(welcomePage, "welcome");
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] %2").arg(timestamp, "no port config found, create a welcome page");
        } else {
            for (const QJsonValue &value: m_portConfig) {
                QJsonObject portConfig = value.toObject();
                auto *pageWidget = new PageWidget(portConfig, m_tabWidget); // NOLINT
                QString portName = portConfig["portName"].toString();
                m_tabWidget->addTab(pageWidget, portName);
                connect(pageWidget, &PageWidget::appendLog, this, &PortModule::appendLog);
                connect(pageWidget->m_port, &BasePort::showPreview, this, &PortModule::previewShow);
                // logging
                QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
                qDebug() << QString("[%1] %2 %3").arg(timestamp, QString::number(portCount), "port config found");
            }
        }
    }
    // port setting gui init
    {
        // init setting dialog & port type combobox
        {
            m_portSettingDialog = new QDialog(m_tabWidget);
            m_portSettingDialog->setFixedWidth(600);
            m_portSettingLayout = new QVBoxLayout(m_portSettingDialog);

            m_portTypeWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_portTypeWidget);
            const auto portTypeLayout = new QHBoxLayout(m_portTypeWidget); // NOLINT
            portTypeLayout->setContentsMargins(0, 0, 0, 0);
            const auto portTypeLabel = new QLabel("port type"); // NOLINT
            portTypeLayout->addWidget(portTypeLabel);
            m_portTypeCombobox = new QComboBox();
            portTypeLayout->addWidget(m_portTypeCombobox);
            m_portTypeCombobox->addItems(QStringList{"choose port type", "serial port", "tcp client", "tcp server", "udp socket", "screen", "camera"});
            connect(m_portTypeCombobox, &QComboBox::currentIndexChanged, this, &PortModule::portSettingTypeSwitch);
        }
        // init serial port settings
        {
            m_serialPortNameWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_serialPortNameWidget);
            const auto serialPortNameLayout = new QHBoxLayout(m_serialPortNameWidget); // NOLINT
            serialPortNameLayout->setContentsMargins(0, 0, 0, 0);
            const auto serialPortNameLabel = new QLabel("port name"); // NOLINT
            serialPortNameLayout->addWidget(serialPortNameLabel);
            m_serialPortNameCombobox = new QComboBox();
            serialPortNameLayout->addWidget(m_serialPortNameCombobox);

            m_serialPortBaudRateWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_serialPortBaudRateWidget);
            const auto serialPortBaudRateLayout = new QHBoxLayout(m_serialPortBaudRateWidget); // NOLINT
            serialPortBaudRateLayout->setContentsMargins(0, 0, 0, 0);
            const auto serialPortBaudRateLabel = new QLabel("baud rate"); // NOLINT
            serialPortBaudRateLayout->addWidget(serialPortBaudRateLabel);
            m_serialPortBaudRateSpinBox = new QSpinBox();
            serialPortBaudRateLayout->addWidget(m_serialPortBaudRateSpinBox);
            m_serialPortBaudRateSpinBox->setRange(1, 5000000);

            m_serialPortDataBitsWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_serialPortDataBitsWidget);
            const auto serialPortDataBitsLayout = new QHBoxLayout(m_serialPortDataBitsWidget); // NOLINT
            serialPortDataBitsLayout->setContentsMargins(0, 0, 0, 0);
            const auto serialPortDataBitsLabel = new QLabel("databits"); // NOLINT
            serialPortDataBitsLayout->addWidget(serialPortDataBitsLabel);
            m_serialPortDataBitsCombobox = new QComboBox();
            serialPortDataBitsLayout->addWidget(m_serialPortDataBitsCombobox);
            m_serialPortDataBitsCombobox->addItem("5", 5);
            m_serialPortDataBitsCombobox->addItem("6", 6);
            m_serialPortDataBitsCombobox->addItem("7", 7);
            m_serialPortDataBitsCombobox->addItem("8", 8);

            m_serialPortParityWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_serialPortParityWidget);
            const auto serialPortParityLayout = new QHBoxLayout(m_serialPortParityWidget); // NOLINT
            serialPortParityLayout->setContentsMargins(0, 0, 0, 0);
            const auto serialPortParityLabel = new QLabel("parity"); // NOLINT
            serialPortParityLayout->addWidget(serialPortParityLabel);
            m_serialPortParityCombobox = new QComboBox();
            serialPortParityLayout->addWidget(m_serialPortParityCombobox);
            m_serialPortParityCombobox->addItem("no", 0);
            m_serialPortParityCombobox->addItem("even", 2);
            m_serialPortParityCombobox->addItem("odd", 3);
            m_serialPortParityCombobox->addItem("space", 4);
            m_serialPortParityCombobox->addItem("mark", 5);

            m_serialPortStopBitsWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_serialPortStopBitsWidget);
            const auto serialPortStopBitsLayout = new QHBoxLayout(m_serialPortStopBitsWidget); // NOLINT
            serialPortStopBitsLayout->setContentsMargins(0, 0, 0, 0);
            const auto serialPortStopBitsLabel = new QLabel("stop bits"); // NOLINT
            serialPortStopBitsLayout->addWidget(serialPortStopBitsLabel);
            m_serialPortStopBitsCombobox = new QComboBox();
            serialPortStopBitsLayout->addWidget(m_serialPortStopBitsCombobox);
            m_serialPortStopBitsCombobox->addItem("1", 1);
            m_serialPortStopBitsCombobox->addItem("1.5", 3);
            m_serialPortStopBitsCombobox->addItem("2", 2);
        }
        // init tcp client settings
        {
            m_tcpClientRemoteAddressWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_tcpClientRemoteAddressWidget);
            const auto tcpClientRemoteAddressLayout = new QHBoxLayout(m_tcpClientRemoteAddressWidget); // NOLINT
            tcpClientRemoteAddressLayout->setContentsMargins(0, 0, 0, 0);
            const auto tcpClientRemoteAddressLabel = new QLabel("remote adress"); // NOLINT
            tcpClientRemoteAddressLayout->addWidget(tcpClientRemoteAddressLabel);
            m_tcpClientRemoteAddressLineEdit = new QLineEdit();
            tcpClientRemoteAddressLayout->addWidget(m_tcpClientRemoteAddressLineEdit);

            m_tcpClientRemotePortWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_tcpClientRemotePortWidget);
            const auto tcpClientRemotePortLayout = new QHBoxLayout(m_tcpClientRemotePortWidget); // NOLINT
            tcpClientRemotePortLayout->setContentsMargins(0, 0, 0, 0);
            const auto tcpClientRemotePortLabel = new QLabel("remote port"); // NOLINT
            tcpClientRemotePortLayout->addWidget(tcpClientRemotePortLabel);
            m_tcpClientRemotePortSpinBox = new QSpinBox();
            tcpClientRemotePortLayout->addWidget(m_tcpClientRemotePortSpinBox);
            m_tcpClientRemotePortSpinBox->setRange(0, 65536);
        }
        // init tcp server settings
        {
            m_tcpServerLocalAddressWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_tcpServerLocalAddressWidget);
            const auto tcpServerLocalAddressLayout = new QHBoxLayout(m_tcpServerLocalAddressWidget); // NOLINT
            tcpServerLocalAddressLayout->setContentsMargins(0, 0, 0, 0);
            const auto tcpServerLocalAddressLabel = new QLabel("local adress"); // NOLINT
            tcpServerLocalAddressLayout->addWidget(tcpServerLocalAddressLabel);
            m_tcpServerLocalAddressLineEdit = new QLineEdit();
            tcpServerLocalAddressLayout->addWidget(m_tcpServerLocalAddressLineEdit);

            m_tcpServerLocalPortWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_tcpServerLocalPortWidget);
            const auto tcpServerLocalPortLayout = new QHBoxLayout(m_tcpServerLocalPortWidget); // NOLINT
            tcpServerLocalPortLayout->setContentsMargins(0, 0, 0, 0);
            const auto tcpServerLocalPortLabel = new QLabel("local port"); // NOLINT
            tcpServerLocalPortLayout->addWidget(tcpServerLocalPortLabel);
            m_tcpServerLocalPortSpinBox = new QSpinBox();
            tcpServerLocalPortLayout->addWidget(m_tcpServerLocalPortSpinBox);
            m_tcpServerLocalPortSpinBox->setRange(0, 65536);
        }
        // init udp socket settings
        {
            m_udpSocketLocalAddressWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_udpSocketLocalAddressWidget);
            const auto udpSocketLocalAddressLayout = new QHBoxLayout(m_udpSocketLocalAddressWidget); // NOLINT
            udpSocketLocalAddressLayout->setContentsMargins(0, 0, 0, 0);
            const auto udpSocketLocalAddressLabel = new QLabel("local adress"); // NOLINT
            udpSocketLocalAddressLayout->addWidget(udpSocketLocalAddressLabel);
            m_udpSocketLocalAddressLineEdit = new QLineEdit();
            udpSocketLocalAddressLayout->addWidget(m_udpSocketLocalAddressLineEdit);

            m_udpSocketLocalPortWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_udpSocketLocalPortWidget);
            const auto udpSocketLocalPortLayout = new QHBoxLayout(m_udpSocketLocalPortWidget); // NOLINT
            udpSocketLocalPortLayout->setContentsMargins(0, 0, 0, 0);
            const auto udpSocketLocalPortLabel = new QLabel("local port"); // NOLINT
            udpSocketLocalPortLayout->addWidget(udpSocketLocalPortLabel);
            m_udpSocketLocalPortSpinBox = new QSpinBox();
            udpSocketLocalPortLayout->addWidget(m_udpSocketLocalPortSpinBox);
            m_udpSocketLocalPortSpinBox->setRange(0, 65536);

            m_udpSocketRemoteAddressWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_udpSocketRemoteAddressWidget);
            const auto udpSocketRemoteAddressLayout = new QHBoxLayout(m_udpSocketRemoteAddressWidget); // NOLINT
            udpSocketRemoteAddressLayout->setContentsMargins(0, 0, 0, 0);
            const auto udpSocketRemoteAddressLabel = new QLabel("remote adress"); // NOLINT
            udpSocketRemoteAddressLayout->addWidget(udpSocketRemoteAddressLabel);
            m_udpSocketRemoteAddressLineEdit = new QLineEdit();
            udpSocketRemoteAddressLayout->addWidget(m_udpSocketRemoteAddressLineEdit);

            m_udpSocketRemotePortWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_udpSocketRemotePortWidget);
            const auto udpSocketRemotePortLayout = new QHBoxLayout(m_udpSocketRemotePortWidget); // NOLINT
            udpSocketRemotePortLayout->setContentsMargins(0, 0, 0, 0);
            const auto udpSocketRemotePortLabel = new QLabel("remote port"); // NOLINT
            udpSocketRemotePortLayout->addWidget(udpSocketRemotePortLabel);
            m_udpSocketRemotePortSpinBox = new QSpinBox();
            udpSocketRemotePortLayout->addWidget(m_udpSocketRemotePortSpinBox);
            m_udpSocketRemotePortSpinBox->setRange(0, 65536);
        }
        // init screen/camera settings
        {
            m_screenNameWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_screenNameWidget);
            const auto screenLayout = new QHBoxLayout(m_screenNameWidget); // NOLINT
            screenLayout->setContentsMargins(0, 0, 0, 0);
            const auto screenNameLabel = new QLabel("screen name"); // NOLINT
            screenLayout->addWidget(screenNameLabel);
            m_screenNameCombobox = new QComboBox();
            screenLayout->addWidget(m_screenNameCombobox);

            m_cameraNameWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_cameraNameWidget);
            const auto cameraLayout = new QHBoxLayout(m_cameraNameWidget); // NOLINT
            cameraLayout->setContentsMargins(0, 0, 0, 0);
            const auto cameraNameLabel = new QLabel("camera name"); // NOLINT
            cameraLayout->addWidget(cameraNameLabel);
            m_cameraNameCombobox = new QComboBox();
            cameraLayout->addWidget(m_cameraNameCombobox);

            m_areaChooseDialog = new AreaSelectDialog(m_portSettingDialog);

            m_areaSelectWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_areaSelectWidget);
            const auto screenAreaLayout = new QHBoxLayout(m_areaSelectWidget); // NOLINT
            screenAreaLayout->setContentsMargins(0, 0, 0, 0);
            const auto screenAreaLabel = new QLabel("capture area"); // NOLINT
            screenAreaLayout->addWidget(screenAreaLabel);
            m_areaSelectPushButton = new QPushButton("choose capture area");
            screenAreaLayout->addWidget(m_areaSelectPushButton);
        }
        // init tx/rx settings
        {
            m_txFormatWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_txFormatWidget);
            const auto txFormatLayout = new QHBoxLayout(m_txFormatWidget); // NOLINT
            txFormatLayout->setContentsMargins(0, 0, 0, 0);
            const auto txFormatLabel = new QLabel("tx format"); // NOLINT
            txFormatLayout->addWidget(txFormatLabel);
            m_txFormatCombobox = new QComboBox();
            txFormatLayout->addWidget(m_txFormatCombobox);
            m_txFormatCombobox->addItems(QStringList{"hex", "ascii", "utf-8"});

            m_txSuffixWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_txSuffixWidget);
            const auto txSuffixLayout = new QHBoxLayout(m_txSuffixWidget); // NOLINT
            txSuffixLayout->setContentsMargins(0, 0, 0, 0);
            const auto txSuffixLabel = new QLabel("tx suffix"); // NOLINT
            txSuffixLayout->addWidget(txSuffixLabel);
            m_txSuffixCombobox = new QComboBox();
            txSuffixLayout->addWidget(m_txSuffixCombobox);
            m_txSuffixCombobox->addItems(QStringList{"null", "crlf", "crc8 maxim", "crc16 modbus"});

            m_rxFormatWidget = new QWidget(m_portSettingDialog);
            m_portSettingLayout->addWidget(m_rxFormatWidget);
            const auto rxFormatLayout = new QHBoxLayout(m_rxFormatWidget); // NOLINT
            rxFormatLayout->setContentsMargins(0, 0, 0, 0);
            const auto rxFormatLabel = new QLabel("rx format"); // NOLINT
            rxFormatLayout->addWidget(rxFormatLabel);
            m_rxFormatCombobox = new QComboBox();
            rxFormatLayout->addWidget(m_rxFormatCombobox);
            m_rxFormatCombobox->addItems(QStringList{"hex", "ascii", "utf-8"});
        }
        // init setting save button
        m_portSettingSavePushButton = new QPushButton("save setting");
        m_portSettingLayout->addWidget(m_portSettingSavePushButton);
        connect(m_portSettingSavePushButton, &QPushButton::clicked, this, [this]() {
            portSettingSave(m_portTypeCombobox->currentIndex());
        });
    }
    // preview dialog
}

void PortModule::portConfigSave() const {
    g_config["portConfig"] = m_portConfig;
}

BasePort *PortModule::portObject(const int index) const {
    BasePort *portObject = nullptr;
    if (index == -1) portObject = qobject_cast<PageWidget *>(m_tabWidget->currentWidget())->m_port;
    else portObject = qobject_cast<PageWidget *>(m_tabWidget->widget(index))->m_port;
    return portObject;
}

// PortModule private
void PortModule::portMenu(const int index, const QPoint &pos) {
    if (m_portConfig.empty()) return;
    m_tabWidget->setCurrentIndex(index);
    QMenu menu;
    menu.addAction("edit", [this, index] { portSettingLoad(index); });
    menu.addAction("duplicate", [this, index] { portDuplicate(index); });
    menu.addAction("remove", [this, index] { portRemove(index); });
    menu.exec(m_tabWidget->tabBar()->mapToGlobal(pos));
}

void PortModule::portSelected(const int index) {
    if (m_portConfig.empty())
        return;
    m_currentIndex = index;
    QJsonObject portInfo = m_portConfig[index].toObject();
    QString portType = portInfo["portType"].toString();
    QString portName = portInfo["portName"].toString();
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, portType, portName, "selected");
}

void PortModule::portDuplicate(const int index) {
    QJsonObject portConfig = m_portConfig[index].toObject();
    m_portConfig.insert(index + 1, portConfig);
    auto *pageWidget = new PageWidget(portConfig, m_tabWidget); // NOLINT
    const QString portName = portConfig["portName"].toString();
    m_tabWidget->insertTab(index + 1, pageWidget, portName);
    connect(pageWidget, &PageWidget::appendLog, this, &PortModule::appendLog);
}

void PortModule::portRemove(const int index) {
    QJsonObject portInfo = m_portConfig[index].toObject();
    QString portType = portInfo["portType"].toString();
    QString portName = portInfo["portName"].toString();
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, portType, portName, "re");
    m_portConfig.removeAt(index);
    QWidget *w = m_tabWidget->widget(index);
    m_tabWidget->removeTab(index);
    if (w) w->deleteLater();
}

void PortModule::portSwap(const int srcIndex, const int dstIndex) {
    // config
    const QJsonValue tmp = m_portConfig.takeAt(srcIndex);
    m_portConfig.insert(dstIndex, tmp);
    // qDebug() << m_portConfig;
}

void PortModule::previewShow(const QList<QPixmap> &pixmapList) const {
    // clear previous preview
    QLayoutItem *item;
    while ((item = m_previewLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    foreach(QPixmap pixmap, pixmapList) {
        auto *label = new QLabel();
        m_previewLayout->addWidget(label);
        label->setPixmap(pixmap);
    }
    m_previewDialog->setVisible(true);
}

void PortModule::portSettingLoad(const int index) {
    m_portSettingDialog->show();
    if (index == -1) {
        m_currentIndex = -1;
        m_portTypeCombobox->setCurrentIndex(0);
        portSettingWidgetReset();
        // serial port
        m_serialPortBaudRateSpinBox->setValue(115200);
        m_serialPortDataBitsCombobox->setCurrentText("8");
        m_serialPortParityCombobox->setCurrentText("no");
        m_serialPortStopBitsCombobox->setCurrentText("1");
        // tx/rx
        m_txFormatCombobox->setCurrentText("hex");
        m_txSuffixCombobox->setCurrentText("null");
        m_rxFormatCombobox->setCurrentText("hex");
    } else {
        m_currentIndex = m_tabWidget->currentIndex();
        QJsonObject portInfo = m_portConfig[index].toObject();
        const QString portType = portInfo["portType"].toString();
        m_portTypeCombobox->setCurrentText(portType);
        if (portType == "serial port") {
            int i = m_serialPortNameCombobox->findData(portInfo["portName"].toString());
            m_serialPortNameCombobox->setCurrentIndex(i);
            m_serialPortBaudRateSpinBox->setValue(portInfo["baudRate"].toInt());
            i = m_serialPortDataBitsCombobox->findData(portInfo["dataBits"].toInt());
            m_serialPortDataBitsCombobox->setCurrentIndex(i);
            i = m_serialPortParityCombobox->findData(portInfo["parity"].toInt());
            m_serialPortParityCombobox->setCurrentIndex(i);
            i = m_serialPortStopBitsCombobox->findData(portInfo["stopBits"].toInt());
            m_serialPortStopBitsCombobox->setCurrentIndex(i);
            m_txFormatCombobox->setCurrentText(portInfo["txFormat"].toString());
            m_txSuffixCombobox->setCurrentText(portInfo["txSuffix"].toString());
            m_rxFormatCombobox->setCurrentText(portInfo["rxFormat"].toString());
        } else if (portType == "tcp client") {
            m_tcpClientRemoteAddressLineEdit->setText(portInfo["tcpClientRemoteAddress"].toString());
            m_tcpClientRemotePortSpinBox->setValue(portInfo["tcpClientRemotePort"].toInt());
            m_txFormatCombobox->setCurrentText(portInfo["txFormat"].toString());
            m_txSuffixCombobox->setCurrentText(portInfo["txSuffix"].toString());
            m_rxFormatCombobox->setCurrentText(portInfo["rxFormat"].toString());
        } else if (portType == "tcp server") {
            m_tcpServerLocalAddressLineEdit->setText(portInfo["tcpServerLocalAddress"].toString());
            m_tcpServerLocalPortSpinBox->setValue(portInfo["tcpServerLocalPort"].toInt());
            m_txFormatCombobox->setCurrentText(portInfo["txFormat"].toString());
            m_txSuffixCombobox->setCurrentText(portInfo["txSuffix"].toString());
            m_rxFormatCombobox->setCurrentText(portInfo["rxFormat"].toString());
        } else if (portType == "udp socket") {
            m_udpSocketLocalAddressLineEdit->setText(portInfo["udpSocketLocalAddress"].toString());
            m_udpSocketLocalPortSpinBox->setValue(portInfo["udpSocketLocalPort"].toInt());
            m_udpSocketRemoteAddressLineEdit->setText(portInfo["udpSocketRemoteAddress"].toString());
            m_udpSocketRemotePortSpinBox->setValue(portInfo["udpSocketRemotePort"].toInt());
            m_txFormatCombobox->setCurrentText(portInfo["txFormat"].toString());
            m_txSuffixCombobox->setCurrentText(portInfo["txSuffix"].toString());
            m_rxFormatCombobox->setCurrentText(portInfo["rxFormat"].toString());
        } else if (portType == "screen") {
            m_screenNameCombobox->setCurrentText(portInfo["portName"].toString());
            m_areaChooseDialog->show();
            m_areaChooseDialog->reload(portInfo);
            m_areaChooseDialog->captureRequest("screen", m_screenNameCombobox->currentText());
        } else /* portType == "camera" */ {
            m_cameraNameCombobox->setCurrentText(portInfo["portName"].toString());
            m_areaChooseDialog->show();
            m_areaChooseDialog->reload(portInfo);
            m_areaChooseDialog->captureRequest("camera", m_cameraNameCombobox->currentText());
        }
    }
}

void PortModule::portSettingWidgetReset() const {
    // enable port type combobox
    m_portTypeCombobox->setEnabled(true);
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
    // tcp client setting widget
    m_tcpClientRemoteAddressWidget->hide();
    m_tcpClientRemotePortWidget->hide();
    // tcp server setting widget
    m_tcpServerLocalAddressWidget->hide();
    m_tcpServerLocalPortWidget->hide();
    // udp socket setting widget
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
    m_areaSelectWidget->hide();
    // rx/tx setting widget
    m_txFormatWidget->hide();
    m_txSuffixWidget->hide();
    m_rxFormatWidget->hide();
    // save button
    m_portSettingSavePushButton->hide();
};

void PortModule::portSettingTypeSwitch(const int type) {
    if (type == 0) {
        portSettingWidgetReset();
    } else if (type == 1) {
        portSettingWidgetReset();
        m_portTypeCombobox->setEnabled(false);
        m_serialPortNameWidget->show();
        m_serialPortBaudRateWidget->show();
        m_serialPortDataBitsWidget->show();
        m_serialPortParityWidget->show();
        m_serialPortStopBitsWidget->show();
        m_txFormatWidget->show();
        m_txSuffixWidget->show();
        m_rxFormatWidget->show();
        m_portSettingSavePushButton->show();
    } else if (type == 2) {
        portSettingWidgetReset();
        m_portTypeCombobox->setEnabled(false);
        m_tcpClientRemoteAddressWidget->show();
        m_tcpClientRemotePortWidget->show();
        m_txFormatWidget->show();
        m_txSuffixWidget->show();
        m_rxFormatWidget->show();
        m_portSettingSavePushButton->show();
    } else if (type == 3) {
        portSettingWidgetReset();
        m_portTypeCombobox->setEnabled(false);
        m_tcpServerLocalAddressWidget->show();
        m_tcpServerLocalPortWidget->show();
        m_txFormatWidget->show();
        m_txSuffixWidget->show();
        m_rxFormatWidget->show();
        m_portSettingSavePushButton->show();
    } else if (type == 4) {
        portSettingWidgetReset();
        m_portTypeCombobox->setEnabled(false);
        m_udpSocketLocalAddressWidget->show();
        m_udpSocketLocalPortWidget->show();
        m_udpSocketRemoteAddressWidget->show();
        m_udpSocketRemotePortWidget->show();
        m_txFormatWidget->show();
        m_txSuffixWidget->show();
        m_rxFormatWidget->show();
        m_portSettingSavePushButton->show();
    } else if (type == 5) {
        portSettingWidgetReset();
        m_portTypeCombobox->setEnabled(false);
        m_screenNameWidget->show();
        m_areaSelectWidget->show();
        disconnect(m_areaSelectPushButton, &QPushButton::clicked, this, nullptr);
        connect(m_areaSelectPushButton, &QPushButton::clicked, this, [this] {
            m_areaChooseDialog->show();
            m_areaChooseDialog->captureRequest("screen", m_screenNameCombobox->currentText());
        });
        m_portSettingSavePushButton->show();
    } else {
        portSettingWidgetReset();
        m_portTypeCombobox->setEnabled(false);
        m_cameraNameWidget->show();
        m_areaSelectWidget->show();
        disconnect(m_areaSelectPushButton, &QPushButton::clicked, this, nullptr);
        connect(m_areaSelectPushButton, &QPushButton::clicked, this, [this] {
            m_areaChooseDialog->show();
            m_areaChooseDialog->captureRequest("camera", m_cameraNameCombobox->currentText());
        });
        m_portSettingSavePushButton->show();
    }
    m_portSettingDialog->adjustSize();
}

void PortModule::portSettingSave(const int type) {
    if (type == 1) {
        QJsonObject portConfig;
        portConfig["portType"] = "serial port";
        portConfig["portName"] = m_serialPortNameCombobox->currentData().toString();
        portConfig["baudRate"] = m_serialPortBaudRateSpinBox->value();
        portConfig["dataBits"] = m_serialPortDataBitsCombobox->currentData().toInt();
        portConfig["parity"] = m_serialPortParityCombobox->currentData().toInt();
        portConfig["stopBits"] = m_serialPortStopBitsCombobox->currentData().toInt();
        portConfig["txFormat"] = m_txFormatCombobox->currentText();
        portConfig["txSuffix"] = m_txSuffixCombobox->currentText();
        portConfig["rxFormat"] = m_rxFormatCombobox->currentText();
        if (m_currentIndex == -1) {
            if (m_portConfig.empty()) {
                m_tabWidget->removeTab(0);
            }
            m_portConfig.append(portConfig);
            auto *pageWidget = new PageWidget(portConfig, m_tabWidget); // NOLINT
            const QString portName = portConfig["portName"].toString();
            m_tabWidget->addTab(pageWidget, portName);
            connect(pageWidget, &PageWidget::appendLog, this, &PortModule::appendLog);
        } else {
            m_portConfig[m_currentIndex] = portConfig;
            const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(m_currentIndex));
            pageWidget->portReload(portConfig);
        }
        m_tabWidget->setTabText(m_currentIndex, m_serialPortNameCombobox->currentData().toString());
    } else if (type == 2) {
        QJsonObject portConfig;
        portConfig["portType"] = "tcp client";
        portConfig["portName"] = "tcp client";
        portConfig["tcpClientRemoteAddress"] = m_tcpClientRemoteAddressLineEdit->text();
        portConfig["tcpClientRemotePort"] = m_tcpClientRemotePortSpinBox->value();
        portConfig["txFormat"] = m_txFormatCombobox->currentText();
        portConfig["txSuffix"] = m_txSuffixCombobox->currentText();
        portConfig["rxFormat"] = m_rxFormatCombobox->currentText();
        if (m_currentIndex == -1) {
            if (m_portConfig.empty()) {
                m_tabWidget->removeTab(0);
            }
            m_portConfig.append(portConfig);
            auto *pageWidget = new PageWidget(portConfig, m_tabWidget); // NOLINT
            const QString portName = portConfig["portName"].toString();
            m_tabWidget->addTab(pageWidget, portName);
            connect(pageWidget, &PageWidget::appendLog, this, &PortModule::appendLog);
        } else {
            m_portConfig[m_currentIndex] = portConfig;
            const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(m_currentIndex));
            pageWidget->portReload(portConfig);
        }
    } else if (type == 3) {
        QJsonObject portConfig;
        portConfig["portType"] = "tcp server";
        portConfig["portName"] = "tcp server";
        portConfig["tcpServerLocalAddress"] = m_tcpServerLocalAddressLineEdit->text();
        portConfig["tcpServerLocalPort"] = m_tcpServerLocalPortSpinBox->value();
        portConfig["txFormat"] = m_txFormatCombobox->currentText();
        portConfig["txSuffix"] = m_txSuffixCombobox->currentText();
        portConfig["rxFormat"] = m_rxFormatCombobox->currentText();
        if (m_currentIndex == -1) {
            if (m_portConfig.empty()) {
                m_tabWidget->removeTab(0);
            }
            m_portConfig.append(portConfig);
            auto *pageWidget = new PageWidget(portConfig, m_tabWidget); // NOLINT
            const QString portName = portConfig["portName"].toString();
            m_tabWidget->addTab(pageWidget, portName);
            connect(pageWidget, &PageWidget::appendLog, this, &PortModule::appendLog);
        } else {
            m_portConfig[m_currentIndex] = portConfig;
            const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(m_currentIndex));
            pageWidget->portReload(portConfig);
        }
    } else if (type == 4) {
        QJsonObject portConfig;
        portConfig["portType"] = "udp socket";
        portConfig["portName"] = "udp socket";
        portConfig["udpSocketLocalAddress"] = m_udpSocketLocalAddressLineEdit->text();
        portConfig["udpSocketLocalPort"] = m_udpSocketLocalPortSpinBox->value();
        portConfig["udpSocketRemoteAddress"] = m_udpSocketRemoteAddressLineEdit->text();
        portConfig["udpSocketRemotePort"] = m_udpSocketRemotePortSpinBox->value();
        portConfig["txFormat"] = m_txFormatCombobox->currentText();
        portConfig["txSuffix"] = m_txSuffixCombobox->currentText();
        portConfig["rxFormat"] = m_rxFormatCombobox->currentText();
        if (m_currentIndex == -1) {
            if (m_portConfig.empty()) {
                m_tabWidget->removeTab(0);
            }
            m_portConfig.append(portConfig);
            auto *pageWidget = new PageWidget(portConfig, m_tabWidget); // NOLINT
            const QString portName = portConfig["portName"].toString();
            m_tabWidget->addTab(pageWidget, portName);
            connect(pageWidget, &PageWidget::appendLog, this, &PortModule::appendLog);
        } else {
            m_portConfig[m_currentIndex] = portConfig;
            const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(m_currentIndex));
            pageWidget->portReload(portConfig);
        }
    } else if (type == 5) {
        QJsonObject portConfig;
        portConfig["portType"] = "screen";
        portConfig["portName"] = m_screenNameCombobox->currentText();
        portConfig["dpr"] = m_areaChooseDialog->dprExport();
        portConfig["charset"] = m_areaChooseDialog->charsetExport();
        portConfig["process"] = m_areaChooseDialog->processExport();
        portConfig["areaList"] = m_areaChooseDialog->areaExport();
        if (m_currentIndex == -1) {
            if (m_portConfig.empty()) {
                m_tabWidget->removeTab(0);
            }
            m_portConfig.append(portConfig);
            auto *pageWidget = new PageWidget(portConfig, m_tabWidget); // NOLINT
            const QString portName = portConfig["portName"].toString();
            m_tabWidget->addTab(pageWidget, portName);
            connect(pageWidget, &PageWidget::appendLog, this, &PortModule::appendLog);
        } else {
            m_portConfig[m_currentIndex] = portConfig;
            const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(m_currentIndex));
            pageWidget->portReload(portConfig);
        }
    } else {
        QJsonObject portConfig;
        portConfig["portType"] = "camera";
        portConfig["portName"] = m_cameraNameCombobox->currentText();
        portConfig["dpr"] = m_areaChooseDialog->dprExport();
        portConfig["charset"] = m_areaChooseDialog->charsetExport();
        portConfig["process"] = m_areaChooseDialog->processExport();
        portConfig["areaList"] = m_areaChooseDialog->areaExport();
        if (m_currentIndex == -1) {
            if (m_portConfig.empty()) {
                m_tabWidget->removeTab(0);
            }
            m_portConfig.append(portConfig);
            auto *pageWidget = new PageWidget(portConfig, m_tabWidget); // NOLINT
            const QString portName = portConfig["portName"].toString();
            m_tabWidget->addTab(pageWidget, portName);
            connect(pageWidget, &PageWidget::appendLog, this, &PortModule::appendLog);
        } else {
            m_portConfig[m_currentIndex] = portConfig;
            const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(m_currentIndex));
            pageWidget->portReload(portConfig);
        }
    }
    m_portSettingDialog->hide();
}

// PageWidget public
PageWidget::PageWidget(const QJsonObject &portConfig, QObject *parent) {
    QString timestamp;
    auto *pageLayout = new QVBoxLayout(this); // NOLINT
    const QString portType = portConfig["portType"].toString();
    QString portName = portConfig["portName"].toString();
    if (portType == "serial port") {
        // ui init
        m_pushButton = new QPushButton("open"); // NOLINT
        m_pushButton->setCheckable(true);
        pageLayout->addWidget(m_pushButton);
        // port init
        m_thread = new QThread(this);
        m_port = new SerialPort(portConfig);
        m_port->moveToThread(m_thread);
        // start thread
        connect(m_pushButton, &QPushButton::clicked, this, &PageWidget::portToggle);
        connect(m_port, &BasePort::appendLog, this, &PageWidget::appendLog);
        connect(m_thread, &QThread::finished, m_port, &QObject::deleteLater);
        m_thread->start();
        // logging
        timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, "serial port", portName, "loaded");
    } else if (portType == "tcp client") {
        // ui init
        m_pushButton = new QPushButton("open"); // NOLINT
        m_pushButton->setCheckable(true);
        pageLayout->addWidget(m_pushButton);
        // port init
        m_thread = new QThread(this);
        m_port = new TcpClient(portConfig);
        m_port->moveToThread(m_thread);
        // start thread
        connect(m_pushButton, &QPushButton::clicked, this, &PageWidget::portToggle);
        connect(m_port, &BasePort::appendLog, this, &PageWidget::appendLog);
        connect(m_thread, &QThread::finished, m_port, &QObject::deleteLater);
        m_thread->start();
        // logging
        timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, "tcp client", portName, "loaded");
    } else if (portType == "tcp server") {
        // ui init
        m_pushButton = new QPushButton("open"); // NOLINT
        m_pushButton->setCheckable(true);
        pageLayout->addWidget(m_pushButton);
        // port init
        m_thread = new QThread(this);
        m_port = new TcpServer(portConfig);
        m_port->moveToThread(m_thread);
        // start thread
        connect(m_pushButton, &QPushButton::clicked, this, &PageWidget::portToggle);
        connect(m_port, &BasePort::appendLog, this, &PageWidget::appendLog);
        connect(m_thread, &QThread::finished, m_port, &QObject::deleteLater);
        m_thread->start();
        // logging
        timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, "tcp server", portName, "loaded");
    } else if (portType == "udp socket") {
        // ui init
        m_pushButton = new QPushButton("open"); // NOLINT
        m_pushButton->setCheckable(true);
        pageLayout->addWidget(m_pushButton);
        // port init
        m_thread = new QThread(this);
        m_port = new UdpSocket(portConfig);
        m_port->moveToThread(m_thread);
        // start thread
        connect(m_pushButton, &QPushButton::clicked, this, &PageWidget::portToggle);
        connect(m_port, &BasePort::appendLog, this, &PageWidget::appendLog);
        connect(m_thread, &QThread::finished, m_port, &QObject::deleteLater);
        m_thread->start();
        // logging
        timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, "udp socket", portName, "loaded");
    } else if (portType == "screen") {
        // ui init
        m_pushButton = new QPushButton("open"); // NOLINT
        m_pushButton->setCheckable(true);
        pageLayout->addWidget(m_pushButton);
        // port init
        m_thread = new QThread(this);
        m_port = new Screen(portConfig);
        m_port->moveToThread(m_thread);
        // start thread
        connect(m_pushButton, &QPushButton::clicked, this, &PageWidget::portToggle);
        // connect(m_port, &BasePort::appendLog, this, &PageWidget::appendLog);
        connect(m_thread, &QThread::finished, m_port, &QObject::deleteLater);
        m_thread->start();
        // logging
        timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, "screen", portName, "loaded");
    } else /* portType == "camera" */
    {
        // ui init
        m_pushButton = new QPushButton("open"); // NOLINT
        m_pushButton->setCheckable(true);
        pageLayout->addWidget(m_pushButton);
        // port init
        m_thread = new QThread(this);
        m_port = new Camera(portConfig);
        m_port->moveToThread(m_thread);
        // start thread
        connect(m_pushButton, &QPushButton::clicked, this, &PageWidget::portToggle);
        // connect(m_port, &BasePort::appendLog, this, &PageWidget::appendLog);
        connect(m_thread, &QThread::finished, m_port, &QObject::deleteLater);
        m_thread->start();
        // logging
        timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, "camera", portName, "loaded");
    }
}

PageWidget::~PageWidget() {
    QMetaObject::invokeMethod(m_port, [this] {
        m_port->close();
    }, Qt::BlockingQueuedConnection);
    m_thread->quit();
    m_thread->wait();
}

void PageWidget::portReload(const QJsonObject &portConfig) const {
    QMetaObject::invokeMethod(m_port, [this] {
        m_port->close();
    }, Qt::BlockingQueuedConnection);
    m_pushButton->setChecked(false);
    QMetaObject::invokeMethod(m_port, [this, portConfig] {
        m_port->reload(portConfig);
    }, Qt::BlockingQueuedConnection);
}

// PageWidget private
void PageWidget::portToggle(const bool status) const {
    if (status) {
        bool ok = false;
        QMetaObject::invokeMethod(m_port, [&ok, this] {
            ok = m_port->open();
        }, Qt::BlockingQueuedConnection);
        m_pushButton->setChecked(ok);
    } else {
        QMetaObject::invokeMethod(m_port, [this] {
            m_port->close();
        }, Qt::BlockingQueuedConnection);
        m_pushButton->setChecked(false);
    }
}






