#include "portModule/portModule.h"

#include <QContextMenuEvent>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QQmlContext>
#include <QQuickItem>
#include <QStandardItemModel>
#include <QTabBar>
#include <QTimer>
#include <QVBoxLayout>

#include "globals.h"
#include "portModule/basePort.h"
#include "portModule/camera.h"
#include "portModule/portPage.h"
#include "portModule/portSetting.h"
#include "portModule/screen.h"
#include "portModule/serialPort.h"
#include "portModule/tcpClient.h"
#include "portModule/tcpServer.h"
#include "portModule/udpSocket.h"
#include "portModule/visa.h"

// PortModule public
PortModule::PortModule()
    : DockWidget("port"),
      m_portWidget(new QQuickWidget()),
      m_portStandardItemModel(new QStandardItemModel()),
      m_portTabWidget(new QTabWidget()) {
    setWidget(m_portWidget);
}

void PortModule::propertySet(const QVariantMap &objects) {
    m_portWidget->rootContext()->setContextProperty("portMenu", qvariant_cast<QObject *>(objects["portModulePortMenu"]));
    m_portWidget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["portModuleRootMenu"]));

    for (const auto &value: g_workspaceConfig["portConfig"].toArray()) {
        const QJsonObject portConfig = value.toObject();
        portInsert(-1, portConfig);
    }
    m_portWidget->rootContext()->setContextProperty("portModule", this);
    m_portWidget->rootContext()->setContextProperty("standardItemModel", m_portStandardItemModel);
    m_portWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_portWidget->setSource(QUrl("qrc:/qml/portModule/portModule.qml"));
    m_portRoot = m_portWidget->rootObject();
}

void PortModule::portConfigSave() const {
    g_workspaceConfig["portConfig"] = m_portConfig;
}

BasePort *PortModule::currentPort() const {
    if (m_portTabWidget->currentWidget()) {
        return static_cast<PortPage *>(m_portTabWidget->currentWidget())->m_port;
    }
    return nullptr;
}

void PortModule::portList(std::vector<std::string> &portList) const {
    for (const QString &portName: m_portHash.keys()) {
        portList.push_back(portName.toStdString());
    }
}

void PortModule::portInsert(int index, QJsonObject portConfig) {
    if (index == -1) {
        index = m_portConfig.size();
    }
    if (portConfig.isEmpty()) {
        const QSet usedPortName(m_portHash.keyBegin(), m_portHash.keyEnd());
        if (PortSetting portSettingDialog(usedPortName); portSettingDialog.exec() == QDialog::Accepted) {
            portConfig = portSettingDialog.portSettingExport();
        } else {
            return;
        }
    }
    const QString portName = portConfig["portName"].toString();
    m_portStandardItemModel->appendRow(new QStandardItem(portName));
    BasePort* port{};
    switch (portConfig["portType"].toInt()) {
        case SERIALPORT: {
            port = new SerialPort(portConfig);
            break;
        }
        case VISA: {
            port = new Visa(portConfig);
            break;
        }
        case TCPCLIENT: {
            port = new TcpClient(portConfig);
            break;
        }
        case TCPSERVER: {
            port = new TcpServer(portConfig);
            break;
        }
        case UDPSOCKET: {
            port = new UdpSocket(portConfig);
            break;
        }
        case SCREEN: {
            port = new Screen(portConfig);
            // connect(m_port, &BasePort::togglePort, this, [this](const bool status) {
            //     m_portToggleButton->setChecked(status);
            //     m_pixmapPreview->setVisible(status);
            // });
            // connect(m_port, &BasePort::showPreview, m_pixmapPreview, &PixmapPreview::previewShow);
            break;
        }
        case CAMERA: {
            port = new Camera(portConfig);
            // connect(m_port, &BasePort::togglePort, this, [this](const bool status) {
            //     m_portToggleButton->setChecked(status);
            //     m_pixmapPreview->setVisible(status);
            // });
            // connect(m_port, &BasePort::showPreview, m_pixmapPreview, &PixmapPreview::previewShow);
            break;
        }
        default: {
            qDebug() << "unknown port type";
            break;
        }
    }
    connect(port, &BasePort::appendLog, this, &PortModule::appendLog);
    m_portConfig.insert(index, portConfig);
    m_portHash.insert(portName, port);
    // logging
    emit appendLog(QString("%1 initialized").arg(portName), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 initialized").arg(timestamp, portName);
}

void PortModule::portRemove(const int index) {
    QJsonObject portConfig = m_portConfig[index].toObject();
    QString portName = portConfig["portName"].toString();
    const QMessageBox::StandardButton reply = QMessageBox::question(
        nullptr,
        tr("Remove Port"),
        QString(tr("Are you sure to remove port %1?")).arg(portName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (reply != QMessageBox::Yes) return;
    // frontend
    QWidget *w = m_portTabWidget->widget(index);
    m_portTabWidget->removeTab(index);
    if (w) w->deleteLater();
    // backend
    m_portConfig.removeAt(index);
    m_portHash.remove(portName);
    // logging
    emit appendLog(QString("%1 removed").arg(portName), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 removed").arg(timestamp, portName);
}

void PortModule::portReload(const int index) {
    const auto *portPage = static_cast<PortPage *>(m_portTabWidget->widget(index));
    PortSetting portSettingDialog{};
    const QJsonObject oldPortConfig = m_portConfig[index].toObject();
    const QString oldPortName = oldPortConfig["portName"].toString();
    portSettingDialog.portSettingImport(oldPortConfig);
    if (portSettingDialog.exec() == QDialog::Accepted) {
        const QJsonObject newPortConfig = portSettingDialog.portSettingExport();
        const QString newPortName = newPortConfig["portName"].toString();
        if (newPortName != oldPortName) {
            // frontend
            m_portTabWidget->setTabText(index, newPortName);
            // backend
            BasePort *port = m_portHash.value(oldPortName);
            m_portHash.remove(oldPortName);
            m_portHash.insert(newPortName, port);
            portAnnotate();
        }
        m_portConfig[index] = newPortConfig;
        portPage->portReload(newPortConfig);
        // logging
        emit appendLog(QString("%1 reloaded").arg(oldPortName), "info");
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 reloaded").arg(timestamp, oldPortName);
    }
}

void PortModule::portAnnotate() const {
    QString annotation;
    annotation += "--- @meta\n\n";
    annotation += "--- @alias port\n";
    for (const QString &portName: m_portHash.keys()) {
        const QString escapedName = QString(portName).replace("\\", "\\\\");
        annotation += QString("--- | '\"%1\"'\n").arg(escapedName);
    }
    annotation += QString("--- | '\"Add New Port\"'\n");
    annotation += "\n";

    const QString rootPath = g_workspaceUrl.toLocalFile();
    const QString annotationPath = QDir(rootPath).filePath("lib/port.d.lua");
    QFile file(annotationPath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&file);
    stream << annotation;
    file.close();
}

void PortModule::portToggle(const QString &portName, const bool status) {
    auto port = m_portHash[portName];
    if (status) {
        bool ok = false;
        QMetaObject::invokeMethod(port, [&port, &ok] {
            ok = port->open();
        }, Qt::BlockingQueuedConnection);
        QMetaObject::invokeMethod(m_portRoot, "setChecked", Q_ARG(QVariant, portName), Q_ARG(QVariant, ok));
    } else {
        QMetaObject::invokeMethod(port, [&port] {
            port->close();
        }, Qt::BlockingQueuedConnection);
        QMetaObject::invokeMethod(m_portRoot, "setChecked", Q_ARG(QVariant, portName), Q_ARG(QVariant, status));
    }
}

// PortModule protected
void PortModule::contextMenuEvent(QContextMenuEvent *event) {
    const QPoint globalPos = event->globalPos();
    const auto *tabBar = m_portTabWidget->tabBar();
    const QPoint tabBarPos = tabBar->mapFromGlobal(globalPos);
    if (tabBar->rect().contains(tabBarPos)) {
        const int index = tabBar->tabAt(tabBarPos);
        m_portTabWidget->setCurrentIndex(index);
        QMenu menu(this);
        menu.addAction("edit", [this, index] { portReload(index); });
        menu.exec(event->globalPos());
    }
}

// PortModule private
void PortModule::portSwap(const int srcIndex, const int dstIndex) {
    // config
    const QJsonValue tmp = m_portConfig.takeAt(srcIndex);
    m_portConfig.insert(dstIndex, tmp);
}
