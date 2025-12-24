#include "portModule/portModule.h"

#include <QDir>
#include <QFile>
#include <QLabel>
#include <QMenu>
#include <QQmlContext>
#include <QQuickItem>
#include <QStandardItemModel>
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
#include "portModule/visa.h"

// PortModule public
PortModule::PortModule()
    : DockWidget("port"),
      m_portWidget(new QQuickWidget()),
      m_portSetting(new PortSetting(this)) {
    setWidget(m_portWidget);
    connect(m_portSetting, &PortSetting::insertPort, this, &PortModule::portInsert);
    connect(m_portSetting, &PortSetting::editPort, this, &PortModule::portEdit);
    g_portStandardItemModel = new QStandardItemModel(this);
    for (const auto &value: g_workspaceConfig["portConfig"].toArray()) {
        const QJsonObject portConfig = value.toObject();
        portInsert(-1, portConfig);
    }
}

void PortModule::propertySet(const QVariantMap &objects) {
    m_portWidget->rootContext()->setContextProperty("portMenu", qvariant_cast<QObject *>(objects["portModulePortMenu"]));
    m_portWidget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["portModuleRootMenu"]));

    m_portWidget->rootContext()->setContextProperty("portModule", this);
    m_portWidget->rootContext()->setContextProperty("standardItemModel", g_portStandardItemModel);
    m_portWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_portWidget->setSource(QUrl("qrc:/qml/portModule/portModule.qml"));
    m_rootItem = m_portWidget->rootObject();
}

void PortModule::portConfigSave() {
    QVariant portList{};
    QMetaObject::invokeMethod(m_rootItem, "getOrder",Q_RETURN_ARG(QVariant, portList));
    QJsonArray portConfigArray{};
    for (const auto &portName: portList.toStringList()) {
        QJsonObject portConfig = m_portHash[portName]->config();
        portConfigArray.append(portConfig);
    }
    g_workspaceConfig["portConfig"] = portConfigArray;
}

void PortModule::portList(QSet<QString> &portList) const {
    for (const QString &portName: m_portHash.keys()) {
        portList.insert(portName);
    }
}

void PortModule::portSetting(const int index) const {
    if (index == -1) {
        m_portSetting->portSettingImport();
    } else {
        const auto *item = g_portStandardItemModel->item(index, 0);
        const QString portName = item->text();
        const auto &portObject = m_portHash[portName];
        const auto &portConfig = portObject->config();
        m_portSetting->portSettingImport(portConfig);
    }
}

void PortModule::portInsert(const int index, const QJsonObject &portConfig) {
    const QString portName = portConfig["portName"].toString();
    auto *item = new QStandardItem(portName); // NOLINT
    item->setData(false, Qt::WhatsThisRole);
    if (index == -1) g_portStandardItemModel->appendRow(item);
    else g_portStandardItemModel->insertRow(index, item);
    BasePort *port{};
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
            // connect(m_port, &BasePort::refreshPort, this, [this](const bool status) {
            //     m_portToggleButton->setChecked(status);
            //     m_pixmapPreview->setVisible(status);
            // });
            // connect(m_port, &BasePort::showPreview, m_pixmapPreview, &PixmapPreview::previewShow);
            break;
        }
        case CAMERA: {
            port = new Camera(portConfig);
            // connect(m_port, &BasePort::refreshPort, this, [this](const bool status) {
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
    connect(port, &BasePort::refreshPort, this, &PortModule::portRefresh);
    m_portHash.insert(portName, port);
    // logging
    emit appendLog(QString("%1 initialized").arg(portName), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 initialized").arg(timestamp, portName);
}

void PortModule::portRemove(const int index) {
    const auto *item = g_portStandardItemModel->item(index, 0);
    const QString portName = item->text();
    g_portStandardItemModel->removeRow(index);
    auto *port = m_portHash[portName];
    QMetaObject::invokeMethod(port, [&port] {
        port->close();
    }, Qt::BlockingQueuedConnection);
    delete port;
    m_portHash.remove(portName);
    m_portWidget->setSource(QUrl("qrc:/qml/portModule/portModule.qml"));
    // logging
    emit appendLog(QString("%1 removed").arg(portName), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 removed").arg(timestamp, portName);
}

void PortModule::portEdit(const QString &oldPortName, const QJsonObject &portConfig) {
    int oldIndex = -1;
    for (int row = 0; row < g_portStandardItemModel->rowCount(); ++row) {
        if (g_portStandardItemModel->item(row, 0)->text() == oldPortName) {
            oldIndex = row;
            break;
        }
    }
    portRemove(oldIndex);
    portInsert(oldIndex, portConfig);
}

void PortModule::portToggle(const int index) {
    const auto *item = g_portStandardItemModel->item(index, 0);
    const QString portName = item->text();
    bool status = item->data(Qt::WhatsThisRole).toBool();
    auto port = m_portHash[portName];
    if (status) {
        QMetaObject::invokeMethod(port, [&port] {
            port->close();
        }, Qt::BlockingQueuedConnection);
    } else {
        QMetaObject::invokeMethod(port, [&port, &status] {
            status = port->open();
        }, Qt::BlockingQueuedConnection);
    }
    portRefresh(portName, status);
}

void PortModule::portRefresh(const QString &portName, const bool status) const {
    for (int row = 0; row < g_portStandardItemModel->rowCount(); ++row) {
        auto *item = g_portStandardItemModel->item(row, 0);
        if (item->text() == portName) {
            item->setData(status, Qt::WhatsThisRole);
            break;
        }
    }
}
