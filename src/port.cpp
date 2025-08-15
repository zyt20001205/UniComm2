#include "../include/port.h"

Port::Port(QObject *parent)
    : QDockWidget("port", qobject_cast<QWidget *>(parent)) {
    uiInit();
}

void Port::uiInit() {
    m_tabWidget = new QTabWidget();
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &Port::portSwitch);
    m_tabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tabWidget->tabBar(), &QTabBar::customContextMenuRequested, this, [this](const QPoint &pos) {
        const int index = m_tabWidget->tabBar()->tabAt(pos);
        portMenu(index, pos);
    });
    setWidget(m_tabWidget);
    m_addButton = new QPushButton(m_tabWidget);
    m_addButton->setIcon(QIcon(":/icon/add.svg"));
    m_tabWidget->setCornerWidget(m_addButton, Qt::TopRightCorner);
    connect(m_addButton, &QPushButton::clicked, this, [this]() {
        portSetting(-1);
    });
    // init port tab
    const auto portCount = m_portConfig.size();
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, QString::number(portCount), "port config found");
    for (int i = 0; i < portCount; ++i) {
        QJsonObject portInfo = m_portConfig[i].toObject();
        QString portType = portInfo["portType"].toString();
        QString portName = portInfo["portName"].toString();
        auto *pageWidget = new QWidget(m_tabWidget);
        auto *pageLayout = new QVBoxLayout(pageWidget);
        if (portType == "serial port") {
            // ui init
            auto *button = new QPushButton("open", pageWidget);
            button->setCheckable(true);
            connect(button, &QPushButton::toggled, this, &Port::portToggle);
            pageLayout->addWidget(button);
            //  port init
            auto *serialPort = new SerialPort(portInfo, this);
            connect(serialPort, &SerialPort::appendLog, this, &Port::appendLog);
            m_portList.append(serialPort);
            // logging
            timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, "serial port", portName, "loaded");
        } else if (portType == "tcp client") {
        } else if (portType == "tcp server") {
        } else if (portType == "udp socket") {
        } else /* portType == "camera" */ {
            // ui init
            // auto *button = new QPushButton("open", pageWidget);
            // button->setCheckable(true);
            // connect(button, &QPushButton::toggled, this, &Port::portToggle);
            // pageLayout->addWidget(button);
            //  port init
            auto *camera = new Camera(portInfo, this);
            // connect(serialPort, &SerialPort::appendLog, this, &Port::appendLog);
            m_portList.append(camera);
            // logging
            timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, "camera", portName, "loaded");
        }
        m_tabWidget->addTab(pageWidget, portName);
    }
    // init setting dialog
    m_settingDialog = new QDialog(m_tabWidget);
    m_settingDialog->setFixedWidth(600);
    m_settingLayout = new QVBoxLayout(m_settingDialog);

    m_portTypeWidget = new QWidget(m_settingDialog);
    m_settingLayout->addWidget(m_portTypeWidget);
    const auto portTypeLayout = new QHBoxLayout(m_portTypeWidget);
    portTypeLayout->setContentsMargins(0, 0, 0, 0);
    m_portTypeLabel = new QLabel("port type");
    portTypeLayout->addWidget(m_portTypeLabel);
    m_portTypeCombobox = new QComboBox();
    portTypeLayout->addWidget(m_portTypeCombobox);
    m_portTypeCombobox->addItems(QStringList{"choose port type", "serial port", "tcp client", "tcp server", "camera"});
    connect(m_portTypeCombobox, &QComboBox::currentIndexChanged, this, &Port::portTypeSwitch);

    // serial port settings
    m_serialPortNameWidget = new QWidget(m_settingDialog);
    m_settingLayout->addWidget(m_serialPortNameWidget);
    const auto serialPortNameLayout = new QHBoxLayout(m_serialPortNameWidget);
    serialPortNameLayout->setContentsMargins(0, 0, 0, 0);
    m_serialPortNameLabel = new QLabel("port name");
    serialPortNameLayout->addWidget(m_serialPortNameLabel);
    m_serialPortNameCombobox = new QComboBox();
    serialPortNameLayout->addWidget(m_serialPortNameCombobox);
    for (QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts(); const QSerialPortInfo &port: ports) {
        m_serialPortNameCombobox->addItem(port.portName() + " " + port.description(), port.portName());
    }

    m_serialPortBaudRateWidget = new QWidget(m_settingDialog);
    m_settingLayout->addWidget(m_serialPortBaudRateWidget);
    const auto serialPortBaudRateLayout = new QHBoxLayout(m_serialPortBaudRateWidget);
    serialPortBaudRateLayout->setContentsMargins(0, 0, 0, 0);
    m_serialPortBaudRateLabel = new QLabel("baud rate");
    serialPortBaudRateLayout->addWidget(m_serialPortBaudRateLabel);
    m_serialPortBaudRateSpinBox = new QSpinBox();
    serialPortBaudRateLayout->addWidget(m_serialPortBaudRateSpinBox);
    m_serialPortBaudRateSpinBox->setRange(1, 5000000);
    m_serialPortBaudRateSpinBox->setValue(115200);

    m_serialPortDataBitsWidget = new QWidget(m_settingDialog);
    m_settingLayout->addWidget(m_serialPortDataBitsWidget);
    const auto serialPortDataBitsLayout = new QHBoxLayout(m_serialPortDataBitsWidget);
    serialPortDataBitsLayout->setContentsMargins(0, 0, 0, 0);
    m_serialPortDataBitsLabel = new QLabel("databits");
    serialPortDataBitsLayout->addWidget(m_serialPortDataBitsLabel);
    m_serialPortDataBitsCombobox = new QComboBox();
    serialPortDataBitsLayout->addWidget(m_serialPortDataBitsCombobox);
    m_serialPortDataBitsCombobox->addItem("5", 5);
    m_serialPortDataBitsCombobox->addItem("6", 6);
    m_serialPortDataBitsCombobox->addItem("7", 7);
    m_serialPortDataBitsCombobox->addItem("8", 8);
    m_serialPortDataBitsCombobox->setCurrentText("8");

    m_serialPortParityWidget = new QWidget(m_settingDialog);
    m_settingLayout->addWidget(m_serialPortParityWidget);
    const auto serialPortParityLayout = new QHBoxLayout(m_serialPortParityWidget);
    serialPortParityLayout->setContentsMargins(0, 0, 0, 0);
    m_serialPortParityLabel = new QLabel("parity");
    serialPortParityLayout->addWidget(m_serialPortParityLabel);
    m_serialPortParityCombobox = new QComboBox();
    serialPortParityLayout->addWidget(m_serialPortParityCombobox);
    m_serialPortParityCombobox->addItem("no", 0);
    m_serialPortParityCombobox->addItem("even", 2);
    m_serialPortParityCombobox->addItem("odd", 3);
    m_serialPortParityCombobox->addItem("space", 4);
    m_serialPortParityCombobox->addItem("mark", 5);
    m_serialPortParityCombobox->setCurrentText("no");

    m_serialPortStopBitsWidget = new QWidget(m_settingDialog);
    m_settingLayout->addWidget(m_serialPortStopBitsWidget);
    const auto serialPortStopBitsLayout = new QHBoxLayout(m_serialPortStopBitsWidget);
    serialPortStopBitsLayout->setContentsMargins(0, 0, 0, 0);
    m_serialPortStopBitsLabel = new QLabel("stop bits");
    serialPortStopBitsLayout->addWidget(m_serialPortStopBitsLabel);
    m_serialPortStopBitsCombobox = new QComboBox();
    serialPortStopBitsLayout->addWidget(m_serialPortStopBitsCombobox);
    m_serialPortStopBitsCombobox->addItem("1", 1);
    m_serialPortStopBitsCombobox->addItem("1.5", 3);
    m_serialPortStopBitsCombobox->addItem("2", 2);
    m_serialPortStopBitsCombobox->setCurrentText("1");

    // camera
    m_cameraNameWidget = new QWidget(m_settingDialog);
    m_settingLayout->addWidget(m_cameraNameWidget);
    const auto cameraLayout = new QHBoxLayout(m_cameraNameWidget);
    cameraLayout->setContentsMargins(0, 0, 0, 0);
    m_cameraNameLabel = new QLabel("camera name");
    cameraLayout->addWidget(m_cameraNameLabel);
    m_cameraNameCombobox = new QComboBox();
    cameraLayout->addWidget(m_cameraNameCombobox);
    for (const QList<QScreen *> screens = QGuiApplication::screens(); const QScreen *screen: screens) {
        qDebug() << screen->name();
        m_cameraNameCombobox->addItem(screen->name() + " " + QString::number(screen->size().width()) + "x" + QString::number(screen->size().height()), screen->name());
    }

    m_cameraAreaWidget = new QWidget(m_settingDialog);
    m_settingLayout->addWidget(m_cameraAreaWidget);
    const auto cameraAreaLayout = new QHBoxLayout(m_cameraAreaWidget);
    cameraAreaLayout->setContentsMargins(0, 0, 0, 0);
    m_cameraAreaLabel = new QLabel("capture area");
    cameraAreaLayout->addWidget(m_cameraAreaLabel);
    m_cameraAreaPushButton = new QPushButton("choose capture area");
    cameraAreaLayout->addWidget(m_cameraAreaPushButton);
    connect(m_cameraAreaPushButton, &QPushButton::clicked, this, [this]() {
    m_cameraAreaChooseDialog->show();
});

    m_cameraAreaChooseDialog = new QDialog(m_settingDialog);
    m_cameraAreaChooseDialog->setFixedSize(1280,720);
    const auto cameraAreaChooseLayout = new QVBoxLayout(m_cameraAreaChooseDialog);
    cameraAreaChooseLayout->setContentsMargins(0, 0, 0, 0);
    m_cameraAreaChooseGraphicsView = new QGraphicsView(m_cameraAreaChooseDialog);
    cameraAreaChooseLayout->addWidget(m_cameraAreaChooseGraphicsView);
    const auto cameraAreaChooseToolBar = new QToolBar(m_settingDialog);
    cameraAreaChooseLayout->addWidget(cameraAreaChooseToolBar);

    // tx/rx
    m_rxTimeoutWidget = new QWidget(m_settingDialog);
    m_settingLayout->addWidget(m_rxTimeoutWidget);
    const auto timeoutLayout = new QHBoxLayout(m_rxTimeoutWidget);
    timeoutLayout->setContentsMargins(0, 0, 0, 0);
    m_rxTimeoutLabel = new QLabel("rx timeout");
    timeoutLayout->addWidget(m_rxTimeoutLabel);
    m_rxTimeoutSpinBox = new QSpinBox();
    timeoutLayout->addWidget(m_rxTimeoutSpinBox);
    m_rxTimeoutSpinBox->setRange(0, 1000);
    m_rxTimeoutSpinBox->setValue(0);
}

void Port::portWrite(const QString &command, const int index) {
    if (index == -1)
        m_portList[m_currentIndex]->write(command);
    else
        m_portList[index]->write(command);
}

QString Port::portRead(const int index) {
    if (index == -1)
        return m_portList[m_currentIndex]->read();
    return m_portList[index]->read();
}

void Port::portToggle(const bool status) {
    if (status)
        m_portList[m_currentIndex]->open();
    else
        m_portList[m_currentIndex]->close();
}

void Port::portMenu(const int index, const QPoint &pos) {
    m_tabWidget->setCurrentIndex(index);
    QMenu menu;
    menu.addAction("edit", [this, index]() {
        portSetting(index);
    });
    menu.addAction("remove", [this, index]() {
        portRemove(index);
        m_tabWidget->removeTab(index);
    });
    menu.exec(m_tabWidget->tabBar()->mapToGlobal(pos));
}

void Port::portSetting(const int index) {
    m_settingDialog->show();
    if (index == -1) {
        m_portTypeCombobox->setCurrentIndex(0);
        portSettingWidgetHide();
    } else {
        QJsonObject portInfo = m_portConfig[index].toObject();
        const QString portType = portInfo["portType"].toString();
        m_portTypeCombobox->setCurrentText(portType);
        if (portType == "serial port") {
            int i = m_serialPortDataBitsCombobox->findData(portInfo["portName"].toString());
            m_serialPortNameCombobox->setCurrentIndex(i);
            m_serialPortBaudRateSpinBox->setValue(portInfo["baudRate"].toInt());
            i = m_serialPortDataBitsCombobox->findData(portInfo["dataBits"].toInt());
            m_serialPortDataBitsCombobox->setCurrentIndex(i);
            i = m_serialPortParityCombobox->findData(portInfo["parity"].toInt());
            m_serialPortParityCombobox->setCurrentIndex(i);
            i = m_serialPortStopBitsCombobox->findData(portInfo["stopBits"].toInt());
            m_serialPortStopBitsCombobox->setCurrentIndex(i);
            m_rxTimeoutSpinBox->setValue(portInfo["rxTimeout"].toInt());
        } else if (portType == "tcp client") {
        } else if (portType == "tcp server") {
        } else if (portType == "udp socket") {
        } else /* portType == "camera" */ {
            int i = m_cameraNameCombobox->findData(portInfo["portName"].toString());
            m_cameraNameCombobox->setCurrentIndex(i);
        }
    }
}

void Port::portSettingWidgetHide() const {
    // serial port
    m_serialPortNameWidget->hide();
    m_serialPortBaudRateWidget->hide();
    m_serialPortDataBitsWidget->hide();
    m_serialPortParityWidget->hide();
    m_serialPortStopBitsWidget->hide();
    // camera
    m_cameraNameWidget->hide();
    // rx/tx
    m_rxTimeoutWidget->hide();
};

void Port::portSwitch(const int index) {
    m_currentIndex = index;
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, "switched to port", QString::number(index));
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

void Port::portTypeSwitch(const int type) {
    qDebug() << type;
    if (type == 0) {
        portSettingWidgetHide();
    } else if (type == 1) {
        portSettingWidgetHide();
        m_serialPortNameWidget->show();
        m_serialPortBaudRateWidget->show();
        m_serialPortDataBitsWidget->show();
        m_serialPortParityWidget->show();
        m_serialPortStopBitsWidget->show();
        m_rxTimeoutWidget->show();
    } else if (type == 2) {
    } else if (type == 3) {
    } else {
        portSettingWidgetHide();
        m_cameraNameWidget->show();
    }
}

SerialPort::SerialPort(const QJsonObject &portConfig, QObject *parent) : BasePort(parent), m_serialPort(new QSerialPort(this)) {
    // port config
    m_portName = portConfig["portName"].toString();
    m_baudRate = portConfig["baudRate"].toInt();
    m_dataBits = portConfig["dataBits"].toInt();
    m_parity = portConfig["parity"].toInt();
    m_stopBits = portConfig["stopBits"].toInt();
    m_timeout = portConfig["rxTimeout"].toInt();
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
    m_rxForward = portConfig["rxForward"].toString();
    // connect slot
    connect(m_serialPort, &QSerialPort::readyRead, this, [this]() {
        QTimer::singleShot(m_timeout, this, &SerialPort::handleRead);
    });
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &SerialPort::handleError);
}

bool SerialPort::open() {
    if (m_serialPort->open(QSerialPort::ReadWrite)) {
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3").arg(timestamp, m_portName, "opened");
        return true;
    }
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3: %4").arg(timestamp, m_portName, "open failed", m_serialPort->errorString());
    return false;
}

void SerialPort::close() {
    m_serialPort->close();
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, m_portName, "closed");
}

void SerialPort::write(const QString &command) {
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
    else // m_txSuffix == none
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

QString SerialPort::read() {
    return m_rxBuffer;
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
        m_txBuffer = data.toHex().toUpper();
    } else if (m_txFormat == "ascii") {
        message = QString::fromLatin1(data);
        m_txBuffer = message;
    } else // m_txFormat == "utf-8"
    {
        message = QString::fromUtf8(data);
        m_txBuffer = message;
    }
    message = QString("[%1] -&gt; %2").arg(m_serialPort->portName(), message);
    emit appendLog(message, "tx");
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
        } else // m_rxFormat == "utf-8"
        {
            message = QString::fromUtf8(data);
            m_rxBuffer = message;
        }
        message = QString("[%1] &lt;- %2").arg(m_serialPort->portName(), message);
        emit appendLog(message, "rx");
    }
}

void SerialPort::handleError() {
}

Camera::Camera(const QJsonObject &portConfig, QObject *parent) : BasePort(parent), m_camera(new QObject(this)) {
    // port config
    m_portName = portConfig["portName"].toString();
}

bool Camera::open() {
}

void Camera::close() {
}

void Camera::write(const QString &command) {
}

QString Camera::read() {
    if (m_portName == "screen") {
        // QScreen *screen = QApplication::primaryScreen();
        // const QPixmap screenshot = screen->grabWindow(0);
        // QImage qimg = screenshot.toImage().convertToFormat(QImage::Format_RGB888);
        //
        // const int width = qimg.width();
        // const int height = qimg.height();
        // PIX *image = pixCreate(width, height, 32);
        //
        // for (int y = 0; y < height; ++y) {
        //     for (int x = 0; x < width; ++x) {
        //         const QRgb pixel = qimg.pixel(x, y);
        //         const l_uint32 pixelData = (0xFF << 24) | (qRed(pixel) << 16) | (qGreen(pixel) << 8) | qBlue(pixel);
        //         pixSetPixel(image, x, y, pixelData);
        //     }
        // }

        // init ocr engine
        auto *ocr = new tesseract::TessBaseAPI();
        ocr->Init(nullptr, "eng");
        // load pic
        const QString imagePath = "test.png";
        PIX *image = pixRead(imagePath.toStdString().c_str());
        ocr->SetImage(image);

        // exec ocr
        char *outText = ocr->GetUTF8Text();
        QString recognizedText = QString::fromUtf8(outText);

        // free resources
        delete[] outText;
        pixDestroy(&image);
        ocr->End();
        delete ocr;
        //
        recognizedText = recognizedText.trimmed().replace("\n", "<br>");;
        return recognizedText.isEmpty() ? "null" : recognizedText;
    }
}
