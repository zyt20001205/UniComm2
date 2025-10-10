#include "portModule/portModule.h"

#include <QContextMenuEvent>
#include <QDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QTabBar>
#include <QTimer>
#include <QVBoxLayout>

#include "globals.h"
#include "utils.h"
#include "portModule/basePort.h"
#include "portModule/camera.h"
#include "portModule/portSetting.h"
#include "portModule/screen.h"
#include "portModule/serialPort.h"
#include "portModule/tcpClient.h"
#include "portModule/tcpServer.h"
#include "portModule/udpSocket.h"

// PortModule public
PortModule::PortModule(QWidget *parent)
    : QDockWidget("port", parent),
      m_portConfig(g_config["portConfig"].toArray()),
      m_portTabWidget(new QTabWidget()),
      m_portTabOverlay(new QWidget(m_portTabWidget)) {
    setWidget(m_portTabWidget);
    m_portTabWidget->setTabsClosable(true);
    m_portTabWidget->setMovable(true);
    // m_portTabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_portTabWidget, &QTabWidget::tabCloseRequested, this, [this](const int index) { portRemove(index); });
    connect(m_portTabWidget->tabBar(), &QTabBar::tabMoved, this, &PortModule::portSwap);
    auto *addButton = new QPushButton(); // NOLINT
    addButton->setIcon(QIcon(":/icon/add.svg"));
    m_portTabWidget->setCornerWidget(addButton, Qt::TopRightCorner);
    connect(addButton, &QPushButton::clicked, this, [this] {
        if (PortSetting portSettingDialog; portSettingDialog.exec() == QDialog::Accepted) {
            const QJsonObject portConfig = portSettingDialog.portSettingExport();
            m_portConfig.append(portConfig);
            portInsert(m_portTabWidget->count(), portConfig);
        }
    });
    // load ports
    int index = 0;
    for (const auto &value: m_portConfig) {
        QJsonObject portConfig = value.toObject();
        portInsert(index, portConfig);
        index++;
    }

    m_portTabOverlay->setStyleSheet("background-color: rgba(0, 0, 0, 96);");
    auto *overlayLayout = new QVBoxLayout(m_portTabOverlay); // NOLINT
    overlayLayout->setAlignment(Qt::AlignCenter);
    overlayLayout->setContentsMargins(0, 0, 0, 0);
    auto *overlayLabel = new QLabel(tr("WIP")); // NOLINT
    overlayLayout->addWidget(overlayLabel);
    overlayLabel->setFont(QFont("Consolas", 12, QFont::Bold));
    overlayLabel->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: white;");
    if (m_portTabWidget->count() == 0) overlayShow();

    QTimer::singleShot(0, this, [this] { overlayResize(); });
}

void PortModule::portConfigSave() const {
    g_config["portConfig"] = m_portConfig;
}

BasePort *PortModule::portObject(const int index) const {
    BasePort *portObject = nullptr;
    if (index == -1) portObject = qobject_cast<PortPage *>(m_portTabWidget->currentWidget())->m_port;
    else portObject = qobject_cast<PortPage *>(m_portTabWidget->widget(index))->m_port;
    return portObject;
}

// PortModule protected
void PortModule::contextMenuEvent(QContextMenuEvent *event) {
    const QPoint globalPos = event->globalPos();
    const auto *tabBar = m_portTabWidget->tabBar();
    const QPoint tabBarPos = tabBar->mapFromGlobal(globalPos);
    if (tabBar->rect().contains(tabBarPos)) {
        const int index = tabBar->tabAt(tabBarPos);
        auto *portPage = qobject_cast<PortPage *>(m_portTabWidget->widget(index));
        m_portTabWidget->setCurrentWidget(portPage);
        QMenu menu(this);
        menu.addAction("edit", [this, index, portPage] {
            PortSetting portSettingDialog;
            QJsonObject portConfig = m_portConfig[index].toObject();
            portSettingDialog.portSettingImport(portConfig);
            if (portSettingDialog.exec() == QDialog::Accepted) {
                portConfig = portSettingDialog.portSettingExport();
                m_portConfig[index] = portConfig;
                portPage->portReload(portConfig);
            }
        });
        menu.addAction("duplicate", [this, index] { portDuplicate(index); });
        menu.exec(event->globalPos());
    }
}

void PortModule::resizeEvent(QResizeEvent *event) {
    QDockWidget::resizeEvent(event);
    if (m_portTabOverlay->isVisible()) overlayResize();
}

// PortModule private
void PortModule::portInsert(const int index, const QJsonObject &portConfig) {
    auto *portPage = new PortPage(portConfig); // NOLINT
    connect(portPage, &PortPage::appendLog, this, &PortModule::appendLog);
    // connect(pageWidget->m_port, &BasePort::showPreview, this, &PortModule::previewShow);
    m_portTabWidget->insertTab(index, portPage, portConfig["portName"].toString());
    overlayHide();
}

void PortModule::portDuplicate(const int index) {
    const QJsonObject portConfig = m_portConfig[index].toObject();
    m_portConfig.insert(index + 1, portConfig);
    portInsert(index + 1, portConfig);
}

void PortModule::portRemove(const int index) {
    QJsonObject portConfig = m_portConfig[index].toObject();
    QString portName = portConfig["portName"].toString();
    const QMessageBox::StandardButton reply =
            QMessageBox::question(
                nullptr,
                tr("Remove Port"),
                QString(tr("Are you sure to remove port %1?")).arg(portName),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);
    if (reply != QMessageBox::Yes) return;
    m_portConfig.removeAt(index);
    QWidget *w = m_portTabWidget->widget(index);
    m_portTabWidget->removeTab(index);
    if (w) w->deleteLater();
    if (m_portTabWidget->count() == 0) overlayShow();
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 removed").arg(timestamp, portName);
}

void PortModule::portSwap(const int srcIndex, const int dstIndex) {
    // config
    const QJsonValue tmp = m_portConfig.takeAt(srcIndex);
    m_portConfig.insert(dstIndex, tmp);
    // qDebug() << m_portConfig;
}

void PortModule::overlayShow() const {
    m_portTabOverlay->raise();
    m_portTabOverlay->show();
}

void PortModule::overlayHide() const {
    m_portTabOverlay->hide();
}

void PortModule::overlayResize() const {
    m_portTabOverlay->setGeometry(m_portTabWidget->rect());
}

// PortPage public
PortPage::PortPage(const QJsonObject &portConfig, QWidget *parent)
    : QWidget(parent),
      m_portToggleButton(new QPushButton(tr("Open"))) {
    auto *pageLayout = new QVBoxLayout(this); // NOLINT
    m_portToggleButton->setCheckable(true);
    pageLayout->addWidget(m_portToggleButton);

    QString timestamp;
    switch (portConfig["portType"].toInt()) {
        case SERIALPORT: {
            m_port = new SerialPort(portConfig);
            connect(m_portToggleButton, &QPushButton::clicked, this, &PortPage::portToggle);
            connect(m_port, &BasePort::appendLog, this, &PortPage::appendLog);
            timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] serial port loaded").arg(timestamp);
            break;
        }
        case TCPCLIENT: {
            m_port = new TcpClient(portConfig);
            connect(m_portToggleButton, &QPushButton::clicked, this, &PortPage::portToggle);
            connect(m_port, &BasePort::appendLog, this, &PortPage::appendLog);
            timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] tcp client loaded").arg(timestamp);
            break;
        }
        case TCPSERVER: {
            m_port = new TcpServer(portConfig);
            connect(m_portToggleButton, &QPushButton::clicked, this, &PortPage::portToggle);
            connect(m_port, &BasePort::appendLog, this, &PortPage::appendLog);
            timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] tcp server loaded").arg(timestamp);
            break;
        }
        case UDPSOCKET: {
            m_port = new UdpSocket(portConfig);
            connect(m_portToggleButton, &QPushButton::clicked, this, &PortPage::portToggle);
            connect(m_port, &BasePort::appendLog, this, &PortPage::appendLog);
            timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] udp socket loaded").arg(timestamp);
            break;
        }
        case SCREEN: {
            m_port = new Screen(portConfig);
            connect(m_portToggleButton, &QPushButton::clicked, this, &PortPage::portToggle);
            // connect(m_port, &BasePort::appendLog, this, &PortPage::appendLog);
            timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] screen loaded").arg(timestamp);
            break;
        }
        case CAMERA: {
            m_port = new Camera(portConfig);
            connect(m_portToggleButton, &QPushButton::clicked, this, &PortPage::portToggle);
            // connect(m_port, &BasePort::appendLog, this, &PortPage::appendLog);
            timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] camera loaded").arg(timestamp);
            break;
        }
        default: {
            qDebug() << "unknown port type";
            break;
        }
    }
}

PortPage::~PortPage() {
    QMetaObject::invokeMethod(m_port, [this] {
        m_port->close();
    }, Qt::BlockingQueuedConnection);
    delete m_port;
}

void PortPage::portReload(const QJsonObject &portConfig) const {
    QMetaObject::invokeMethod(m_port, [this] {
        m_port->close();
    }, Qt::BlockingQueuedConnection);
    m_portToggleButton->setChecked(false);
    QMetaObject::invokeMethod(m_port, [this, portConfig] {
        m_port->reload(portConfig);
    }, Qt::BlockingQueuedConnection);
}

// PortPage private
void PortPage::portToggle(const bool status) const {
    if (status) {
        bool ok = false;
        QMetaObject::invokeMethod(m_port, [&ok, this] {
            ok = m_port->open();
        }, Qt::BlockingQueuedConnection);
        m_portToggleButton->setChecked(ok);
    } else {
        QMetaObject::invokeMethod(m_port, [this] {
            m_port->close();
        }, Qt::BlockingQueuedConnection);
        m_portToggleButton->setChecked(false);
    }
}
