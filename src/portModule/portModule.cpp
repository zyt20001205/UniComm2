#include "portModule/portModule.h"

#include <QContextMenuEvent>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QTabBar>
#include <QTimer>
#include <QVBoxLayout>

#include "globals.h"
#include "portModule/basePort.h"
#include "portModule/camera.h"
#include "portModule/portSetting.h"
#include "portModule/screen.h"
#include "portModule/serialPort.h"
#include "portModule/tcpClient.h"
#include "portModule/tcpServer.h"
#include "portModule/udpSocket.h"

// PortModule public
PortModule::PortModule()
    : DockWidget("port"),
      m_portConfig(g_config["portConfig"].toArray()),
      m_portTabWidget(new QTabWidget()),
      m_portTabOverlay(new QWidget(m_portTabWidget)) {
    setMinimumHeight(100);
    setWidget(m_portTabWidget);
    m_portTabWidget->setTabsClosable(true);
    m_portTabWidget->setMovable(true);
    connect(m_portTabWidget, &QTabWidget::tabCloseRequested, this, [this](const int index) { portRemove(index); });
    connect(m_portTabWidget->tabBar(), &QTabBar::tabMoved, this, &PortModule::portSwap);
    auto *addButton = new QPushButton(); // NOLINT
    addButton->setIcon(QIcon(":/icon/add.svg"));
    m_portTabWidget->setCornerWidget(addButton, Qt::TopRightCorner);
    connect(addButton, &QPushButton::clicked, this, [this] {
        const QSet usedPortName(m_portHash.keyBegin(), m_portHash.keyEnd());
        if (PortSetting portSettingDialog(usedPortName); portSettingDialog.exec() == QDialog::Accepted) {
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

    m_portTabOverlay->installEventFilter(this);
    m_portTabOverlay->setStyleSheet("background-color: rgba(0, 0, 0, 96);");
    auto *overlayLayout = new QVBoxLayout(m_portTabOverlay); // NOLINT
    overlayLayout->setAlignment(Qt::AlignCenter);
    overlayLayout->setContentsMargins(0, 0, 0, 0);
    auto *overlayLabel = new QLabel(tr("Click to add a port")); // NOLINT
    overlayLayout->addWidget(overlayLabel);
    overlayLabel->setFont(QFont("Consolas", 12, QFont::Bold));
    overlayLabel->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: white;");
    if (m_portTabWidget->count() == 0) overlayShow();
}

void PortModule::workspaceOpen(const QUrl &rootUrl) {
    didOpenNotification();
}

void PortModule::portConfigSave() const {
    g_config["portConfig"] = m_portConfig;
}

BasePort *PortModule::currentPort() const {
    if (m_portTabWidget->currentWidget()) {
        return static_cast<PortPage *>(m_portTabWidget->currentWidget())->m_port;
    }
    return nullptr;
}

// PortModule protected
void PortModule::contextMenuEvent(QContextMenuEvent *event) {
    const QPoint globalPos = event->globalPos();
    const auto *tabBar = m_portTabWidget->tabBar();
    const QPoint tabBarPos = tabBar->mapFromGlobal(globalPos);
    if (tabBar->rect().contains(tabBarPos)) {
        const int index = tabBar->tabAt(tabBarPos);
        auto *portPage = static_cast<PortPage *>(m_portTabWidget->widget(index));
        m_portTabWidget->setCurrentWidget(portPage);
        QMenu menu(this);
        menu.addAction("edit", [this, index, portPage] {
            const QSet usedPortName(m_portHash.keyBegin(), m_portHash.keyEnd());
            PortSetting portSettingDialog(usedPortName);
            const QJsonObject oldPortConfig = m_portConfig[index].toObject();
            portSettingDialog.portSettingImport(oldPortConfig);
            if (portSettingDialog.exec() == QDialog::Accepted) {
                const QJsonObject newPortConfig = portSettingDialog.portSettingExport();
                m_portConfig[index] = newPortConfig;
                if (newPortConfig["portName"].toString() != oldPortConfig["portName"].toString()) {
                    m_portTabWidget->setTabText(index, newPortConfig["portName"].toString());

                    BasePort *port = m_portHash.value(oldPortConfig["portName"].toString());
                    m_portHash.remove(oldPortConfig["portName"].toString());
                    m_portHash.insert(newPortConfig["portName"].toString(), port);
                    didChangeNotification();
                    qDebug() << m_portHash;
                }
                portPage->portReload(newPortConfig);
            }
        });
        menu.exec(event->globalPos());
    }
}

bool PortModule::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_portTabOverlay && event->type() == QEvent::MouseButtonPress) {
        const QSet usedPortName(m_portHash.keyBegin(), m_portHash.keyEnd());
        if (PortSetting portSettingDialog(usedPortName); portSettingDialog.exec() == QDialog::Accepted) {
            const QJsonObject portConfig = portSettingDialog.portSettingExport();
            m_portConfig.append(portConfig);
            portInsert(m_portTabWidget->count(), portConfig);
        }
        return true;
    }
    return DockWidget::eventFilter(obj, event);
}

void PortModule::resizeEvent(QResizeEvent *event) {
    DockWidget::resizeEvent(event);
    if (!m_portTabOverlay->isHidden()) overlayResize();
}

// PortModule private
void PortModule::portInsert(const int index, const QJsonObject &portConfig) {
    auto *portPage = new PortPage(portConfig); // NOLINT
    connect(portPage, &PortPage::appendLog, this, &PortModule::appendLog);
    // connect(pageWidget->m_port, &BasePort::showPreview, this, &PortModule::previewShow);
    m_portTabWidget->insertTab(index, portPage, portConfig["portName"].toString());
    m_portHash.insert(portConfig["portName"].toString(), portPage->m_port);
    didChangeNotification();
    qDebug() << m_portHash;
    overlayHide();
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
    m_portHash.remove(portName);
    didChangeNotification();
    qDebug() << m_portHash;
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
    overlayResize();
    m_portTabOverlay->raise();
    m_portTabOverlay->show();
}

void PortModule::overlayHide() const {
    m_portTabOverlay->hide();
}

void PortModule::overlayResize() const {
    m_portTabOverlay->resize(m_portTabWidget->size());
    m_portTabOverlay->move(0, 0);
}

QString PortModule::portAnnotationGet() const {
    QString annotation;

    annotation += "--- @meta\n\n";
    annotation += "--- @alias port\n";
    QStringList portList = m_portHash.keys();
    for (const QString &portName: portList) {
        annotation += QString("--- | '\"%1\"'\n").arg(portName);
    }
    if (portList.isEmpty()) {
        annotation += "--- | string\n";
    }
    annotation += "\n";

    return annotation;
}

void PortModule::didOpenNotification() {
    // didOpen notification to lua language server
    const QJsonObject didOpenParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()},
                {"languageId", "lua"},
                {"version", m_version++},
                {"text", portAnnotationGet()}
            }
        }
    };
    emit notificationJson("textDocument/didOpen", didOpenParams);
}

void PortModule::didChangeNotification() {
    // didChange notification to lua language server
    const QJsonObject didChangeParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()},
                {"version", m_version++}
            }
        },
        {
            "contentChanges", QJsonArray{
                QJsonObject{
                    {"text", portAnnotationGet()}
                }
            }
        }
    };
    emit notificationJson("textDocument/didChange", didChangeParams);
}

// PortPage public
PortPage::PortPage(const QJsonObject &portConfig, QWidget *parent)
    : QWidget(parent),
      m_portToggleButton(new QPushButton(tr("Open"))) {
    auto *layout = new QVBoxLayout(this); // NOLINT
    m_portToggleButton->setCheckable(true);


    QString timestamp;
    switch (portConfig["portType"].toInt()) {
        case SERIALPORT: {
            layout->addWidget(m_portToggleButton);
            connect(m_portToggleButton, &QPushButton::clicked, this, &PortPage::portToggle);

            m_port = new SerialPort(portConfig);
            connect(m_port, &BasePort::appendLog, this, &PortPage::appendLog);
            connect(m_port, &BasePort::togglePort, this, [this](const bool status) {
                m_portToggleButton->setChecked(status);
            });
            timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] serial port loaded").arg(timestamp);
            break;
        }
        case TCPCLIENT: {
            layout->addWidget(m_portToggleButton);
            connect(m_portToggleButton, &QPushButton::clicked, this, &PortPage::portToggle);

            m_port = new TcpClient(portConfig);
            connect(m_port, &BasePort::appendLog, this, &PortPage::appendLog);
            connect(m_port, &BasePort::togglePort, this, [this](const bool status) {
                m_portToggleButton->setChecked(status);
            });
            timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] tcp client loaded").arg(timestamp);
            break;
        }
        case TCPSERVER: {
            layout->addWidget(m_portToggleButton);
            connect(m_portToggleButton, &QPushButton::clicked, this, &PortPage::portToggle);

            m_port = new TcpServer(portConfig);
            connect(m_port, &BasePort::appendLog, this, &PortPage::appendLog);
            connect(m_port, &BasePort::togglePort, this, [this](const bool status) {
                m_portToggleButton->setChecked(status);
            });
            timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] tcp server loaded").arg(timestamp);
            break;
        }
        case UDPSOCKET: {
            layout->addWidget(m_portToggleButton);
            connect(m_portToggleButton, &QPushButton::clicked, this, &PortPage::portToggle);

            m_port = new UdpSocket(portConfig);
            connect(m_port, &BasePort::appendLog, this, &PortPage::appendLog);
            connect(m_port, &BasePort::togglePort, this, [this](const bool status) {
                m_portToggleButton->setChecked(status);
            });
            timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] udp socket loaded").arg(timestamp);
            break;
        }
        case SCREEN: {
            layout->addWidget(m_portToggleButton);
            connect(m_portToggleButton, &QPushButton::clicked, this, &PortPage::portToggle);

            m_port = new Screen(portConfig);
            // connect(m_port, &BasePort::appendLog, this, &PortPage::appendLog);
            connect(m_port, &BasePort::togglePort, this, [this](const bool status) {
                m_portToggleButton->setChecked(status);
            });
            timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] screen loaded").arg(timestamp);
            break;
        }
        case CAMERA: {
            layout->addWidget(m_portToggleButton);
            connect(m_portToggleButton, &QPushButton::clicked, this, &PortPage::portToggle);

            m_port = new Camera(portConfig);
            // connect(m_port, &BasePort::appendLog, this, &PortPage::appendLog);
            connect(m_port, &BasePort::togglePort, this, [this](const bool status) {
                m_portToggleButton->setChecked(status);
            });
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
