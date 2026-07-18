#include "port/portModule.h"

#include <QDir>
#include <QJsonArray>
#include <QLabel>
#include <QMenu>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QThread>
#include <QTimer>

#include "globals.h"
#include "core/globalManager.h"
#include "port/basePort.h"
#include "port/portSetting.h"
#include "port/serialPort.h"
#include "port/sslClient.h"
#include "port/tcpClient.h"
#include "port/tcpServer.h"
#include "port/udpSocket.h"
#include "port/videoStream.h"
#include "port/visa.h"

// public
PortModule::PortModule()
    : DockWidget("Port"),
      m_widget(new QQuickWidget()),
      m_portSetting(new PortSetting(this)) {
    setWidget(m_widget);
    connect(m_portSetting, &PortSetting::insertPort, this, &PortModule::portInsert);
    connect(m_portSetting, &PortSetting::editPort, this, &PortModule::portEdit);
    g_portModel = new PortModel(this);
    for (const auto &value: g_workspaceConfig["portConfig"].toArray()) {
        const QJsonObject portConfig = value.toObject();
        portInsert(-1, portConfig);
    }
}

PortModule::~PortModule() {
    for (const auto &port: m_portHash) {
        auto *thread = port->thread();
        thread->quit();
        thread->wait();
    }
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void PortModule::propertySet(const QVariantHash &objects) {
    m_widget->rootContext()->setContextProperty("portModule", this);
    m_widget->rootContext()->setContextProperty("global", g_globalManager);
    m_widget->rootContext()->setContextProperty("tableMenu", qvariant_cast<QObject *>(objects["portModuleTableMenu"]));
    m_widget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["portModuleRootMenu"]));
    m_widget->rootContext()->setContextProperty("portModel", g_portModel);

    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/port/portModule.qml"));
    m_root = m_widget->rootObject();

    m_portSetting->propertySet(QVariantHash{
        {"mainWindowToolTip", objects["mainWindowToolTip"]}
    });
}

void PortModule::portConfigSave() {
    QJsonArray portConfigArray{};
    for (int i = 0; i < g_portModel->rowCount(); ++i) {
        const QString portName = g_portModel->item(i, 0)->text();
        QJsonObject portConfig = m_portHash[portName]->config();
        portConfigArray.append(portConfig);
    }
    g_workspaceConfig["portConfig"] = portConfigArray;
}

QSet<QString> PortModule::portList() const {
    QSet<QString> keys{};
    for (const auto &portName: m_portHash.keys()) {
        keys.insert(portName);
    }
    return keys;
}

void PortModule::portSetting(const int index) const {
    if (index == -1) {
        m_portSetting->portSettingImport();
    } else {
        const auto *item = g_portModel->item(index, 0);
        const QString portName = item->text();
        const auto &portObject = m_portHash[portName];
        const auto &portConfig = portObject->config();
        m_portSetting->portSettingImport(portConfig);
    }
}

void PortModule::portInsert(int index, const QJsonObject &portConfig) {
    if (index == -1) index = g_portModel->rowCount();
    const QString portName = portConfig["portName"].toString();
    auto *item = new QStandardItem(portName); // NOLINT
    item->setData(false, Qt::UserRole + 1);
    item->setData(0, Qt::UserRole + 2);
    item->setData(0, Qt::UserRole + 3);
    item->setData(0, Qt::UserRole + 4);
    item->setData(0, Qt::UserRole + 5);
    item->setData(0, Qt::UserRole + 6);
    item->setData(0, Qt::UserRole + 7);
    item->setData(0, Qt::UserRole + 8);
    g_portModel->insertRow(index, item);
    BasePort *port{};
    switch (portConfig["portType"].toInt()) {
        case PortType::SerialPort: {
            port = new SerialPort(portConfig);
            break;
        }
        case PortType::Visa: {
            port = new Visa(portConfig);
            break;
        }
        case PortType::TcpClient: {
            port = new TcpClient(portConfig);
            break;
        }
        case PortType::SslClient: {
            port = new SslClient(portConfig);
            break;
        }
        case PortType::TcpServer: {
            port = new TcpServer(portConfig);
            break;
        }
        case PortType::UdpSocket: {
            port = new UdpSocket(portConfig);
            break;
        }
        case PortType::VideoStream: {
            port = new VideoStream(portConfig);
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
    emit appendLog(LogLevel::Info, QString("[%1]").arg(portName), "initialized");
}

void PortModule::portRemove(const int index) {
    const auto *item = g_portModel->item(index, 0);
    const QString portName = item->text();
    g_portModel->removeRow(index);
    auto *port = m_portHash.take(portName);
    auto *thread = port->thread();
    thread->quit();
    thread->wait();
    emit appendLog(LogLevel::Info, QString("[%1]").arg(portName), "removed");
}

void PortModule::portSwap(const int src, const int dst) {
    const auto tmp = g_portModel->takeRow(src);
    g_portModel->insertRow(dst, tmp);
}

void PortModule::portEdit(const QString &oldPortName, const QJsonObject &portConfig) {
    int oldIndex = -1;
    for (int row = 0; row < g_portModel->rowCount(); ++row) {
        if (g_portModel->item(row, 0)->text() == oldPortName) {
            oldIndex = row;
            break;
        }
    }
    portRemove(oldIndex);
    portInsert(oldIndex, portConfig);
}

void PortModule::portToggle(const int index) {
    const auto *item = g_portModel->item(index, 0);
    const QString portName = item->text();
    bool status = item->data(Qt::UserRole + 1).toBool();
    auto port = m_portHash[portName];
    if (status) {
        QMetaObject::invokeMethod(port, [port] {
            port->close();
        }, Qt::QueuedConnection);
    } else {
        QMetaObject::invokeMethod(port, [port] {
            port->open();
        }, Qt::QueuedConnection);
    }
}

void PortModule::portMonitor(const int index, const bool enabled) {
    const QString portName = g_portModel->item(index, 0)->text();
    auto *port = m_portHash[portName];
    QMetaObject::invokeMethod(port, [port, enabled] {
        port->monitor(enabled);
    }, Qt::QueuedConnection);
}

void PortModule::portRefresh(const QString &portName, const QVariantHash &session) {
    for (int row = 0; row < g_portModel->rowCount(); ++row) {
        auto *item = g_portModel->item(row, 0);
        if (item->text() == portName) {
            if (session.contains("active")) item->setData(session.value("active"), Qt::UserRole + 1);
            if (session.contains("capacity")) item->setData(session.value("capacity"), Qt::UserRole + 2);
            if (session.contains("used")) item->setData(session.value("used"), Qt::UserRole + 3);
            if (session.contains("lifetime")) item->setData(session.value("lifetime"), Qt::UserRole + 4);
            if (session.contains("readCount")) item->setData(session.value("readCount"), Qt::UserRole + 5);
            if (session.contains("readBytes")) item->setData(session.value("readBytes"), Qt::UserRole + 6);
            if (session.contains("writeCount")) item->setData(session.value("writeCount"), Qt::UserRole + 7);
            if (session.contains("writeBytes")) item->setData(session.value("writeBytes"), Qt::UserRole + 8);
            break;
        }
    }
}

// public
PortModel::PortModel(QObject *parent)
    : QStandardItemModel(parent) {
    connect(this, &QAbstractItemModel::rowsInserted, this, &PortModel::emptyChanged);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &PortModel::emptyChanged);
    connect(this, &QAbstractItemModel::modelReset, this, &PortModel::emptyChanged);
}

QHash<int, QByteArray> PortModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[Qt::UserRole + 1] = "active";
    roles[Qt::UserRole + 2] = "capacity";
    roles[Qt::UserRole + 3] = "used";
    roles[Qt::UserRole + 4] = "lifetime";
    roles[Qt::UserRole + 5] = "readCount";
    roles[Qt::UserRole + 6] = "readBytes";
    roles[Qt::UserRole + 7] = "writeCount";
    roles[Qt::UserRole + 8] = "writeBytes";
    return roles;
}
