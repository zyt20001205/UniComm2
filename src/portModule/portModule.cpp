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
      m_portStandardItemModel(new QStandardItemModel()),
      m_portSetting(new PortSetting(this)) {
    setWidget(m_portWidget);
    connect(m_portSetting, &PortSetting::insertPort, this, &PortModule::portInsert);
    connect(m_portSetting, &PortSetting::editPort, this, &PortModule::portEdit);
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

void PortModule::portConfigSave() {
    QVariant portList{};
    QMetaObject::invokeMethod(m_portRoot, "getOrder",Q_RETURN_ARG(QVariant, portList));
    QJsonArray portConfigArray{};
    for (const auto &portName: portList.toStringList()) {
        QJsonObject portConfig = m_portHash[portName]->config();
        portConfigArray.append(portConfig);
    }
    g_workspaceConfig["portConfig"] = portConfigArray;
}

BasePort *PortModule::currentPort() const {
    return nullptr;
}

void PortModule::portList(QSet<QString> &portList) const {
    for (const QString &portName: m_portHash.keys()) {
        portList.insert(portName);
    }
}

void PortModule::portSetting(const QString &portName) const {
    if (portName.isEmpty()) {
        m_portSetting->portSettingImport();
    } else {
        const auto &portObject = m_portHash[portName];
        const auto &portConfig = portObject->config();
        m_portSetting->portSettingImport(portConfig);
    }
}

void PortModule::portInsert(int index, const QJsonObject &portConfig) {
    const QString portName = portConfig["portName"].toString();
    m_portStandardItemModel->appendRow(new QStandardItem(portName));
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
    connect(port, &BasePort::togglePort, this, [this, portName](const bool status) { portSetChecked(portName, status); });
    m_portHash.insert(portName, port);
    // logging
    emit appendLog(QString("%1 initialized").arg(portName), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 initialized").arg(timestamp, portName);
}

void PortModule::portRemove(const QString &portName) {
    for (int row = 0; row < m_portStandardItemModel->rowCount(); ++row) {
        if (m_portStandardItemModel->item(row, 0)->text() == portName) {
            m_portStandardItemModel->removeRow(row);
            break;
        }
    }
    auto *port = m_portHash[portName];
    QMetaObject::invokeMethod(port, [&port] {
        port->close();
    }, Qt::BlockingQueuedConnection);
    delete port;
    m_portHash.remove(portName);
    // logging
    emit appendLog(QString("%1 removed").arg(portName), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 removed").arg(timestamp, portName);
}

void PortModule::portEdit(const QString &oldPortName, const QJsonObject &portConfig) {
    portRemove(oldPortName);
    portInsert(-1, portConfig);
}

void PortModule::portToggle(const QString &portName, const bool status) {
    auto port = m_portHash[portName];
    if (status) {
        bool ok = false;
        QMetaObject::invokeMethod(port, [&port, &ok] {
            ok = port->open();
        }, Qt::BlockingQueuedConnection);
        portSetChecked(portName, ok);
    } else {
        QMetaObject::invokeMethod(port, [&port] {
            port->close();
        }, Qt::BlockingQueuedConnection);
        portSetChecked(portName, status);
    }
}

// PortModule private
void PortModule::portSetChecked(const QString &portName, const bool status) const {
    QMetaObject::invokeMethod(m_portRoot, "setChecked", Q_ARG(QVariant, portName), Q_ARG(QVariant, status));
}
