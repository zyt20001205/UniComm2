#include "../include/port.h"

Port::Port(QObject *parent)
    : QDockWidget("port", qobject_cast<QWidget *>(parent)) {
    uiInit();
    portSettingUiInit();
}

void Port::uiInit() {
    m_tabWidget = new QTabWidget();
    setWidget(m_tabWidget);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &Port::portSelected);
    m_tabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tabWidget->tabBar(), &QTabBar::customContextMenuRequested, this, [this](const QPoint &pos) {
        const int index = m_tabWidget->tabBar()->tabAt(pos);
        portMenu(index, pos);
    });
    m_addButton = new QPushButton(m_tabWidget);
    m_addButton->setIcon(QIcon(":/icon/add.svg"));
    m_tabWidget->setCornerWidget(m_addButton, Qt::TopRightCorner);
    connect(m_addButton, &QPushButton::clicked, this, [this]() {
        portSettingLoad(-1);
    });
    // init port tab
    if (const auto portCount = m_portConfig.size(); portCount == 0) {
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "no port config found, create a welcome page");
        auto welcomePage = new QWidget(); // NOLINT
        auto welcomeLayout = new QVBoxLayout(welcomePage); // NOLINT
        auto welcomeLabel = new QLabel("welcome"); // NOLINT
        welcomeLayout->addWidget(welcomeLabel);
        m_tabWidget->addTab(welcomePage, "welcome");
    } else {
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3").arg(timestamp, QString::number(portCount), "port config found");
        for (int i = 0; i < portCount; ++i) {
            QJsonObject portConfig = m_portConfig[i].toObject();
            auto *pageWidget = new PageWidget(m_tabWidget); // NOLINT
            pageWidget->init(portConfig);
            QString portName = portConfig["portName"].toString();
            m_tabWidget->addTab(pageWidget, portName);
            connect(pageWidget, &PageWidget::appendLog, this, &Port::appendLog);
        }
    }
}

void Port::portConfigSave() const {
    g_config["portConfig"] = m_portConfig;
}

void Port::portOpen(const int index) const {
    if (index == -1) {
        const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(m_currentIndex));
        pageWidget->portOpen();
    } else {
        const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(index));
        pageWidget->portOpen();
    }
}

void Port::portClose(const int index) const {
    if (index == -1) {
        const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(m_currentIndex));
        pageWidget->portClose();
    } else {
        const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(index));
        pageWidget->portClose();
    }
}

QString Port::portInfo(const int index) const {
    if (index == -1) {
        const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(m_currentIndex));
        return pageWidget->portInfo();
    }
    const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(index));
    return pageWidget->portInfo();
}

void Port::portWrite(const int index, const QString &command, const QString &peerIp) const {
    if (index == -1) {
        const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(m_currentIndex));
        pageWidget->portWrite(command, peerIp);
    } else {
        const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(index));
        pageWidget->portWrite(command, peerIp);
    }
}

QString Port::portRead(const int index) const {
    if (index == -1) {
        const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(m_currentIndex));
        return pageWidget->portRead();
    }
    const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(index));
    return pageWidget->portRead();
}

void Port::portMenu(const int index, const QPoint &pos) {
    if (m_portConfig.size() == 0)
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
        m_tabWidget->removeTab(index);
    });
    menu.exec(m_tabWidget->tabBar()->mapToGlobal(pos));
}

void Port::portSelected(const int index) {
    if (m_portConfig.size() == 0)
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
    auto *pageWidget = new PageWidget(m_tabWidget); // NOLINT
    pageWidget->init(portConfig);
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
    qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, portType, portName, "removed");
    m_portConfig.removeAt(index);
}

void Port::portSettingUiInit() {
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

    // init screen/camera settings
    {
        m_screenNameWidget = new QWidget(m_portSettingDialog);
        m_portSettingLayout->addWidget(m_screenNameWidget);
        const auto screenLayout = new QHBoxLayout(m_screenNameWidget);
        screenLayout->setContentsMargins(0, 0, 0, 0);
        const auto screenNameLabel = new QLabel("screen name");
        screenLayout->addWidget(screenNameLabel);
        m_screenNameCombobox = new QComboBox();
        screenLayout->addWidget(m_screenNameCombobox);

        m_cameraNameWidget = new QWidget(m_portSettingDialog);
        m_portSettingLayout->addWidget(m_cameraNameWidget);
        const auto cameraLayout = new QHBoxLayout(m_cameraNameWidget);
        cameraLayout->setContentsMargins(0, 0, 0, 0);
        const auto cameraNameLabel = new QLabel("camera name");
        cameraLayout->addWidget(cameraNameLabel);
        m_cameraNameCombobox = new QComboBox();
        cameraLayout->addWidget(m_cameraNameCombobox);

        m_areaChooseDialog = new AreaSelectDialog(m_portSettingDialog);

        m_areaSelectWidget = new QWidget(m_portSettingDialog);
        m_portSettingLayout->addWidget(m_areaSelectWidget);
        const auto screenAreaLayout = new QHBoxLayout(m_areaSelectWidget);
        screenAreaLayout->setContentsMargins(0, 0, 0, 0);
        const auto screenAreaLabel = new QLabel("capture area");
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

        m_txIntervalWidget = new QWidget(m_portSettingDialog);
        m_portSettingLayout->addWidget(m_txIntervalWidget);
        const auto txIntervalLayout = new QHBoxLayout(m_txIntervalWidget); // NOLINT
        txIntervalLayout->setContentsMargins(0, 0, 0, 0);
        const auto txIntervalLabel = new QLabel("tx interval"); // NOLINT
        txIntervalLayout->addWidget(txIntervalLabel);
        m_txIntervalSpinBox = new QSpinBox();
        txIntervalLayout->addWidget(m_txIntervalSpinBox);
        m_txIntervalSpinBox->setRange(0, 1000);
        m_txIntervalSpinBox->setSuffix("ms");

        m_rxFormatWidget = new QWidget(m_portSettingDialog);
        m_portSettingLayout->addWidget(m_rxFormatWidget);
        const auto rxFormatLayout = new QHBoxLayout(m_rxFormatWidget); // NOLINT
        rxFormatLayout->setContentsMargins(0, 0, 0, 0);
        const auto rxFormatLabel = new QLabel("rx format"); // NOLINT
        rxFormatLayout->addWidget(rxFormatLabel);
        m_rxFormatCombobox = new QComboBox();
        rxFormatLayout->addWidget(m_rxFormatCombobox);
        m_rxFormatCombobox->addItems(QStringList{"hex", "ascii", "utf-8"});

        m_rxTimeoutWidget = new QWidget(m_portSettingDialog);
        m_portSettingLayout->addWidget(m_rxTimeoutWidget);
        const auto rxTimeoutLayout = new QHBoxLayout(m_rxTimeoutWidget); // NOLINT
        rxTimeoutLayout->setContentsMargins(0, 0, 0, 0);
        const auto rxTimeoutLabel = new QLabel("rx timeout"); // NOLINT
        rxTimeoutLayout->addWidget(rxTimeoutLabel);
        m_rxTimeoutSpinBox = new QSpinBox();
        rxTimeoutLayout->addWidget(m_rxTimeoutSpinBox);
        m_rxTimeoutSpinBox->setRange(0, 1000);
        m_rxTimeoutSpinBox->setSuffix("ms");
    }
    // init setting save button
    m_portSettingSavePushButton = new QPushButton("save setting");
    m_portSettingLayout->addWidget(m_portSettingSavePushButton);
    connect(m_portSettingSavePushButton, &QPushButton::clicked, this, [this]() {
        portSettingSave(m_portTypeCombobox->currentIndex());
    });
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
        m_txIntervalSpinBox->setValue(0);
        m_rxFormatCombobox->setCurrentText("hex");
        m_rxTimeoutSpinBox->setValue(0);
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
            m_txIntervalSpinBox->setValue(portInfo["txInterval"].toInt());
            m_rxFormatCombobox->setCurrentText(portInfo["rxFormat"].toString());
            m_rxTimeoutSpinBox->setValue(portInfo["rxTimeout"].toInt());
        } else if (portType == "tcp client") {
            m_tcpClientRemoteAddressLineEdit->setText(portInfo["tcpClientRemoteAddress"].toString());
            m_tcpClientRemotePortSpinBox->setValue(portInfo["tcpClientRemotePort"].toInt());
            m_txFormatCombobox->setCurrentText(portInfo["txFormat"].toString());
            m_txSuffixCombobox->setCurrentText(portInfo["txSuffix"].toString());
            m_txIntervalSpinBox->setValue(portInfo["txInterval"].toInt());
            m_rxFormatCombobox->setCurrentText(portInfo["rxFormat"].toString());
            m_rxTimeoutSpinBox->setValue(portInfo["rxTimeout"].toInt());
        } else if (portType == "tcp server") {
            m_tcpServerLocalAddressLineEdit->setText(portInfo["tcpServerLocalAddress"].toString());
            m_tcpServerLocalPortSpinBox->setValue(portInfo["tcpServerLocalPort"].toInt());
            m_txFormatCombobox->setCurrentText(portInfo["txFormat"].toString());
            m_txSuffixCombobox->setCurrentText(portInfo["txSuffix"].toString());
            m_txIntervalSpinBox->setValue(portInfo["txInterval"].toInt());
            m_rxFormatCombobox->setCurrentText(portInfo["rxFormat"].toString());
            m_rxTimeoutSpinBox->setValue(portInfo["rxTimeout"].toInt());
        } else if (portType == "udp socket") {
        } else if (portType == "screen") {
            m_screenNameCombobox->setCurrentText(portInfo["portName"].toString());
        } else /* portType == "camera" */ {
            m_cameraNameCombobox->setCurrentText(portInfo["portName"].toString());
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
    m_txIntervalWidget->hide();
    m_rxFormatWidget->hide();
    m_rxTimeoutWidget->hide();
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
        m_txIntervalWidget->show();
        m_rxFormatWidget->show();
        m_rxTimeoutWidget->show();
        m_portSettingSavePushButton->show();
    } else if (type == 2) {
        portSettingWidgetReset();
        m_portTypeCombobox->setEnabled(false);
        m_tcpClientRemoteAddressWidget->show();
        m_tcpClientRemotePortWidget->show();
        m_txFormatWidget->show();
        m_txSuffixWidget->show();
        m_txIntervalWidget->show();
        m_rxFormatWidget->show();
        m_rxTimeoutWidget->show();
        m_portSettingSavePushButton->show();
    } else if (type == 3) {
        portSettingWidgetReset();
        m_portTypeCombobox->setEnabled(false);
        m_tcpServerLocalAddressWidget->show();
        m_tcpServerLocalPortWidget->show();
        m_txFormatWidget->show();
        m_txSuffixWidget->show();
        m_txIntervalWidget->show();
        m_rxFormatWidget->show();
        m_rxTimeoutWidget->show();
        m_portSettingSavePushButton->show();
    } else if (type == 4) {
        portSettingWidgetReset();
        m_portTypeCombobox->setEnabled(false);
        m_txFormatWidget->show();
        m_txSuffixWidget->show();
        m_txIntervalWidget->show();
        m_rxFormatWidget->show();
        m_rxTimeoutWidget->show();
        m_portSettingSavePushButton->show();
    } else if (type == 5) {
        portSettingWidgetReset();
        m_portTypeCombobox->setEnabled(false);
        m_screenNameWidget->show();
        m_areaSelectWidget->show();
        disconnect(m_areaSelectPushButton, &QPushButton::clicked, this, nullptr);
        connect(m_areaSelectPushButton, &QPushButton::clicked, this, [this]() {
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
        connect(m_areaSelectPushButton, &QPushButton::clicked, this, [this]() {
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
        portConfig["txInterval"] = m_txIntervalSpinBox->value();
        portConfig["rxFormat"] = m_rxFormatCombobox->currentText();
        portConfig["rxTimeout"] = m_rxTimeoutSpinBox->value();
        if (m_currentIndex == -1) {
            if (m_portConfig.size() == 0) {
                m_tabWidget->removeTab(0);
            }
            m_portConfig.append(portConfig);
            auto *pageWidget = new PageWidget(m_tabWidget); // NOLINT
            pageWidget->init(portConfig);
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
        portConfig["txInterval"] = m_txIntervalSpinBox->value();
        portConfig["rxFormat"] = m_rxFormatCombobox->currentText();
        portConfig["rxTimeout"] = m_rxTimeoutSpinBox->value();
        if (m_currentIndex == -1) {
            if (m_portConfig.size() == 0) {
                m_tabWidget->removeTab(0);
            }
            m_portConfig.append(portConfig);
            auto *pageWidget = new PageWidget(m_tabWidget); // NOLINT
            pageWidget->init(portConfig);
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
        portConfig["txInterval"] = m_txIntervalSpinBox->value();
        portConfig["rxFormat"] = m_rxFormatCombobox->currentText();
        portConfig["rxTimeout"] = m_rxTimeoutSpinBox->value();
        if (m_currentIndex == -1) {
            if (m_portConfig.size() == 0) {
                m_tabWidget->removeTab(0);
            }
            m_portConfig.append(portConfig);
            auto *pageWidget = new PageWidget(m_tabWidget); // NOLINT
            pageWidget->init(portConfig);
            const QString portName = portConfig["portName"].toString();
            m_tabWidget->addTab(pageWidget, portName);
            connect(pageWidget, &PageWidget::appendLog, this, &Port::appendLog);
        } else {
            m_portConfig[m_currentIndex] = portConfig;
            const auto pageWidget = qobject_cast<PageWidget *>(m_tabWidget->widget(m_currentIndex));
            pageWidget->portReload(portConfig);
        }
    } else if (type == 4) {
    } else if (type == 5) {
        QJsonObject portConfig;
        portConfig["portType"] = "screen";
        portConfig["portName"] = m_screenNameCombobox->currentText();
        portConfig["area"] = m_areaChooseDialog->save();
        if (m_currentIndex == -1) {
            if (m_portConfig.size() == 0) {
                m_tabWidget->removeTab(0);
            }
            m_portConfig.append(portConfig);
            auto *pageWidget = new PageWidget(m_tabWidget); // NOLINT
            pageWidget->init(portConfig);
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
        portConfig["area"] = m_areaChooseDialog->save();
        if (m_currentIndex == -1) {
            if (m_portConfig.size() == 0) {
                m_tabWidget->removeTab(0);
            }
            m_portConfig.append(portConfig);
            auto *pageWidget = new PageWidget(m_tabWidget); // NOLINT
            pageWidget->init(portConfig);
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

AreaSelectDialog::AreaSelectDialog(QWidget *parent)
    : QDialog(parent) {
    this->setFixedSize(1280, 720);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_graphicsView = new QGraphicsView();
    layout->addWidget(m_graphicsView);
    auto *toolbar = new QToolBar();
    layout->addWidget(toolbar);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    const auto *refreshAction = toolbar->addAction(QIcon(":/icon/arrowClockwise.svg"), "refresh");
    connect(refreshAction, &QAction::triggered, this, [this]() {
        capture(m_type, m_target);
    });
    const auto *addAction = toolbar->addAction(QIcon(":/icon/crop.svg"), "crop");
    connect(addAction, &QAction::triggered, this, [this]() {
        crop();
    });
}

void AreaSelectDialog::capture(const QString &type, const QString &target) {
    m_type = type;
    m_target = target;
    QPixmap shot;
    if (m_type == "screen") {
        // find screen
        m_screen = nullptr;
        for (QScreen *screen: QGuiApplication::screens()) {
            if (screen->name() == target) {
                m_screen = screen;
                break;
            }
        }
        if (!m_screen) return;
        // screenshot
        shot = m_screen->grabWindow(0);
    } else {
        // find camera
        m_camera = QCameraDevice();
        for (const QCameraDevice &camera: QMediaDevices::videoInputs()) {
            if (camera.description() == target) {
                m_camera = camera;
                break;
            }
        }
        if (m_camera.isNull()) return;
        // take picture
        const auto camera = new QCamera(m_camera, this);
        QMediaCaptureSession captureSession;
        captureSession.setCamera(camera);
        QImageCapture imageCapture;
        captureSession.setImageCapture(&imageCapture);
        QEventLoop loop;
        connect(&imageCapture, &QImageCapture::imageCaptured, this, [&](int id, const QImage &img) {
            Q_UNUSED(id);
            shot = QPixmap::fromImage(img);
            loop.quit();
        });
        camera->start();
        imageCapture.capture();
        loop.exec();
        camera->stop();
        delete camera;
    }
    // show in graphics view (native pixel size, no smoothing)
    auto *scene = new QGraphicsScene(m_graphicsView);
    auto *item = scene->addPixmap(shot);
    item->setTransformationMode(Qt::FastTransformation);
    m_graphicsView->setRenderHint(QPainter::SmoothPixmapTransform, false);
    m_graphicsView->setScene(scene);
    m_graphicsView->resetTransform();
    m_graphicsView->setSceneRect(shot.rect());
    m_graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    m_graphicsView->setAlignment(Qt::AlignCenter);
}

QJsonArray AreaSelectDialog::save() {
    return m_area;
}

void AreaSelectDialog::crop() {
    m_graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    connect(m_graphicsView, &QGraphicsView::rubberBandChanged, this, &AreaSelectDialog::getCropArea, Qt::UniqueConnection);
}

void AreaSelectDialog::getCropArea(const QRect &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint) {
    const bool rubberBandEnded = viewportRect.isNull();
    QRect sceneRect = QRectF(fromScenePoint, toScenePoint).normalized().toRect();
    if (!sceneRect.isNull() && sceneRect.isValid()) {
        m_rect = sceneRect;
    }
    if (rubberBandEnded) {
        if (m_type == "screen") {
            const qreal dpr = m_screen->devicePixelRatio();
            m_area = {
                static_cast<int>(m_rect.x() * dpr),
                static_cast<int>(m_rect.y() * dpr),
                static_cast<int>(m_rect.width() * dpr),
                static_cast<int>(m_rect.height() * dpr)
            };
        } else {
            m_area = {
                m_rect.x(),
                m_rect.y(),
                m_rect.width(),
                m_rect.height()
            };
        }
    }
}

PageWidget::PageWidget(QObject *parent) {
}

void PageWidget::init(const QJsonObject &portConfig) {
    QString timestamp;
    auto *pageLayout = new QVBoxLayout(this); // NOLINT
    const QString portType = portConfig["portType"].toString();
    QString portName = portConfig["portName"].toString();
    if (portType == "serial port") {
        // ui init
        m_pushButton = new QPushButton("open"); // NOLINT
        m_pushButton->setCheckable(true);
        pageLayout->addWidget(m_pushButton);
        connect(m_pushButton, &QPushButton::clicked, this, &PageWidget::portToggle);
        //  port init
        m_port = new SerialPort(portConfig, this);
        connect(m_port, &BasePort::appendLog, this, &PageWidget::appendLog);
        // logging
        timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, "serial port", portName, "loaded");
    } else if (portType == "tcp client") {
        // ui init
        m_pushButton = new QPushButton("open"); // NOLINT
        m_pushButton->setCheckable(true);
        pageLayout->addWidget(m_pushButton);
        connect(m_pushButton, &QPushButton::clicked, this, &PageWidget::portToggle);
        //  port init
        m_port = new TcpClient(portConfig, this);
        connect(m_port, &BasePort::appendLog, this, &PageWidget::appendLog);
        // logging
        timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, "tcp client", portName, "loaded");
    } else if (portType == "tcp server") {
        // ui init
        m_pushButton = new QPushButton("open"); // NOLINT
        m_pushButton->setCheckable(true);
        pageLayout->addWidget(m_pushButton);
        connect(m_pushButton, &QPushButton::clicked, this, &PageWidget::portToggle);
        //  port init
        m_port = new TcpServer(portConfig, this);
        connect(m_port, &BasePort::appendLog, this, &PageWidget::appendLog);
        // logging
        timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, "tcp server", portName, "loaded");
    } else if (portType == "udp socket") {
    } else if (portType == "screen") {
        // ui init
        m_pushButton = new QPushButton("open"); // NOLINT
        m_pushButton->setCheckable(true);
        pageLayout->addWidget(m_pushButton);
        connect(m_pushButton, &QPushButton::clicked, this, &PageWidget::portToggle);
        //  port init
        m_port = new Screen(portConfig, this);
        // connect(serialPort, &SerialPort::appendLog, this, &Port::appendLog);
        // logging
        timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, "screen", portName, "loaded");
    } else /* portType == "camera" */
    {
        // ui init
        m_pushButton = new QPushButton("open"); // NOLINT
        m_pushButton->setCheckable(true);
        pageLayout->addWidget(m_pushButton);
        connect(m_pushButton, &QPushButton::clicked, this, &PageWidget::portToggle);
        //  port init
        m_port = new Camera(portConfig, this);
        // connect(serialPort, &SerialPort::appendLog, this, &Port::appendLog);
        // logging
        timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, "camera", portName, "loaded");
    }
}

void PageWidget::portReload(const QJsonObject &portConfig) const {
    m_port->close();
    m_pushButton->setChecked(false);
    m_port->reload(portConfig);
}

QString PageWidget::portInfo() const {
    return m_port->info();
}

void PageWidget::portOpen() const {
    if (m_port->open()) {
        m_pushButton->setChecked(true);
    } else {
        m_pushButton->setChecked(false);
    }
}

void PageWidget::portClose() const {
    m_port->close();
    m_pushButton->setChecked(false);
}

void PageWidget::portWrite(const QString &command, const QString &peerIp) const {
    m_port->write(command, peerIp);
}

QString PageWidget::portRead() const {
    return m_port->read();
}

void PageWidget::portToggle(const bool status) {
    if (status) {
        if (!m_port->open()) {
            m_pushButton->setChecked(false);
        }
    } else
        m_port->close();
}

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
    m_txInterval = portConfig["txInterval"].toInt();
    m_rxFormat = portConfig["rxFormat"].toString();
    m_rxTimeout = portConfig["rxTimeout"].toInt();
    // connect slot
    connect(m_serialPort, &QSerialPort::readyRead, this, [this]() {
        QTimer::singleShot(m_rxTimeout, this, &SerialPort::handleRead);
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
    m_txInterval = portConfig["txInterval"].toInt();
    // rx config
    m_rxFormat = portConfig["rxFormat"].toString();
    m_rxTimeout = portConfig["rxTimeout"].toInt();
    m_rxForward = portConfig["rxForward"].toString();
    // connect slot
    disconnect(m_serialPort, &QSerialPort::readyRead, this, nullptr);
    connect(m_serialPort, &QSerialPort::readyRead, this, [this]() {
        QTimer::singleShot(m_rxTimeout, this, &SerialPort::handleRead);
    });
}

QString SerialPort::info() {
    QString status;
    if (m_serialPort->isOpen())
        status = "opened";
    else
        status = "closed";
    QString portName = m_portName;
    QString baudRate = QString::number(m_baudRate);
    QString dataBits = QString::number(m_dataBits);
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
    return QString("(%1) %2 baudrate: %3, databits: %4, parity: %5, stopbits: %6").arg(status, portName, baudRate, dataBits, parity, stopBits);
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

void SerialPort::write(const QString &command, const QString &peerIp) {
    // check serial port status
    if (!m_serialPort->isOpen()) {
        emit appendLog(QString("%1 %2 %3").arg("serial port", m_portName, "is not opened"), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, "serial port", m_portName, "is not opened");
        return;
    }
    // remove space
    QString f_command = command;
    if (m_txFormat == "hex")
        f_command.remove(" ");
    // suffix attach
    QString suffix;
    if (m_txSuffix == "crlf")
        suffix = "\r\n";
    else if (m_txSuffix == "crc8 maxim")
        suffix = crc8Maxim(command);
    else if (m_txSuffix == "crc16 modbus")
        suffix = crc16Modbus(command);
    else /* m_txSuffix == "null" */
        suffix = "";
    const QString j_command = command + suffix;
    // command reformat
    QByteArray data;
    if (m_txFormat == "hex") {
        data = QByteArray::fromHex(j_command.toUtf8());
    } else if (m_txFormat == "ascii")
        data = j_command.toLatin1();
    else // txFormat == "utf-8"
        data = j_command.toUtf8();
    m_txQueue.append(data);
    if (!m_txBlock) {
        m_txBlock = true;
        handleWrite();
    }
}

void SerialPort::handleWrite() {
    QByteArray data;
    if (!m_txQueue.isEmpty()) {
        data = m_txQueue.takeFirst();
        m_serialPort->write(data);
        QTimer::singleShot(m_txInterval, this, &SerialPort::handleWrite);
    } else {
        m_txBlock = false;
        return;
    }
    QString message;
    if (m_txFormat == "hex") {
        message = data.toHex(' ').toUpper();
    } else if (m_txFormat == "ascii") {
        message = QString::fromLatin1(data);
    } else /* m_txFormat == "utf-8" */ {
        message = QString::fromUtf8(data);
    }
    message = QString("[%1] -&gt; %2").arg(m_serialPort->portName(), message);
    emit appendLog(message, "tx");
}

QString SerialPort::read() {
    return m_rxBuffer;
}

void SerialPort::handleRead() {
    if (const QByteArray data = m_serialPort->readAll(); !data.isEmpty()) {
        QString message;
        if (m_rxFormat == "hex") {
            message = data.toHex(' ').toUpper();
            m_rxBuffer = data.toHex().toUpper();
        } else if (m_rxFormat == "ascii") {
            message = QString::fromLatin1(data);
            m_rxBuffer = message;
        } else /* m_rxFormat == "utf-8" */ {
            message = QString::fromUtf8(data);
            m_rxBuffer = message;
        }
        message = QString("[%1] &lt;- %2").arg(m_serialPort->portName(), message);
        emit appendLog(message, "rx");
    }
}

void SerialPort::handleError() {
}

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
    m_txInterval = portConfig["txInterval"].toInt();
    m_rxFormat = portConfig["rxFormat"].toString();
    m_rxTimeout = portConfig["rxTimeout"].toInt();
    // connect slot
    connect(m_tcpClient, &QTcpSocket::connected, this, &TcpClient::handleConnected);
    connect(m_tcpClient, &QTcpSocket::disconnected, this, &TcpClient::handleDisconnected);
    connect(m_tcpClient, &QTcpSocket::readyRead, this, [this]() {
        QTimer::singleShot(m_rxTimeout, this, &TcpClient::handleRead);
    });
    connect(m_tcpClient, &QTcpSocket::errorOccurred, this, &TcpClient::handleError);
}

void TcpClient::reload(const QJsonObject &portConfig) {
    // port config
    m_tcpClientRemoteAddress = portConfig["tcpClientRemoteAddress"].toString();
    m_tcpClientRemotePort = portConfig["tcpClientRemotePort"].toInt();
    // tx config
    m_txFormat = portConfig["txFormat"].toString();
    m_txSuffix = portConfig["txSuffix"].toString();
    m_txInterval = portConfig["txInterval"].toInt();
    // rx config
    m_rxFormat = portConfig["rxFormat"].toString();
    m_rxTimeout = portConfig["rxTimeout"].toInt();
    m_rxForward = portConfig["rxForward"].toString();
    // connect slot
    disconnect(m_tcpClient, &QTcpSocket::readyRead, this, nullptr);
    connect(m_tcpClient, &QTcpSocket::readyRead, this, [this]() {
        QTimer::singleShot(m_rxTimeout, this, &TcpClient::handleRead);
    });
}

QString TcpClient::info() {
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
    QString localAddress = m_tcpClient->localAddress().toString();
    QString localPort = QString::number(m_tcpClient->localPort());
    QString remoteAddress = m_tcpClientRemoteAddress;
    QString remotePort = QString::number(m_tcpClientRemotePort);

    return QString("(%1) local ip: %2:%3, remote ip: %4:%5").arg(status, localAddress, localPort, remoteAddress, remotePort);
}

bool TcpClient::open() {
    m_tcpClient->connectToHost(m_tcpClientRemoteAddress, m_tcpClientRemotePort);
    return true;
}

void TcpClient::close() {
    m_tcpClient->disconnectFromHost();
}

void TcpClient::write(const QString &command, const QString &peerIp) {
    // check serial port status
    if (!m_tcpClient->isOpen()) {
        emit appendLog("tcp client is not opened", "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "tcp client is not opened");
        return;
    }
    // remove space
    QString f_command = command;
    if (m_txFormat == "hex")
        f_command.remove(" ");
    // suffix attach
    QString suffix;
    if (m_txSuffix == "crlf")
        suffix = "\r\n";
    else if (m_txSuffix == "crc8 maxim")
        suffix = crc8Maxim(command);
    else if (m_txSuffix == "crc16 modbus")
        suffix = crc16Modbus(command);
    else /* m_txSuffix == "null" */
        suffix = "";
    const QString j_command = command + suffix;
    // command reformat
    QByteArray data;
    if (m_txFormat == "hex") {
        data = QByteArray::fromHex(j_command.toUtf8());
    } else if (m_txFormat == "ascii")
        data = j_command.toLatin1();
    else // txFormat == "utf-8"
        data = j_command.toUtf8();
    m_txQueue.append(data);
    if (!m_txBlock) {
        m_txBlock = true;
        handleWrite();
    }
}

QString TcpClient::read() {
    return m_rxBuffer;
}

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

void TcpClient::handleWrite() {
    QByteArray data;
    if (!m_txQueue.isEmpty()) {
        data = m_txQueue.takeFirst();
        m_tcpClient->write(data);
        QTimer::singleShot(m_txInterval, this, &TcpClient::handleWrite);
    } else {
        m_txBlock = false;
        return;
    }
    QString message;
    if (m_txFormat == "hex") {
        message = data.toHex(' ').toUpper();
    } else if (m_txFormat == "ascii") {
        message = QString::fromLatin1(data);
    } else /* m_txFormat == "utf-8" */ {
        message = QString::fromUtf8(data);
    }
    message = QString("[%1:%2 -&gt; %3:%4] %5").arg(m_tcpClientLocalAddress, QString::number(m_tcpClientLocalPort), m_tcpClientRemoteAddress,
                                                    QString::number(m_tcpClientRemotePort), message);
    emit appendLog(message, "tx");
}

void TcpClient::handleRead() {
    if (const QByteArray data = m_tcpClient->readAll(); !data.isEmpty()) {
        QString message;
        if (m_rxFormat == "hex") {
            message = data.toHex(' ').toUpper();
            m_rxBuffer = data.toHex().toUpper();
        } else if (m_rxFormat == "ascii") {
            message = QString::fromLatin1(data);
            m_rxBuffer = message;
        } else /* m_rxFormat == "utf-8" */ {
            message = QString::fromUtf8(data);
            m_rxBuffer = message;
        }
        message = QString("[%1:%2 &lt;- %3:%4] %5").arg(m_tcpClientLocalAddress, QString::number(m_tcpClientLocalPort), m_tcpClientRemoteAddress,
                                                        QString::number(m_tcpClientRemotePort), message);
        emit appendLog(message, "rx");
    }
}

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
    m_txInterval = portConfig["txInterval"].toInt();
    m_rxFormat = portConfig["rxFormat"].toString();
    m_rxTimeout = portConfig["rxTimeout"].toInt();
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
    m_txInterval = portConfig["txInterval"].toInt();
    // rx config
    m_rxFormat = portConfig["rxFormat"].toString();
    m_rxTimeout = portConfig["rxTimeout"].toInt();
    m_rxForward = portConfig["rxForward"].toString();
}

QString TcpServer::info() {
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

    return message;
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

void TcpServer::write(const QString &command, const QString &peerIp) {
    // check serial port status
    if (!m_tcpServer->isListening()) {
        emit appendLog("tcp server is not opened", "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "tcp server is not opened");
        return;
    }
    // remove space
    QString f_command = command;
    if (m_txFormat == "hex")
        f_command.remove(" ");
    // suffix attach
    QString suffix;
    if (m_txSuffix == "crlf")
        suffix = "\r\n";
    else if (m_txSuffix == "crc8 maxim")
        suffix = crc8Maxim(command);
    else if (m_txSuffix == "crc16 modbus")
        suffix = crc16Modbus(command);
    else /* m_txSuffix == "null" */
        suffix = "";
    const QString j_command = command + suffix;
    // command reformat
    QByteArray data;
    if (m_txFormat == "hex") {
        data = QByteArray::fromHex(j_command.toUtf8());
    } else if (m_txFormat == "ascii")
        data = j_command.toLatin1();
    else // txFormat == "utf-8"
        data = j_command.toUtf8();
    m_txQueue.append(data);
    if (!m_txBlock) {
        m_txBlock = true;
        handleWrite(peerIp);
    }
}

QString TcpServer::read() {
    return m_rxBuffer;
};

void TcpServer::handleNewConnection() {
    while (m_tcpServer->hasPendingConnections()) {
        QTcpSocket *tcpServerPeer = m_tcpServer->nextPendingConnection();
        handleConnected(tcpServerPeer);
        connect(tcpServerPeer, &QTcpSocket::readyRead, this, [this, tcpServerPeer]() {
            QTimer::singleShot(m_rxTimeout, this, [this, tcpServerPeer] {
                handleRead(tcpServerPeer);
            });
        });
        connect(tcpServerPeer, &QTcpSocket::disconnected, this, [this, tcpServerPeer]() {
            handleDisconnected(tcpServerPeer);
        });
        connect(tcpServerPeer, &QTcpSocket::errorOccurred, this, [this, tcpServerPeer](QAbstractSocket::SocketError error) {
            handleError(tcpServerPeer);
        });
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

void TcpServer::handleWrite(const QString &peerIp) {
    if (peerIp == "") {
        QByteArray data;
        if (!m_txQueue.isEmpty()) {
            data = m_txQueue.takeFirst();
            foreach(QTcpSocket* tcpServerPeer, m_tcpServerPeerList) {
                tcpServerPeer->write(data);
            }
            QTimer::singleShot(m_txInterval, this, [this,peerIp] {
                handleWrite(peerIp);
            });
        } else {
            m_txBlock = false;
            return;
        }
        QString message;
        if (m_txFormat == "hex") {
            message = data.toHex(' ').toUpper();
        } else if (m_txFormat == "ascii") {
            message = QString::fromLatin1(data);
        } else /* m_txFormat == "utf-8" */ {
            message = QString::fromUtf8(data);
        }
        message = QString("[%1:%2 -&gt; %3] %4").arg(m_tcpServerLocalAddress, QString::number(m_tcpServerLocalPort), "broadcast", message);
        emit appendLog(message, "tx");
    } else {
        QTcpSocket *tcpServerPeer = nullptr;
        foreach(QTcpSocket* peer, m_tcpServerPeerList) {
            if (peerIp == QString("%1:%2").arg(peer->peerAddress().toString(), QString::number(peer->peerPort())))
                tcpServerPeer = peer;
        }
        QByteArray data;
        if (!m_txQueue.isEmpty()) {
            data = m_txQueue.takeFirst();
            if (tcpServerPeer == nullptr) {
                emit appendLog("peer not found", "error");
                // logging
                QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
                qDebug() << QString("[%1] %2").arg(timestamp, "peer not found");
                m_txBlock = false;
                return;
            }
            tcpServerPeer->write(data);
            QTimer::singleShot(m_txInterval, this, [this,peerIp] {
                handleWrite(peerIp);
            });
        } else {
            m_txBlock = false;
            return;
        }
        QString message;
        if (m_txFormat == "hex") {
            message = data.toHex(' ').toUpper();
        } else if (m_txFormat == "ascii") {
            message = QString::fromLatin1(data);
        } else /* m_txFormat == "utf-8" */ {
            message = QString::fromUtf8(data);
        }
        QString peerAddress = tcpServerPeer->peerAddress().toString();
        QString peerPort = QString::number(tcpServerPeer->peerPort());
        message = QString("[%1:%2 -&gt; %3:%4] %5").arg(m_tcpServerLocalAddress, QString::number(m_tcpServerLocalPort), peerAddress, peerPort, message);
        emit appendLog(message, "tx");
    }
}

void TcpServer::handleRead(QTcpSocket *tcpServerPeer) {
    QString peerAddress = tcpServerPeer->peerAddress().toString();
    QString peerPort = QString::number(tcpServerPeer->peerPort());
    if (const QByteArray data = tcpServerPeer->readAll(); !data.isEmpty()) {
        QString message;
        if (m_rxFormat == "hex") {
            message = data.toHex(' ').toUpper();
            m_rxBuffer = data.toHex().toUpper();
        } else if (m_rxFormat == "ascii") {
            message = QString::fromLatin1(data);
            m_rxBuffer = message;
        } else /* m_rxFormat == "utf-8" */ {
            message = QString::fromUtf8(data);
            m_rxBuffer = message;
        }
        message = QString("[%1:%2 &lt;- %3:%4] %5").arg(m_tcpServerLocalAddress, QString::number(m_tcpServerLocalPort), peerAddress,
                                                        peerPort, message);
        emit appendLog(message, "rx");
    }
}

Screen::Screen(const QJsonObject &portConfig, QObject *parent) : BasePort(parent) {
    // port config
    m_portName = portConfig["portName"].toString();
    m_area = QRect(portConfig["area"][0].toInt(), portConfig["area"][1].toInt(), portConfig["area"][2].toInt(), portConfig["area"][3].toInt());
    auto *layout = new QVBoxLayout(m_previewDialog);
    layout->addWidget(m_previewLabel);
}

void Screen::reload(const QJsonObject &portConfig) {
    // port config
    m_portName = portConfig["portName"].toString();
    m_area = QRect(portConfig["area"][0].toInt(), portConfig["area"][1].toInt(), portConfig["area"][2].toInt(), portConfig["area"][3].toInt());
}

bool Screen::open() {
    m_previewDialog->show();
    return true;
}

void Screen::close() {
    m_previewDialog->hide();
}

QString Screen::info() {
    return "";
}

void Screen::write(const QString &command, const QString &peerIp) {
}

QString Screen::read() {
    // find screen
    for (QScreen *screen: QGuiApplication::screens()) {
        if (screen->name() == m_portName) {
            m_screen = screen;
            break;
        }
    }
    if (!m_screen)
        return "screen not found";
    // screenshot and crop
    const QPixmap shot = m_screen->grabWindow(0).copy(m_area);

    if (m_previewDialog->isVisible())
        m_previewLabel->setPixmap(shot);

    QImage image = shot.toImage().convertToFormat(QImage::Format_RGB888);

    // init ocr engine
    auto *ocr = new tesseract::TessBaseAPI();
    ocr->Init(nullptr, "eng");
    // load pic
    ocr->SetImage(
        image.bits(),
        image.width(),
        image.height(),
        3,
        image.bytesPerLine()
    );

    // exec ocr
    char *outText = ocr->GetUTF8Text();
    QString recognizedText = QString::fromUtf8(outText);

    // free resources
    delete[] outText;
    ocr->End();
    delete ocr;
    //
    recognizedText = recognizedText.trimmed().replace("\n", "<br>");;
    return recognizedText.isEmpty() ? "null" : recognizedText;
}

Camera::Camera(const QJsonObject &portConfig, QObject *parent) : BasePort(parent) {
    // port config
    m_portName = portConfig["portName"].toString();
    m_area = QRect(portConfig["area"][0].toInt(), portConfig["area"][1].toInt(), portConfig["area"][2].toInt(), portConfig["area"][3].toInt());
}

void Camera::reload(const QJsonObject &portConfig) {
    // port config
    m_portName = portConfig["portName"].toString();
    m_area = QRect(portConfig["area"][0].toInt(), portConfig["area"][1].toInt(), portConfig["area"][2].toInt(), portConfig["area"][3].toInt());
}

bool Camera::open() {
    qDebug() << m_area;
    return true;
}

void Camera::close() {
}

QString Camera::info() {
    return "";
}

void Camera::write(const QString &command, const QString &peerIp) {
}

QString Camera::read() {
    QPixmap shot;
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
    const auto camera = new QCamera(m_camera, this);
    QMediaCaptureSession captureSession;
    captureSession.setCamera(camera);
    QImageCapture imageCapture;
    captureSession.setImageCapture(&imageCapture);
    QEventLoop loop;
    connect(&imageCapture, &QImageCapture::imageCaptured, this, [&](int id, const QImage &img) {
        Q_UNUSED(id);
        shot = QPixmap::fromImage(img).copy(m_area);
        loop.quit();
    });
    camera->start();
    imageCapture.capture();
    loop.exec();
    camera->stop();
    delete camera;

    // auto *tmp = new QDialog();
    // auto *layout = new QVBoxLayout(tmp);
    // auto *label = new QLabel();
    // layout->addWidget(label);
    // label->setPixmap(shot);
    // tmp->show();

    QImage image = shot.toImage().convertToFormat(QImage::Format_RGB888);

    // init ocr engine
    auto *ocr = new tesseract::TessBaseAPI();
    ocr->Init(nullptr, "eng");
    // load pic
    ocr->SetImage(
        image.bits(),
        image.width(),
        image.height(),
        3,
        image.bytesPerLine()
    );

    // exec ocr
    char *outText = ocr->GetUTF8Text();
    QString recognizedText = QString::fromUtf8(outText);

    // free resources
    delete[] outText;
    ocr->End();
    delete ocr;
    //
    recognizedText = recognizedText.trimmed().replace("\n", "<br>");;
    return recognizedText.isEmpty() ? "null" : recognizedText;
}
