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






