#include "../include/port.h"

// Port public
Port::Port(QObject *parent)
    : QDockWidget("port", qobject_cast<QWidget *>(parent)) {
    // port widget gui init
    {
        m_tabWidget = new QTabWidget();
        setWidget(m_tabWidget);
        connect(m_tabWidget, &QTabWidget::currentChanged, this, &Port::portSelected);
        m_tabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_tabWidget->tabBar(), &QTabBar::customContextMenuRequested, this, [this](const QPoint &pos) {
            const int index = m_tabWidget->tabBar()->tabAt(pos);
            portMenu(index, pos);
        });
        m_tabWidget->setMovable(true);
        connect(m_tabWidget->tabBar(), &QTabBar::tabMoved, this, &Port::portSwap);
        m_addButton = new QPushButton(m_tabWidget);
        m_addButton->setIcon(QIcon(":/icon/add.svg"));
        m_tabWidget->setCornerWidget(m_addButton, Qt::TopRightCorner);
        connect(m_addButton, &QPushButton::clicked, this, [this]() {
            portSettingLoad(-1);
        });
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
                connect(pageWidget, &PageWidget::appendLog, this, &Port::appendLog);
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
            connect(m_portTypeCombobox, &QComboBox::currentIndexChanged, this, &Port::portSettingTypeSwitch);
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
}

void Port::portConfigSave() const {
    g_config["portConfig"] = m_portConfig;
}

BasePort *Port::portObject(const int index) const {
    BasePort *portObject = nullptr;
    if (index == -1) portObject = qobject_cast<PageWidget *>(m_tabWidget->currentWidget())->m_port;
    else portObject = qobject_cast<PageWidget *>(m_tabWidget->widget(index))->m_port;
    return portObject;
}

// Port private
void Port::portMenu(const int index, const QPoint &pos) {
    if (m_portConfig.empty())
        return;
    m_tabWidget->setCurrentIndex(index);
    QMenu menu;
    menu.addAction("edit", [this, index]() {
        portSettingLoad(index);
    });
    menu.addAction("duplicate", [this, index]() {
        portDuplicate(index);
    });
    menu.addAction("remove", [this, index]() {
        portRemove(index);
    });
    menu.exec(m_tabWidget->tabBar()->mapToGlobal(pos));
}

void Port::portSelected(const int index) {
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

void Port::portDuplicate(const int index) {
    QJsonObject portConfig = m_portConfig[index].toObject();
    m_portConfig.insert(index + 1, portConfig);
    auto *pageWidget = new PageWidget(portConfig, m_tabWidget); // NOLINT
    const QString portName = portConfig["portName"].toString();
    m_tabWidget->insertTab(index + 1, pageWidget, portName);
    connect(pageWidget, &PageWidget::appendLog, this, &Port::appendLog);
}

void Port::portRemove(const int index) {
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

void Port::portSwap(const int srcIndex, const int dstIndex) {
    // config
    const QJsonValue tmp = m_portConfig.takeAt(srcIndex);
    m_portConfig.insert(dstIndex, tmp);
    // qDebug() << m_portConfig;
}

void Port::portSettingLoad(const int index) {
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
            m_areaChooseDialog->capture("screen", m_screenNameCombobox->currentText());
            m_areaChooseDialog->reload(portInfo);
        } else /* portType == "camera" */ {
            m_cameraNameCombobox->setCurrentText(portInfo["portName"].toString());
            m_areaChooseDialog->show();
            m_areaChooseDialog->capture("camera", m_cameraNameCombobox->currentText());
            m_areaChooseDialog->reload(portInfo);
        }
    }
}

void Port::portSettingWidgetReset() const {
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

void Port::portSettingTypeSwitch(const int type) {
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
            m_areaChooseDialog->capture("screen", m_screenNameCombobox->currentText());
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
            m_areaChooseDialog->capture("camera", m_cameraNameCombobox->currentText());
        });
        m_portSettingSavePushButton->show();
    }
    m_portSettingDialog->adjustSize();
}

void Port::portSettingSave(const int type) {
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
            connect(pageWidget, &PageWidget::appendLog, this, &Port::appendLog);
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
            connect(pageWidget, &PageWidget::appendLog, this, &Port::appendLog);
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
            connect(pageWidget, &PageWidget::appendLog, this, &Port::appendLog);
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
            connect(pageWidget, &PageWidget::appendLog, this, &Port::appendLog);
        } else {
            m_portConfig[m_currentIndex] = portConfig;
            const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(m_currentIndex));
            pageWidget->portReload(portConfig);
        }
    } else if (type == 5) {
        QJsonObject portConfig;
        portConfig["portType"] = "screen";
        portConfig["portName"] = m_screenNameCombobox->currentText();
        portConfig["areaList"] = m_areaChooseDialog->areaExport();
        if (m_currentIndex == -1) {
            if (m_portConfig.empty()) {
                m_tabWidget->removeTab(0);
            }
            m_portConfig.append(portConfig);
            auto *pageWidget = new PageWidget(portConfig, m_tabWidget); // NOLINT
            const QString portName = portConfig["portName"].toString();
            m_tabWidget->addTab(pageWidget, portName);
            connect(pageWidget, &PageWidget::appendLog, this, &Port::appendLog);
        } else {
            m_portConfig[m_currentIndex] = portConfig;
            const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(m_currentIndex));
            pageWidget->portReload(portConfig);
        }
    } else {
        QJsonObject portConfig;
        portConfig["portType"] = "camera";
        portConfig["portName"] = m_cameraNameCombobox->currentText();
        portConfig["areaList"] = m_areaChooseDialog->areaExport();
        if (m_currentIndex == -1) {
            if (m_portConfig.empty()) {
                m_tabWidget->removeTab(0);
            }
            m_portConfig.append(portConfig);
            auto *pageWidget = new PageWidget(portConfig, m_tabWidget); // NOLINT
            const QString portName = portConfig["portName"].toString();
            m_tabWidget->addTab(pageWidget, portName);
            connect(pageWidget, &PageWidget::appendLog, this, &Port::appendLog);
        } else {
            m_portConfig[m_currentIndex] = portConfig;
            const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(m_currentIndex));
            pageWidget->portReload(portConfig);
        }
    }
    m_portSettingDialog->hide();
}

// AreaSelectDialog public
AreaSelectDialog::AreaSelectDialog(QWidget *parent)
    : QDialog(parent) {
    this->setFixedSize(1280, 720);
    auto *layout = new QHBoxLayout(this); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    auto *splitter = new QSplitter(Qt::Horizontal); // NOLINT
    layout->addWidget(splitter);

    m_graphicsView = new QGraphicsView();
    splitter->addWidget(m_graphicsView);
    connect(m_graphicsView, &QGraphicsView::rubberBandChanged, this, &AreaSelectDialog::selectionHandle);

    auto *ctrlWidget = new QWidget(); // NOLINT
    splitter->addWidget(ctrlWidget);
    auto *ctrlLayout = new QVBoxLayout(ctrlWidget);
    ctrlLayout->setContentsMargins(0, 0, 0, 0);
    ctrlLayout->setAlignment(Qt::AlignLeft);
    auto *refreshButton = new QPushButton("refresh");
    ctrlLayout->addWidget(refreshButton);
    refreshButton->setFixedSize(80, 48);
    refreshButton->setIcon(QIcon(":/icon/arrowClockwise.svg"));
    connect(refreshButton, &QPushButton::clicked, this, [this] { capture(m_type, m_target); });

    auto *cropButton = new QPushButton("crop");
    ctrlLayout->addWidget(cropButton);
    cropButton->setCheckable(true);
    cropButton->setFixedSize(80, 48);
    cropButton->setIcon(QIcon(":/icon/crop.svg"));
    connect(cropButton, &QPushButton::clicked, this, &AreaSelectDialog::select);

    // charset widget
    {
        auto *charsetWidget = new QWidget();
        ctrlLayout->addWidget(charsetWidget);
        auto *charsetLayout = new QVBoxLayout(charsetWidget);
        charsetLayout->setContentsMargins(0, 0, 0, 0);

        auto *charsetLabel = new QLabel(tr("Charset"));
        charsetLayout->addWidget(charsetLabel);
        charsetLabel->setFont(QFont("consolas", 16));

        auto *seperator = new QFrame();
        seperator->setFrameShape(QFrame::HLine);
        seperator->setLineWidth(1);
        charsetLayout->addWidget(seperator);

        m_charsetListView = new QListView();
        charsetLayout->addWidget(m_charsetListView);
        m_charsetListView->setStyleSheet("QListView::item { min-height: 30px; }");
        m_charsetListView->setDragDropMode(QAbstractItemView::InternalMove);
        m_charsetListView->setDefaultDropAction(Qt::MoveAction);
        m_charsetListView->setDragEnabled(true);
        m_charsetListView->setAcceptDrops(true);
        m_charsetListView->setDropIndicatorShown(true);
        m_charsetModel = new QStandardItemModel();
        m_charsetListView->setModel(m_charsetModel);
        auto *engItem = new QStandardItem();
        m_charsetModel->appendRow(engItem);
        engItem->setText(tr("English"));
        engItem->setData("eng", Qt::UserRole + 1);
        engItem->setCheckable(true);
        engItem->setCheckState(Qt::Checked);
        engItem->setFlags(engItem->flags() &= ~Qt::ItemIsDropEnabled);
        auto *ssegItem = new QStandardItem();
        m_charsetModel->appendRow(ssegItem);
        ssegItem->setText(tr("Seven Segment"));
        ssegItem->setData("7seg", Qt::UserRole + 1);
        ssegItem->setCheckable(true);
        ssegItem->setFlags(ssegItem->flags() &= ~Qt::ItemIsDropEnabled);
        connect(m_charsetModel, &QStandardItemModel::rowsRemoved, this, [this] { charsetRefresh(); });
        connect(m_charsetModel, &QStandardItemModel::itemChanged, this, [this] { charsetRefresh(); });
    }

    // selection widget
    {
        auto *selectionWidget = new QWidget();
        ctrlLayout->addWidget(selectionWidget);
        auto *selectionLayout = new QVBoxLayout(selectionWidget);
        selectionLayout->setContentsMargins(0, 0, 0, 0);

        auto *selectionLabel = new QLabel(tr("Selection"));
        selectionLayout->addWidget(selectionLabel);
        selectionLabel->setFont(QFont("consolas", 16));

        auto *seperator = new QFrame();
        seperator->setFrameShape(QFrame::HLine);
        seperator->setLineWidth(1);
        selectionLayout->addWidget(seperator);

        m_selectionListView = new QListView();
        selectionLayout->addWidget(m_selectionListView);
        m_selectionListView->setStyleSheet("QListView::item { min-height: 30px; }");
        m_selectionListView->setDragDropMode(QAbstractItemView::InternalMove);
        m_selectionListView->setDefaultDropAction(Qt::MoveAction);
        m_selectionListView->setDragEnabled(true);
        m_selectionListView->setAcceptDrops(true);
        m_selectionListView->setDropIndicatorShown(true);
        m_selectionListView->installEventFilter(this);
        m_selectionModel = new QStandardItemModel();
        m_selectionListView->setModel(m_selectionModel);
        connect(m_selectionModel, &QStandardItemModel::rowsMoved, this, [this] { selectionRefresh(); });
        connect(m_selectionModel, &QStandardItemModel::rowsRemoved, this, [this] { selectionRefresh(); });
    }

    // auto *processCombobox = new QComboBox();
    // ctrlLayout->addWidget(processCombobox);
    // processCombobox->addItem(tr("raw"));
    // processCombobox->addItem(tr("threshold"));
    // connect(processCombobox, &QComboBox::currentIndexChanged, this, [this, processCombobox] {
    //     m_process = processCombobox->currentIndex();
    //     capture(m_type, m_target);
    // });

    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
}

void AreaSelectDialog::capture(const QString &type, const QString &target) {
    m_type = type;
    m_target = target;
    QPixmap shot;
    if (m_type == "screen") {
        // find screen
        QScreen *screen = nullptr;
        for (QScreen *s: QGuiApplication::screens()) {
            if (s->name() == target) {
                screen = s;
                break;
            }
        }
        if (!screen) return;
        m_dpr = screen->devicePixelRatio();
        // screenshot
        shot = screen->grabWindow(0);
    } else {
        // find camera
        QCameraDevice cameraDevice;
        for (const QCameraDevice &c: QMediaDevices::videoInputs()) {
            if (c.description() == target) {
                cameraDevice = c;
                break;
            }
        }
        if (cameraDevice.isNull()) return;
        m_dpr = 1;
        // take picture
        const auto camera = new QCamera(cameraDevice, this);
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
    }
    // image process
    m_shot = shot;

    // if (m_process != RAW) {
    //     QImage image = shot.toImage();
    //     image.setDevicePixelRatio(1.0);
    //     cv::Mat cvImg(image.height(), image.width(),
    //                   image.format() == QImage::Format_RGB32 ? CV_8UC4 : CV_8UC3,
    //                   image.bits(),
    //                   image.bytesPerLine());
    //     cv::Mat processed;
    //     switch (m_process) {
    //         case THRESHOLD: {
    //             cv::Mat gray;
    //             cv::cvtColor(cvImg, gray, cv::COLOR_BGRA2GRAY);
    //             cv::threshold(gray, processed, 128, 255, cv::THRESH_BINARY);
    //             cv::cvtColor(processed, processed, cv::COLOR_GRAY2BGRA);
    //             break;
    //         }
    //         break;
    //         default: break;
    //     }
    //     QImage result(
    //         processed.data,
    //         processed.cols,
    //         processed.rows,
    //         processed.step,
    //         image.format()
    //     );
    //     shot = QPixmap::fromImage(result.copy());
    // }

    // show in graphics view
    auto *scene = new QGraphicsScene(m_graphicsView); // NOLINT
    auto *item = scene->addPixmap(shot);
    item->setTransformationMode(Qt::FastTransformation);
    m_graphicsView->setScene(scene);
    m_graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

    selectionRefresh();
    ocrRefresh();
}

void AreaSelectDialog::reload(const QJsonObject &config) const {
    // load charset string

    // load area list
    m_selectionModel->clear();
    const QJsonArray areaList = config["areaList"].toArray();
    for (const QJsonValue &value: areaList) {
        QJsonArray areaArray = value.toArray();
        const int x = areaArray[0].toInt();
        const int y = areaArray[1].toInt();
        const int width = areaArray[2].toInt();
        const int height = areaArray[3].toInt();
        const auto physicalRect = QRectF(x, y, width, height);
        const auto logicalRect = QRectF(physicalRect.x() / m_dpr, physicalRect.y() / m_dpr, physicalRect.width() / m_dpr, physicalRect.height() / m_dpr);
        auto *item = new QStandardItem();
        const QPixmap cropped = m_shot.copy(physicalRect.toRect());
        const QString recognizedText = ocr(cropped, m_charsetString);
        item->setText(recognizedText);
        item->setData(QVariant::fromValue(logicalRect), Qt::UserRole + 1);
        item->setData(QVariant::fromValue(physicalRect), Qt::UserRole + 2);
        item->setFlags(item->flags() &= ~Qt::ItemIsDropEnabled);
        m_selectionModel->appendRow(item);
    }
    selectionRefresh();
}

QJsonArray AreaSelectDialog::areaExport() const {
    QJsonArray areaList;
    for (int row = 0; row < m_selectionModel->rowCount(); ++row) {
        const QStandardItem *item = m_selectionModel->item(row);
        const auto physicalRect = item->data(Qt::UserRole + 2).value<QRectF>().toRect();
        QJsonArray areaArray = {physicalRect.x(), physicalRect.y(), physicalRect.width(), physicalRect.height()};
        areaList.append(areaArray);
    }
    return areaList;
}

// AreaSelectDialog protected
bool AreaSelectDialog::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_selectionListView && event->type() == QEvent::KeyPress) {
        switch (static_cast<QKeyEvent *>(event)->key()) {
            case Qt::Key_Delete: {
                const int row = m_selectionListView->currentIndex().row();
                m_selectionModel->removeRow(row);
                return true;
            }
            default:
                break;
        }
    }
    return QDialog::eventFilter(obj, event);
}

// AreaSelectDialog private
void AreaSelectDialog::select(const bool status) const {
    if (status) {
        m_graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    } else {
        m_graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    }
}

void AreaSelectDialog::charsetRefresh() {
    QStringList characterStringList;
    for (int row = 0; row < m_charsetModel->rowCount(); ++row) {
        const QStandardItem *item = m_charsetModel->item(row);
        if (item->checkState() == Qt::Checked) {
            const auto lang = item->data(Qt::UserRole + 1).value<QString>();
            characterStringList.append(lang);
        }
    }
    m_charsetString = characterStringList.join("+");
    ocrRefresh();
}

void AreaSelectDialog::selectionHandle(const QRectF &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint) {
    const bool rubberBandEnded = viewportRect.isNull();
    auto sceneRect = QRectF(fromScenePoint, toScenePoint);
    if (!sceneRect.isNull() && sceneRect.isValid()) {
        m_rectF = sceneRect;
    }
    if (rubberBandEnded) {
        const auto logicalRect = m_rectF;
        const auto physicalRect = QRectF(m_rectF.x() * m_dpr, m_rectF.y() * m_dpr, m_rectF.width() * m_dpr, m_rectF.height() * m_dpr);
        auto *item = new QStandardItem();
        const QPixmap cropped = m_shot.copy(physicalRect.toRect());
        const QString recognizedText = ocr(cropped, m_charsetString);
        item->setText(recognizedText);
        item->setData(QVariant::fromValue(logicalRect), Qt::UserRole + 1);
        item->setData(QVariant::fromValue(physicalRect), Qt::UserRole + 2);
        item->setFlags(item->flags() &= ~Qt::ItemIsDropEnabled);
        m_selectionModel->appendRow(item);
        selectionRefresh();
    }
}

void AreaSelectDialog::selectionRefresh() const {
    m_graphicsView->scene()->clear();
    m_graphicsView->scene()->addPixmap(m_shot);
    for (int row = 0; row < m_selectionModel->rowCount(); ++row) {
        const QStandardItem *item = m_selectionModel->item(row);
        const auto logicalRect = item->data(Qt::UserRole + 1).value<QRectF>().toRect();
        // gui
        auto *graphicsRectItem = new QGraphicsRectItem(logicalRect);
        m_graphicsView->scene()->addItem(graphicsRectItem);
        graphicsRectItem->setPen(QPen(Qt::red, 2));
        auto *graphicsTextItem = new QGraphicsSimpleTextItem(QString::number(row + 1));
        m_graphicsView->scene()->addItem(graphicsTextItem);
        graphicsTextItem->setPos(logicalRect.center() - graphicsTextItem->boundingRect().center());
        graphicsTextItem->setBrush(Qt::red);
        graphicsTextItem->setFont(QFont("consolas", 12));
    }
}

void AreaSelectDialog::ocrRefresh() const {
    for (int row = 0; row < m_selectionModel->rowCount(); ++row) {
        QStandardItem *item = m_selectionModel->item(row);
        const auto physicalRect = item->data(Qt::UserRole + 2).value<QRectF>().toRect();
        // update ocr result
        const QPixmap cropped = m_shot.copy(physicalRect);
        const QString recognizedText = ocr(cropped, m_charsetString);
        item->setText(recognizedText);
    }
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

// SerialPort public
SerialPort::SerialPort(const QJsonObject &portConfig, QObject *parent) : BasePort(parent), m_serialPort(new QSerialPort(this)) {
    // port config
    m_portName = portConfig["portName"].toString();
    m_baudRate = portConfig["baudRate"].toInt();
    m_dataBits = portConfig["dataBits"].toInt();
    m_parity = portConfig["parity"].toInt();
    m_stopBits = portConfig["stopBits"].toInt();
    // port init
    m_serialPort->setPortName(m_portName);
    m_serialPort->setBaudRate(m_baudRate);
    m_serialPort->setDataBits(static_cast<QSerialPort::DataBits>(m_dataBits));
    m_serialPort->setParity(static_cast<QSerialPort::Parity>(m_parity));
    m_serialPort->setStopBits(static_cast<QSerialPort::StopBits>(m_stopBits));
    // tx/rx config
    m_txFormat = portConfig["txFormat"].toString();
    m_txSuffix = portConfig["txSuffix"].toString();
    m_rxFormat = portConfig["rxFormat"].toString();
    // connect slot
    connect(m_serialPort, &QSerialPort::readyRead, this, [this] {
        handleRead(0, 0);
    });
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &SerialPort::handleError);
}

void SerialPort::reload(const QJsonObject &portConfig) {
    // port config
    m_portName = portConfig["portName"].toString();
    m_baudRate = portConfig["baudRate"].toInt();
    m_dataBits = portConfig["dataBits"].toInt();
    m_parity = portConfig["parity"].toInt();
    m_stopBits = portConfig["stopBits"].toInt();
    // port init
    m_serialPort->setPortName(m_portName);
    m_serialPort->setBaudRate(m_baudRate);
    m_serialPort->setDataBits(static_cast<QSerialPort::DataBits>(m_dataBits));
    m_serialPort->setParity(static_cast<QSerialPort::Parity>(m_parity));
    m_serialPort->setStopBits(static_cast<QSerialPort::StopBits>(m_stopBits));
    // tx config
    m_txFormat = portConfig["txFormat"].toString();
    m_txSuffix = portConfig["txSuffix"].toString();
    // rx config
    m_rxFormat = portConfig["rxFormat"].toString();
}

QHash<QString, QVariant> SerialPort::info() {
    const bool status = m_serialPort->isOpen();
    const QString portName = m_portName;
    const QString baudRate = QString::number(m_baudRate);
    const QString dataBits = QString::number(m_dataBits);
    QString parity;
    switch (m_parity) {
        case 0: parity = "no";
            break;
        case 2: parity = "even";
            break;
        case 3: parity = "odd";
            break;
        case 4: parity = "space";
            break;
        case 5: parity = "mark";
            break;
        default: parity = "unknown";
    }
    QString stopBits;
    switch (m_stopBits) {
        case 1: stopBits = "1";
            break;
        case 3: stopBits = "1.5";
            break;
        case 2: stopBits = "2";
            break;
        default: stopBits = "unknown";
    }

    QHash<QString, QVariant> infoHash;
    infoHash["status"] = status;
    infoHash["portName"] = portName;
    infoHash["baudRate"] = baudRate;
    infoHash["dataBits"] = dataBits;
    infoHash["parity"] = parity;
    infoHash["stopBits"] = stopBits;
    return infoHash;
}

bool SerialPort::open() {
    if (m_serialPort->open(QSerialPort::ReadWrite)) {
        emit appendLog(QString("%1 %2 %3").arg("serial port", m_portName, "opened"), "info");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, "serial port", m_portName, "opened");
        return true;
    }
    emit appendLog(QString("%1 %2 %3: %4").arg("serial port", m_portName, "open failed", m_serialPort->errorString()), "error");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3 %4: %5").arg(timestamp, "serial port", m_portName, "open failed", m_serialPort->errorString());
    return false;
}

void SerialPort::close() {
    m_serialPort->close();
    emit appendLog(QString("%1 %2 %3").arg("serial port", m_portName, "closed"), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, "serial port", m_portName, "closed");
}

void SerialPort::writeText(const QString &txText) {
    // tx text reformat
    QString f_txText = txText;
    // 1: remove space if tx format is hex
    if (m_txFormat == "hex") f_txText.remove(" ");
    // 2: convert to byte array
    QByteArray txData;
    if (m_txFormat == "hex") txData = QByteArray::fromHex(f_txText.toUtf8());
    else if (m_txFormat == "ascii") txData = f_txText.toLatin1();
    else /* txFormat == "utf-8" */ txData = f_txText.toUtf8();
    writeData(txData);
}

void SerialPort::writeData(const QByteArray &txData) {
    // check port status
    if (!m_serialPort->isOpen()) {
        emit appendLog(QString("serial port %1 is not opened").arg(m_portName), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] serial port %2 is not opened").arg(timestamp, m_portName);
        return;
    }
    // tx data reformat
    QByteArray f_txData = txData;
    // 1: append suffix according to tx suffix
    if (m_txSuffix == "crlf") f_txData += "\r\n";
    else if (m_txSuffix == "crc8 maxim") f_txData += crc8Maxim(txData);
    else if (m_txSuffix == "crc16 modbus") f_txData += modbusCRC(txData);
    else; /* m_txSuffix == "null" */
    // call handle write
    handleWrite(f_txData);
}

QString SerialPort::readText(const int timeout, const int length) {
    const QByteArray rxData = readData(timeout, length);
    if (rxData == "timeout") return "timeout";
    if (m_rxFormat == "hex") return m_rxBuffer.toHex().toUpper();
    if (m_rxFormat == "ascii") return QString::fromLatin1(m_rxBuffer);
    /* m_rxFormat == "utf-8" */
    return QString::fromUtf8(m_rxBuffer);
}

QByteArray SerialPort::readData(const int timeout, const int length) {
    QByteArray rxData;
    // async mode
    if (timeout == 0) {
        rxData = m_rxBuffer;
    }
    // sync mode
    else {
        disconnect(m_serialPort, &QSerialPort::readyRead, this, nullptr);
        rxData = handleRead(timeout, length);
        connect(m_serialPort, &QSerialPort::readyRead, this, [this] { handleRead(0, 0); });
    }
    return rxData;
}

// SerialPort private
void SerialPort::handleWrite(const QByteArray &f_txData) {
    m_serialPort->write(f_txData);
    // tx message reformat
    QString txMessage;
    // 1: encode tx message according to tx format
    if (m_txFormat == "hex") txMessage = f_txData.toHex(' ').toUpper();
    else if (m_txFormat == "ascii") txMessage = QString::fromLatin1(f_txData);
    else /* m_txFormat == "utf-8" */ txMessage = QString::fromUtf8(f_txData);
    // 2: add port info
    txMessage = QString("[%1] -&gt; %2").arg(m_serialPort->portName(), txMessage);
    emit appendLog(txMessage, "tx");
}

QByteArray SerialPort::handleRead(const int timeout, const int length) {
    QByteArray rxData = m_serialPort->readAll();
    if (timeout != 0 && length != 0) {
        int time = 0;
        while (rxData.size() != length) {
            if (m_serialPort->waitForReadyRead(10)) {
                rxData += m_serialPort->readAll();
            }
            time += 10;
            if (time >= timeout) {
                rxData = "timeout";
                break;
            }
        }
    }
    m_rxBuffer = rxData;
    // rx message reformat
    QString rxMessage;
    // 1: encode rx message according to rx format
    if (m_rxFormat == "hex") rxMessage = rxData.toHex(' ').toUpper();
    else if (m_rxFormat == "ascii") rxMessage = QString::fromLatin1(rxData);
    else /* m_rxFormat == "utf-8" */ rxMessage = QString::fromUtf8(rxData);
    // 2: add port info
    rxMessage = QString("[%1] &lt;- %2").arg(m_serialPort->portName(), rxMessage);
    emit appendLog(rxMessage, "rx");
    return rxData;
}

void SerialPort::handleError() {
}

// TcpClient public
TcpClient::TcpClient(const QJsonObject &portConfig, QObject *parent) : BasePort(parent), m_tcpClient(new QTcpSocket(this)) {
    // port config
    m_portName = portConfig["portName"].toString();
    m_tcpClientRemoteAddress = portConfig["tcpClientRemoteAddress"].toString();
    m_tcpClientRemotePort = portConfig["tcpClientRemotePort"].toInt();
    // port init
    m_tcpClient->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    m_tcpClient->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    // tx/rx config
    m_txFormat = portConfig["txFormat"].toString();
    m_txSuffix = portConfig["txSuffix"].toString();
    m_rxFormat = portConfig["rxFormat"].toString();
    // connect slot
    connect(m_tcpClient, &QTcpSocket::connected, this, &TcpClient::handleConnected);
    connect(m_tcpClient, &QTcpSocket::disconnected, this, &TcpClient::handleDisconnected);
    connect(m_tcpClient, &QTcpSocket::readyRead, this, [this] { handleRead(0, 0); });
    connect(m_tcpClient, &QTcpSocket::errorOccurred, this, &TcpClient::handleError);
}

void TcpClient::reload(const QJsonObject &portConfig) {
    // port config
    m_tcpClientRemoteAddress = portConfig["tcpClientRemoteAddress"].toString();
    m_tcpClientRemotePort = portConfig["tcpClientRemotePort"].toInt();
    // tx config
    m_txFormat = portConfig["txFormat"].toString();
    m_txSuffix = portConfig["txSuffix"].toString();
    // rx config
    m_rxFormat = portConfig["rxFormat"].toString();
}

QHash<QString, QVariant> TcpClient::info() {
    QString status;
    switch (m_tcpClient->state()) {
        case QAbstractSocket::UnconnectedState: status = "unconnected";
            break;
        case QAbstractSocket::HostLookupState: status = "looking up host";
            break;
        case QAbstractSocket::ConnectingState: status = "connecting";
            break;
        case QAbstractSocket::ConnectedState: status = "connected";
            break;
        case QAbstractSocket::ClosingState: status = "closing";
            break;
        case QAbstractSocket::BoundState: status = "bound to local address";
            break;
        default: status = "unknown";
    }
    const QString localAddress = m_tcpClient->localAddress().toString();
    const QString localPort = QString::number(m_tcpClient->localPort());
    const QString remoteAddress = m_tcpClientRemoteAddress;
    const QString remotePort = QString::number(m_tcpClientRemotePort);

    QHash<QString, QVariant> infoHash;
    infoHash["status"] = status;
    infoHash["localAddress"] = localAddress;
    infoHash["localPort"] = localPort;
    infoHash["remoteAddress"] = remoteAddress;
    infoHash["remotePort"] = remotePort;
    return infoHash;
}

bool TcpClient::open() {
    m_tcpClient->connectToHost(m_tcpClientRemoteAddress, m_tcpClientRemotePort);
    return true;
}

void TcpClient::close() {
    m_tcpClient->disconnectFromHost();
}

void TcpClient::writeText(const QString &txText) {
    // tx text reformat
    QString f_txText = txText;
    // 1: remove space if tx format is hex
    if (m_txFormat == "hex") f_txText.remove(" ");
    // 2: convert to byte array
    QByteArray txData;
    if (m_txFormat == "hex") txData = QByteArray::fromHex(f_txText.toUtf8());
    else if (m_txFormat == "ascii") txData = f_txText.toLatin1();
    else /* txFormat == "utf-8" */ txData = f_txText.toUtf8();
    writeData(txData);
}

void TcpClient::writeData(const QByteArray &txData) {
    // check port status
    if (!m_tcpClient->isOpen()) {
        emit appendLog("tcp client is not opened", "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] tcp client is not opened").arg(timestamp);
        return;
    }
    // tx data reformat
    QByteArray f_txData = txData;
    // 1: append suffix according to tx suffix
    if (m_txSuffix == "crlf") f_txData += "\r\n";
    else if (m_txSuffix == "crc8 maxim") f_txData += crc8Maxim(txData);
    else if (m_txSuffix == "crc16 modbus") f_txData += modbusCRC(txData);
    else; /* m_txSuffix == "null" */
    // call handle write
    handleWrite(f_txData);
}

QString TcpClient::readText(const int timeout, const int length) {
    const QByteArray rxData = readData(timeout, length);
    if (rxData == "timeout") return "timeout";
    if (m_rxFormat == "hex") return m_rxBuffer.toHex().toUpper();
    if (m_rxFormat == "ascii") return QString::fromLatin1(m_rxBuffer);
    /* m_rxFormat == "utf-8" */
    return QString::fromUtf8(m_rxBuffer);
}

QByteArray TcpClient::readData(const int timeout, const int length) {
    QByteArray rxData;
    // async mode
    if (timeout == 0) {
        rxData = m_rxBuffer;
    }
    // sync mode
    else {
        disconnect(m_tcpClient, &QTcpSocket::readyRead, this, nullptr);
        rxData = handleRead(timeout, length);
        connect(m_tcpClient, &QTcpSocket::readyRead, this, [this] { handleRead(0, 0); });
    }
    return rxData;
}

// TcpClient private
void TcpClient::handleConnected() {
    m_tcpClientLocalAddress = m_tcpClient->localAddress().toString();
    m_tcpClientLocalPort = m_tcpClient->localPort();
    emit appendLog(QString("%1 %2:%3").arg("tcp client connected to", m_tcpClientRemoteAddress, QString::number(m_tcpClientRemotePort)), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3:%4").arg(timestamp, "tcp client connected to", m_tcpClientRemoteAddress, QString::number(m_tcpClientRemotePort));
}

void TcpClient::handleDisconnected() {
    emit appendLog(QString("%1 %2:%3").arg("tcp client disconnected from", m_tcpClientRemoteAddress, QString::number(m_tcpClientRemotePort)), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3:%4").arg(timestamp, "tcp client disconnected from", m_tcpClientRemoteAddress, QString::number(m_tcpClientRemotePort));
}

void TcpClient::handleError() {
}

void TcpClient::handleWrite(const QByteArray &f_txData) {
    m_tcpClient->write(f_txData);
    // tx message reformat
    QString txMessage;
    // 1: encode tx message according to tx format
    if (m_txFormat == "hex") txMessage = f_txData.toHex(' ').toUpper();
    else if (m_txFormat == "ascii") txMessage = QString::fromLatin1(f_txData);
    else /* m_txFormat == "utf-8" */ txMessage = QString::fromUtf8(f_txData);
    // 2: add port info
    txMessage = QString("[%1:%2 -&gt; %3:%4] %5").arg(m_tcpClientLocalAddress, QString::number(m_tcpClientLocalPort), m_tcpClientRemoteAddress,
                                                      QString::number(m_tcpClientRemotePort), txMessage);
    emit appendLog(txMessage, "tx");
}

QByteArray TcpClient::handleRead(const int timeout, const int length) {
    QByteArray rxData = m_tcpClient->readAll();
    if (timeout != 0 && length != 0) {
        int time = 0;
        while (rxData.size() != length) {
            if (!m_tcpClient->waitForReadyRead(10)) {
                rxData += m_tcpClient->readAll();
            }
            time += 10;
            if (time >= timeout) {
                rxData = "timeout";
                break;
            }
        }
    }
    m_rxBuffer = rxData;
    // rx message reformat
    QString rxMessage;
    // 1: encode rx message according to rx format
    if (m_rxFormat == "hex") rxMessage = rxData.toHex(' ').toUpper();
    else if (m_rxFormat == "ascii") rxMessage = QString::fromLatin1(rxData);
    else /* m_rxFormat == "utf-8" */ rxMessage = QString::fromUtf8(rxData);
    // 2: add port info
    rxMessage = QString("[%1:%2 &lt;- %3:%4] %5").arg(m_tcpClientLocalAddress, QString::number(m_tcpClientLocalPort), m_tcpClientRemoteAddress,
                                                      QString::number(m_tcpClientRemotePort), rxMessage);
    emit appendLog(rxMessage, "rx");
    return rxData;
}

// TcpServer public
TcpServer::TcpServer(const QJsonObject &portConfig, QObject *parent) : BasePort(parent), m_tcpServer(new QTcpServer(this)) {
    // port config
    m_portName = portConfig["portName"].toString();
    m_tcpServerLocalAddress = portConfig["tcpServerLocalAddress"].toString();
    m_tcpServerLocalPort = portConfig["tcpServerLocalPort"].toInt();
    // port init
    // m_tcpServer->setMaxPendingConnections();
    // tx/rx config
    m_txFormat = portConfig["txFormat"].toString();
    m_txSuffix = portConfig["txSuffix"].toString();
    m_rxFormat = portConfig["rxFormat"].toString();
    // connect slot
    connect(m_tcpServer, &QTcpServer::newConnection, this, &TcpServer::handleNewConnection);
    connect(m_tcpServer, &QTcpServer::acceptError, this, &TcpServer::handleServerError);
}

void TcpServer::reload(const QJsonObject &portConfig) {
    // port config
    m_tcpServerLocalAddress = portConfig["tcpServerLocalAddress"].toString();
    m_tcpServerLocalPort = portConfig["tcpServerLocalPort"].toInt();
    // tx config
    m_txFormat = portConfig["txFormat"].toString();
    m_txSuffix = portConfig["txSuffix"].toString();
    // rx config
    m_rxFormat = portConfig["rxFormat"].toString();
}

QHash<QString, QVariant> TcpServer::info() {
    QString status;
    if (m_tcpServer->isListening())
        status = "listening";
    else
        status = "idle";
    QString localAddress = m_tcpServerLocalAddress;
    QString localPort = QString::number(m_tcpServerLocalPort);
    QString message = QString("(%1) local ip: %2:%3 remote ip: [").arg(status, localAddress, localPort);
    for (QTcpSocket *tcpServerPeer: m_tcpServerPeerList) {
        QString peerIp = QString("%1:%2 ").arg(tcpServerPeer->peerAddress().toString(), QString::number(tcpServerPeer->peerPort()));
        message.append(peerIp);
    }
    message.append("]");
    return {};
    // return message;
}

bool TcpServer::open() {
    if (m_tcpServer->listen(QHostAddress(m_tcpServerLocalAddress), m_tcpServerLocalPort)) {
        emit appendLog(QString("%1 %2:%3").arg("tcp server started on", m_tcpServerLocalAddress, QString::number(m_tcpServerLocalPort)), "info");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3:%4").arg(timestamp, "tcp server started on", m_tcpServerLocalAddress, QString::number(m_tcpServerLocalPort));
        return true;
    }
    emit appendLog(QString("%1: %2").arg("tcp server open failed", m_tcpServer->errorString()), "error");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2: %3").arg(timestamp, "tcp server open failed", m_tcpServer->errorString());
    return false;
}

void TcpServer::close() {
    m_tcpServer->close();
    for (QTcpSocket *tcpServerPeer: m_tcpServerPeerList) {
        if (tcpServerPeer) {
            tcpServerPeer->disconnectFromHost();
            if (tcpServerPeer->state() != QAbstractSocket::UnconnectedState) {
                tcpServerPeer->waitForDisconnected(1000);
            }
            tcpServerPeer->deleteLater();
        }
    }
    m_tcpServerPeerList.clear();
    emit appendLog("tcp server closed", "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "tcp server closed");
}

void TcpServer::writeText(const QString &txText) {
    // tx text reformat
    QString f_txText = txText;
    // 1: remove space if tx format is hex
    if (m_txFormat == "hex") f_txText.remove(" ");
    // 2: convert to byte array
    QByteArray txData;
    if (m_txFormat == "hex") txData = QByteArray::fromHex(f_txText.toUtf8());
    else if (m_txFormat == "ascii") txData = f_txText.toLatin1();
    else /* txFormat == "utf-8" */ txData = f_txText.toUtf8();
    writeData(txData);
}

void TcpServer::writeText(const QString &txText, const QString &peerIp) {
    // tx text reformat
    QString f_txText = txText;
    // 1: remove space if tx format is hex
    if (m_txFormat == "hex") f_txText.remove(" ");
    // 2: convert to byte array
    QByteArray txData;
    if (m_txFormat == "hex") txData = QByteArray::fromHex(f_txText.toUtf8());
    else if (m_txFormat == "ascii") txData = f_txText.toLatin1();
    else /* txFormat == "utf-8" */ txData = f_txText.toUtf8();
    writeData(txData, peerIp);
}

void TcpServer::writeData(const QByteArray &txData) {
    // check port status
    if (!m_tcpServer->isListening()) {
        emit appendLog("tcp server is not opened", "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "tcp server is not opened");
        return;
    }
    // tx data reformat
    QByteArray f_txData = txData;
    // 1: append suffix according to tx suffix
    if (m_txSuffix == "crlf") f_txData += "\r\n";
    else if (m_txSuffix == "crc8 maxim") f_txData += crc8Maxim(txData);
    else if (m_txSuffix == "crc16 modbus") f_txData += modbusCRC(txData);
    else; /* m_txSuffix == "null" */
    // call handle write
    handleWrite(f_txData);
}

void TcpServer::writeData(const QByteArray &txData, const QString &peerIp) {
    // check port status
    if (!m_tcpServer->isListening()) {
        emit appendLog("tcp server is not opened", "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "tcp server is not opened");
        return;
    }
    // tx data reformat
    QByteArray f_txData = txData;
    // 1: append suffix according to tx suffix
    if (m_txSuffix == "crlf") f_txData += "\r\n";
    else if (m_txSuffix == "crc8 maxim") f_txData += crc8Maxim(txData);
    else if (m_txSuffix == "crc16 modbus") f_txData += modbusCRC(txData);
    else; /* m_txSuffix == "null" */
    // call handle write
    handleWrite(f_txData, peerIp);
}

QString TcpServer::readText(const int timeout, const int length, const QString &peerIp) {
    const QByteArray rxData = readData(timeout, length, peerIp);
    if (rxData == "timeout") return "timeout";
    if (m_rxFormat == "hex") return m_rxBuffer.toHex().toUpper();
    if (m_rxFormat == "ascii") return QString::fromLatin1(m_rxBuffer);
    /* m_rxFormat == "utf-8" */
    return QString::fromUtf8(m_rxBuffer);
}

QByteArray TcpServer::readData(const int timeout, const int length, const QString &peerIp) {
    QByteArray rxData;
    // async mode
    if (timeout == 0) {
        rxData = m_rxBuffer;
    }
    // sync mode
    else {
        foreach(QTcpSocket* tcpServerPeer, m_tcpServerPeerList) {
            disconnect(tcpServerPeer, &QTcpSocket::readyRead, this, nullptr);
            rxData = handleRead(timeout, length, tcpServerPeer);
            connect(tcpServerPeer, &QTcpSocket::readyRead, this, [this, tcpServerPeer] { handleRead(0, 0, tcpServerPeer); });
        }
    }
    return rxData;
}

// TcpServer private
void TcpServer::handleNewConnection() {
    while (m_tcpServer->hasPendingConnections()) {
        QTcpSocket *tcpServerPeer = m_tcpServer->nextPendingConnection();
        handleConnected(tcpServerPeer);
        connect(tcpServerPeer, &QTcpSocket::readyRead, this, [this, tcpServerPeer] { handleRead(0, 0, tcpServerPeer); });
        connect(tcpServerPeer, &QTcpSocket::disconnected, this, [this, tcpServerPeer] { handleDisconnected(tcpServerPeer); });
        connect(tcpServerPeer, &QTcpSocket::errorOccurred, this, [this, tcpServerPeer](QAbstractSocket::SocketError error) { handleError(tcpServerPeer); });
    }
}

void TcpServer::handleServerError() {
};

void TcpServer::handleConnected(QTcpSocket *tcpServerPeer) {
    m_tcpServerPeerList.append(tcpServerPeer);
    QString peerAddress = tcpServerPeer->peerAddress().toString();
    QString peerPort = QString::number(tcpServerPeer->peerPort());
    emit appendLog(QString("%1 %2:%3").arg("new client connected", peerAddress, peerPort), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3:%4").arg(timestamp, "new client connected", peerAddress, peerPort);
}

void TcpServer::handleDisconnected(QTcpSocket *tcpServerPeer) {
    m_tcpServerPeerList.removeOne(tcpServerPeer);
    QString peerAddress = tcpServerPeer->peerAddress().toString();
    QString peerPort = QString::number(tcpServerPeer->peerPort());
    emit appendLog(QString("%1 %2:%3").arg("client disconnected", peerAddress, peerPort), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3:%4").arg(timestamp, "client disconnected", peerAddress, peerPort);
}

void TcpServer::handleError(QTcpSocket *tcpServerPeer) {
}

void TcpServer::handleWrite(const QByteArray &f_txData, const QString &peerIp) {
    if (peerIp == "") {
        foreach(QTcpSocket* tcpServerPeer, m_tcpServerPeerList) {
            tcpServerPeer->write(f_txData);
        }
        // tx message reformat
        QString txMessage;
        // 1: encode tx message according to tx format
        if (m_txFormat == "hex") txMessage = f_txData.toHex(' ').toUpper();
        else if (m_txFormat == "ascii") txMessage = QString::fromLatin1(f_txData);
        else /* m_txFormat == "utf-8" */ txMessage = QString::fromUtf8(f_txData);
        // 2: add port info
        txMessage = QString("[%1:%2 -&gt; %3] %4").arg(m_tcpServerLocalAddress, QString::number(m_tcpServerLocalPort), "broadcast", txMessage);
        emit appendLog(txMessage, "tx");
    } else {
        QTcpSocket *tcpServerPeer = nullptr;
        foreach(QTcpSocket* peer, m_tcpServerPeerList) {
            if (peerIp == QString("%1:%2").arg(peer->peerAddress().toString(), QString::number(peer->peerPort()))) {
                tcpServerPeer = peer;
                break;
            }
        }
        if (tcpServerPeer == nullptr) {
            emit appendLog("peer not found", "error");
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] %2").arg(timestamp, "peer not found");
            return;
        }
        tcpServerPeer->write(f_txData);
        // tx message reformat
        QString txMessage;
        // 1: encode tx message according to tx format
        if (m_txFormat == "hex") txMessage = f_txData.toHex(' ').toUpper();
        else if (m_txFormat == "ascii") txMessage = QString::fromLatin1(f_txData);
        else /* m_txFormat == "utf-8" */ txMessage = QString::fromUtf8(f_txData);
        // 2: add port info
        QString peerAddress = tcpServerPeer->peerAddress().toString();
        QString peerPort = QString::number(tcpServerPeer->peerPort());
        txMessage = QString("[%1:%2 -&gt; %3:%4] %5").arg(m_tcpServerLocalAddress, QString::number(m_tcpServerLocalPort), peerAddress, peerPort, txMessage);
        emit appendLog(txMessage, "tx");
    }
}

QByteArray TcpServer::handleRead(const int timeout, const int length, QTcpSocket *tcpServerPeer) {
    QByteArray rxData = tcpServerPeer->readAll();
    if (timeout != 0 && length != 0) {
        int time = 0;
        while (rxData.size() != length) {
            if (!tcpServerPeer->waitForReadyRead(10)) {
                rxData += tcpServerPeer->readAll();
            }
            time += 10;
            if (time >= timeout) {
                rxData = "timeout";
                break;
            }
        }
    }
    m_rxBuffer = rxData;
    // rx message reformat
    QString peerAddress = tcpServerPeer->peerAddress().toString();
    QString peerPort = QString::number(tcpServerPeer->peerPort());
    QString rxMessage;
    // 1: encode rx message according to rx format
    if (m_rxFormat == "hex") rxMessage = rxData.toHex(' ').toUpper();
    else if (m_rxFormat == "ascii") rxMessage = QString::fromLatin1(rxData);
    else /* m_rxFormat == "utf-8" */ rxMessage = QString::fromUtf8(rxData);
    // 2: add port info
    rxMessage = QString("[%1:%2 &lt;- %3:%4] %5").arg(m_tcpServerLocalAddress, QString::number(m_tcpServerLocalPort), peerAddress,
                                                      peerPort, rxMessage);
    emit appendLog(rxMessage, "rx");
    return rxData;
}

// UdpSocket public
UdpSocket::UdpSocket(const QJsonObject &portConfig, QObject *parent) : BasePort(parent), m_udpSocket(new QUdpSocket(this)) {
    // port config
    m_portName = portConfig["portName"].toString();
    m_udpSocketLocalAddress = portConfig["udpSocketLocalAddress"].toString();
    m_udpSocketLocalPort = portConfig["udpSocketLocalPort"].toInt();
    m_udpSocketRemoteAddress = portConfig["udpSocketRemoteAddress"].toString();
    m_udpSocketRemotePort = portConfig["udpSocketRemotePort"].toInt();
    // tx/rx config
    m_txFormat = portConfig["txFormat"].toString();
    m_txSuffix = portConfig["txSuffix"].toString();
    m_rxFormat = portConfig["rxFormat"].toString();
    // connect slot
    connect(m_udpSocket, &QUdpSocket::readyRead, this, [this] { handleRead(0, 0); });
    connect(m_udpSocket, &QUdpSocket::errorOccurred, this, &UdpSocket::handleError);
}

void UdpSocket::reload(const QJsonObject &portConfig) {
    // port config
    m_portName = portConfig["portName"].toString();
    m_udpSocketLocalAddress = portConfig["udpSocketLocalAddress"].toString();
    m_udpSocketLocalPort = portConfig["udpSocketLocalPort"].toInt();
    m_udpSocketRemoteAddress = portConfig["udpSocketRemoteAddress"].toString();
    m_udpSocketRemotePort = portConfig["udpSocketRemotePort"].toInt();
    // tx/rx config
    m_txFormat = portConfig["txFormat"].toString();
    m_txSuffix = portConfig["txSuffix"].toString();
    m_rxFormat = portConfig["rxFormat"].toString();
}

QHash<QString, QVariant> UdpSocket::info() {
    bool status;
    if (m_udpSocket->state() == QAbstractSocket::ConnectedState)
        status = true;
    else
        status = false;
    const QString localAddress = m_udpSocketLocalAddress;
    const QString localPort = QString::number(m_udpSocketLocalPort);
    const QString remoteAddress = m_udpSocketRemoteAddress;
    const QString remotePort = QString::number(m_udpSocketRemotePort);

    QHash<QString, QVariant> infoHash;
    infoHash["status"] = status;
    infoHash["localAddress"] = localAddress;
    infoHash["localPort"] = localPort;
    infoHash["remoteAddress"] = remoteAddress;
    infoHash["remotePort"] = remotePort;
    return infoHash;
}

bool UdpSocket::open() {
    if (!m_udpSocket->bind(QHostAddress(m_udpSocketLocalAddress), m_udpSocketLocalPort)) {
        emit appendLog(QString("udp socket open failed: %1").arg(m_udpSocket->errorString()), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] udp socket open failed: %2").arg(timestamp, m_udpSocket->errorString());
        return false;
    }
    m_udpSocket->connectToHost(m_udpSocketRemoteAddress, m_udpSocketRemotePort);
    emit appendLog(QString("udp socket opened: %1:%2->%3:%4").arg(m_udpSocketLocalAddress, QString::number(m_udpSocketLocalPort), m_udpSocketRemoteAddress,
                                                                  QString::number(m_udpSocketRemotePort)), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] udp socket opened: %2:%3->%4:%5").arg(timestamp, m_udpSocketLocalAddress, QString::number(m_udpSocketLocalPort), m_udpSocketRemoteAddress,
                                                                    QString::number(m_udpSocketRemotePort));
    return true;
}

void UdpSocket::close() {
    m_udpSocket->close();
}

void UdpSocket::writeText(const QString &txText) {
    // tx text reformat
    QString f_txText = txText;
    // 1: remove space if tx format is hex
    if (m_txFormat == "hex") f_txText.remove(" ");
    // 2: convert to byte array
    QByteArray txData;
    if (m_txFormat == "hex") txData = QByteArray::fromHex(f_txText.toUtf8());
    else if (m_txFormat == "ascii") txData = f_txText.toLatin1();
    else /* txFormat == "utf-8" */ txData = f_txText.toUtf8();
    writeData(txData);
}

void UdpSocket::writeData(const QByteArray &txData) {
    // check port status
    if (!m_udpSocket->isOpen()) {
        emit appendLog("udp socket is not opened", "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] udp socket is not opened").arg(timestamp);
        return;
    }
    // tx data reformat
    QByteArray f_txData = txData;
    // 1: append suffix according to tx suffix
    if (m_txSuffix == "crlf") f_txData += "\r\n";
    else if (m_txSuffix == "crc8 maxim") f_txData += crc8Maxim(txData);
    else if (m_txSuffix == "crc16 modbus") f_txData += modbusCRC(txData);
    else; /* m_txSuffix == "null" */
    // call handle write
    handleWrite(f_txData);
}

QString UdpSocket::readText(const int timeout, const int length) {
    const QByteArray rxData = readData(timeout, length);
    if (rxData == "timeout") return "timeout";
    if (m_rxFormat == "hex") return m_rxBuffer.toHex().toUpper();
    if (m_rxFormat == "ascii") return QString::fromLatin1(m_rxBuffer);
    /* m_rxFormat == "utf-8" */
    return QString::fromUtf8(m_rxBuffer);
}

QByteArray UdpSocket::readData(const int timeout, const int length) {
    QByteArray rxData;
    // async mode
    if (timeout == 0) {
        rxData = m_rxBuffer;
    }
    // sync mode
    else {
        disconnect(m_udpSocket, &QUdpSocket::readyRead, this, nullptr);
        rxData = handleRead(timeout, length);
        connect(m_udpSocket, &QUdpSocket::readyRead, this, [this] { handleRead(0, 0); });
    }
    return rxData;
}

// UdpSocket private
void UdpSocket::handleError() {
}

void UdpSocket::handleWrite(const QByteArray &f_txData) {
    m_udpSocket->write(f_txData);
    // tx message reformat
    QString txMessage;
    // 1: encode tx message according to tx format
    if (m_txFormat == "hex") txMessage = f_txData.toHex(' ').toUpper();
    else if (m_txFormat == "ascii") txMessage = QString::fromLatin1(f_txData);
    else /* m_txFormat == "utf-8" */ txMessage = QString::fromUtf8(f_txData);
    // 2: add port info
    txMessage = QString("[%1:%2 -&gt; %3:%4] %5").arg(m_udpSocketLocalAddress, QString::number(m_udpSocketLocalPort), m_udpSocketRemoteAddress,
                                                      QString::number(m_udpSocketRemotePort), txMessage);
    emit appendLog(txMessage, "tx");
}

QByteArray UdpSocket::handleRead(const int timeout, const int length) {
    QByteArray rxData = m_udpSocket->readAll();
    if (timeout != 0 && length != 0) {
        int time = 0;
        while (rxData.size() != length) {
            if (!m_udpSocket->waitForReadyRead(10)) {
                rxData += m_udpSocket->readAll();
            }
            time += 10;
            if (time >= timeout) {
                rxData = "timeout";
                break;
            }
        }
    }
    m_rxBuffer = rxData;
    // rx message reformat
    QString rxMessage;
    // 1: encode rx message according to rx format
    if (m_rxFormat == "hex") rxMessage = rxData.toHex(' ').toUpper();
    else if (m_rxFormat == "ascii") rxMessage = QString::fromLatin1(rxData);
    else /* m_rxFormat == "utf-8" */ rxMessage = QString::fromUtf8(rxData);
    // 2: add port info
    rxMessage = QString("[%1:%2 &lt;- %3:%4] %5").arg(m_udpSocketLocalAddress, QString::number(m_udpSocketLocalPort), m_udpSocketRemoteAddress,
                                                      QString::number(m_udpSocketRemotePort), rxMessage);
    emit appendLog(rxMessage, "rx");
    return rxData;
}

// Screen public
Screen::Screen(const QJsonObject &portConfig, QObject *parent) : BasePort(parent) {
    // port config
    m_portName = portConfig["portName"].toString();
    m_areaList = portConfig["areaList"].toArray();
}

void Screen::reload(const QJsonObject &portConfig) {
    // port config
    m_portName = portConfig["portName"].toString();
    m_areaList = portConfig["areaList"].toArray();
}

bool Screen::open() {
    return true;
}

void Screen::close() {
}

QHash<QString, QVariant> Screen::info() {
    return {};
}

QString Screen::readText(const int timeout, const int length) {
    // find screen
    for (QScreen *s: QGuiApplication::screens()) {
        if (s->name() == m_portName) {
            m_screen = s;
            break;
        }
    }
    if (!m_screen) return "screen not found";
    const auto shot = m_screen->grabWindow(0);
    QStringList resultList;
    for (const QJsonValue &value: m_areaList) {
        QJsonArray areaArray = value.toArray();
        const int x = areaArray[0].toInt();
        const int y = areaArray[1].toInt();
        const int width = areaArray[2].toInt();
        const int height = areaArray[3].toInt();
        const auto rect = QRect(x, y, width, height);
        const QPixmap cropped = shot.copy(rect);
        const QString text = ocr(cropped, "eng");
        resultList.append(text);
    }
    return resultList.join("\x1E");
}

// Camera public
Camera::Camera(const QJsonObject &portConfig, QObject *parent) : BasePort(parent) {
    // port config
    m_portName = portConfig["portName"].toString();
    m_areaList = portConfig["areaList"].toArray();
}

void Camera::reload(const QJsonObject &portConfig) {
    // port config
    m_portName = portConfig["portName"].toString();
    m_areaList = portConfig["areaList"].toArray();
}

bool Camera::open() {
    return true;
}

void Camera::close() {
}

QHash<QString, QVariant> Camera::info() {
    return {};
}

QString Camera::readText(const int timeout, const int length) {
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
    QStringList resultList;
    for (const QJsonValue &value: m_areaList) {
        QJsonArray areaArray = value.toArray();
        const int x = areaArray[0].toInt();
        const int y = areaArray[1].toInt();
        const int width = areaArray[2].toInt();
        const int height = areaArray[3].toInt();
        const auto rect = QRect(x, y, width, height);
        const QPixmap cropped = shot.copy(rect);
        const QString text = ocr(cropped, "eng");
        resultList.append(text);
    }
    return resultList.join("\x1E");
}
