#include "api/ftp.h"

#include <QHostAddress>
#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <sol/error.hpp>

#include "globals.h"
#include "port/tcpServer.h"
#include "port/portModule.h"

// public
Ftp::Ftp(QObject *parent)
    : QObject(parent) {
}

void Ftp::init(const std::string &portName, const int timeout) {
    const auto name = QString::fromStdString(portName);
    const auto port = g_port->m_portHash.constFind(name);
    if (port == g_port->m_portHash.constEnd()) throw sol::error(portName + " does not exist");
    m_portName = portName;
    m_timeout = timeout;
    m_port = static_cast<TcpServer *>(port.value());
}

// private
QVariantHash Ftp::parser(const QByteArray &rxData) {
    return {};
}
