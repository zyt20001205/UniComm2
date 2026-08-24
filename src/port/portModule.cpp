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
#include "core/undoModule.h"
#include "port/basePort.h"
#include "port/bluetoothLe.h"
#include "port/module/deviceDiscovery.h"
#include "port/portSetting.h"
#include "port/serialPort.h"
#include "port/tcpClient.h"
#include "port/tcpServer.h"
#include "port/sslClient.h"
#include "port/sslServer.h"
#include "port/webSocketClient.h"
#include "port/webSocketServer.h"
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
        auto portConfig = value.toObject();
        portDefaults(portConfig);
        if (!portCheck(portConfig).isEmpty()) continue;
        _portInsert(g_portModel->rowCount(), portConfig);
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

QJsonObject PortModule::portConfigGet(const int portType) {
    QJsonObject portConfig{};
    switch (portType) {
        case PortType::SerialPort: {
            portConfig = {
                {"type", "object"},
                {"required", QJsonArray{"portName"}},
                {
                    "defaults", QJsonObject{
                        {"baudRate", 115200},
                        {"dataBits", 8},
                        {"parity", 0},
                        {"stopBits", 1},
                        {"txFormat", "utf-8"},
                        {"txSuffix", "null"},
                        {"rxFormat", "utf-8"},
                        {"bufferSize", 65536}
                    }
                },
                {
                    "properties", QJsonObject{
                        {"portName", QJsonObject{{"type", "string"}, {"enum", QJsonArray::fromStringList(DeviceDiscovery::serialPorts())}}},
                        {"baudRate", QJsonObject{{"type", "integer"}, {"minimum", 1}, {"maximum", 5000000}}},
                        {"dataBits", QJsonObject{{"enum", QJsonArray{5, 6, 7, 8}}}},
                        {"parity", QJsonObject{{"enum", QJsonArray{0, 2, 3, 4, 5}}, {"description", "0=no, 2=even, 3=odd, 4=space, 5=mark"}}},
                        {"stopBits", QJsonObject{{"enum", QJsonArray{1, 2, 3}}, {"description", "1=one, 2=two, 3=one and a half"}}},
                        {"txFormat", QJsonObject{{"enum", QJsonArray{"raw", "hex", "ascii", "utf-8"}}}},
                        {"txSuffix", QJsonObject{{"enum", QJsonArray{"null", "crlf", "modbus crc", "modbus lrc"}}}},
                        {"rxFormat", QJsonObject{{"enum", QJsonArray{"raw", "hex", "ascii", "utf-8"}}}},
                        {"bufferSize", QJsonObject{{"type", "integer"}, {"minimum", 1}, {"maximum", 1048576}}}
                    }
                }
            };
            break;
        }
        case PortType::TcpClient:
        case PortType::SslClient: {
            portConfig = {
                {"type", "object"},
                {
                    "required", QJsonArray{
                        "portName",
                        "remoteHost",
                        "remotePort"
                    }
                },
                {
                    "defaults", QJsonObject{
                        {"txFormat", "utf-8"},
                        {"txSuffix", "null"},
                        {"rxFormat", "utf-8"},
                        {"bufferSize", 65536}
                    }
                },
                {
                    "properties", QJsonObject{
                        {"portName", QJsonObject{{"type", "string"}}},
                        {"remoteHost", QJsonObject{{"type", "string"}}},
                        {"remotePort", QJsonObject{{"type", "integer"}, {"minimum", 1}, {"maximum", 65535}}},
                        {"txFormat", QJsonObject{{"enum", QJsonArray{"raw", "hex", "ascii", "utf-8"}}}},
                        {"txSuffix", QJsonObject{{"enum", QJsonArray{"null", "crlf", "modbus crc", "modbus lrc"}}}},
                        {"rxFormat", QJsonObject{{"enum", QJsonArray{"raw", "hex", "ascii", "utf-8"}}}},
                        {"bufferSize", QJsonObject{{"type", "integer"}, {"minimum", 1}, {"maximum", 1048576}}}
                    }
                }
            };
            break;
        }
        default: break;
    }
    return portConfig;
}

QString PortModule::portCheck(const QJsonObject &portConfig, const QString &oldPortName) const {
    const auto portType = portConfig.value("portType").toInt(-1);
    const auto portName = portConfig.value("portName").toString();
    if (portType < PortType::SerialPort || portType > PortType::BluetoothLe) return QString("Unsupported port type: %1.").arg(portType);
    if (portName.trimmed().isEmpty()) return "Port check failed: portName is empty.";
    if (portName != oldPortName && m_portHash.contains(portName)) return QString("Port check failed: '%1' already exists.").arg(portName);

    if (portType == PortType::SerialPort) {
        const auto baudRate = portConfig.value("baudRate").toInt();
        if (baudRate < 1 || baudRate > 5000000) return "Port check failed: baudRate must be between 1 and 5000000.";

        if (!QJsonArray{5, 6, 7, 8}.contains(portConfig.value("dataBits"))) return "Port check failed: invalid dataBits.";
        if (!QJsonArray{0, 2, 3, 4, 5}.contains(portConfig.value("parity"))) return "Port check failed: invalid parity.";
        if (!QJsonArray{1, 2, 3}.contains(portConfig.value("stopBits"))) return "Port check failed: invalid stopBits.";
    }

    if (portType == PortType::TcpClient || portType == PortType::SslClient) {
        if (portConfig.value("remoteHost").toString().trimmed().isEmpty()) return "Port check failed: remoteHost is empty.";

        const auto remotePort = portConfig.value("remotePort").toInt();
        if (remotePort < 1 || remotePort > 65535) return "Port check failed: remotePort must be between 1 and 65535.";
    }

    if (portType == PortType::SerialPort || portType == PortType::TcpClient || portType == PortType::SslClient) {
        const QJsonArray formats{"raw", "hex", "ascii", "utf-8"};
        if (!formats.contains(portConfig.value("txFormat"))) return "Port check failed: invalid txFormat.";
        if (!formats.contains(portConfig.value("rxFormat"))) return "Port check failed: invalid rxFormat.";

        const QJsonArray suffixes{"null", "crlf", "modbus crc", "modbus lrc"};
        if (!suffixes.contains(portConfig.value("txSuffix"))) return "Port check failed: invalid txSuffix.";

        const auto bufferSize = portConfig.value("bufferSize").toInt();
        if (bufferSize < 1 || bufferSize > 1048576) return "Port check failed: bufferSize must be between 1 and 1048576.";
    }
    return {};
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

QString PortModule::portInsert(int index, QJsonObject portConfig) {
    portDefaults(portConfig);

    const auto exception = portCheck(portConfig);
    const auto portName = portConfig.value("portName").toString();
    if (!exception.isEmpty()) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(portName), exception);
        return exception;
    }

    if (index == -1) index = g_portModel->rowCount();
    if (index < 0 || index > g_portModel->rowCount()) {
        const auto error = QString("Port insert failed: invalid index %1.").arg(index);
        emit appendLog(LogLevel::Error, QString("[%1]").arg(portName), error);
        return error;
    }

    g_undo->push(
        tr("Port Insert (%1)").arg(portName),
        [this, index, portConfig] { _portInsert(index, portConfig); },
        [this, portName] { _portRemove(portName); });
    return QString("Port '%1' inserted.").arg(portName);
}

void PortModule::portRemove(const int index) {
    if (index < 0 || index >= g_portModel->rowCount()) {
        emit appendLog(LogLevel::Error, "[Port]", QString("Port remove failed: invalid index %1.").arg(index));
        return;
    }

    const auto portName = g_portModel->item(index, 0)->text();
    const auto portConfig = m_portHash[portName]->config();
    g_undo->push(
        tr("Port Remove (%1)").arg(portName),
        [this, portName] { _portRemove(portName); },
        [this, index, portConfig] { _portInsert(index, portConfig); });
}

QString PortModule::portRemove(const QString &portName) {
    const auto name = portName.trimmed();
    const auto items = g_portModel->findItems(name);
    if (items.isEmpty()) return QString("Port remove failed: '%1' does not exist.").arg(name);

    portRemove(items.constFirst()->row());
    return QString("Port '%1' removed.").arg(name);
}

void PortModule::portMove(const int src, const int dst) {
    if (src < 0 || src >= g_portModel->rowCount() || dst < 0 || dst >= g_portModel->rowCount()) {
        emit appendLog(LogLevel::Error, "[Port]", "Port move failed: invalid index.");
        return;
    }
    if (src == dst) return;

    const auto portName = g_portModel->item(src, 0)->text();
    g_undo->push(
        tr("Port Move (%1: %2->%3)").arg(portName).arg(src + 1).arg(dst + 1),
        [this, src, dst] { _portMove(src, dst); },
        [this, src, dst] { _portMove(dst, src); });
}

void PortModule::portEdit(const QString &oldPortName, const QJsonObject &portConfig) {
    const auto items = g_portModel->findItems(oldPortName);
    if (items.isEmpty()) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(oldPortName), "Port edit failed: port does not exist.");
        return;
    }

    auto newPortConfig = portConfig;
    portDefaults(newPortConfig);
    const auto portName = newPortConfig.value("portName").toString();
    if (const auto exception = portCheck(newPortConfig, oldPortName); !exception.isEmpty()) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(portName), exception);
        return;
    }

    const auto index = items.constFirst()->row();
    const auto oldPortConfig = m_portHash[oldPortName]->config();
    if (newPortConfig == oldPortConfig) return;

    const auto commandText = portName == oldPortName
                                 ? tr("Port Edit (%1)").arg(portName)
                                 : tr("Port Rename (%1->%2)").arg(oldPortName, portName);
    g_undo->push(
        commandText,
        [this, index, oldPortName, newPortConfig] { _portEdit(index, oldPortName, newPortConfig); },
        [this, index, portName, oldPortConfig] { _portEdit(index, portName, oldPortConfig); });
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

// private
void PortModule::portDefaults(QJsonObject &portConfig) {
    const auto defaults = portConfigGet(portConfig.value("portType").toInt(-1)).value("defaults").toObject();
    for (auto iterator = defaults.constBegin(); iterator != defaults.constEnd(); ++iterator) {
        if (!portConfig.contains(iterator.key())) portConfig.insert(iterator.key(), iterator.value());
    }
}

void PortModule::_portInsert(const int index, const QJsonObject &portConfig) {
    const auto portName = portConfig.value("portName").toString();
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
        case PortType::TcpServer: {
            port = new TcpServer(portConfig);
            break;
        }
        case PortType::SslClient: {
            port = new SslClient(portConfig);
            break;
        }
        case PortType::SslServer: {
            port = new SslServer(portConfig);
            break;
        }
        case PortType::WebSocketClient: {
            port = new WebSocketClient(portConfig);
            break;
        }
        case PortType::WebSocketServer: {
            port = new WebSocketServer(portConfig);
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
        case PortType::BluetoothLe: {
            port = new BluetoothLe(portConfig);
            break;
        }
        default: break;
    }
    connect(port, &BasePort::appendLog, this, &PortModule::appendLog);
    connect(port, &BasePort::refreshPort, this, &PortModule::portRefresh);
    m_portHash.insert(portName, port);
    emit appendLog(LogLevel::Info, QString("[%1]").arg(portName), "initialized");
}

void PortModule::_portRemove(const QString &portName) {
    const auto items = g_portModel->findItems(portName);
    g_portModel->removeRow(items.constFirst()->row());
    auto *port = m_portHash.take(portName);
    auto *thread = port->thread();
    thread->quit();
    thread->wait();
    emit appendLog(LogLevel::Info, QString("[%1]").arg(portName), "removed");
}

void PortModule::_portEdit(const int index, const QString &oldPortName, const QJsonObject &portConfig) {
    _portRemove(oldPortName);
    _portInsert(index, portConfig);
}

void PortModule::_portMove(const int src, const int dst) {
    const auto tmp = g_portModel->takeRow(src);
    g_portModel->insertRow(dst, tmp);
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
