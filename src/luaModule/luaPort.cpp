#include "luaModule/luaPort.h"

#include <sol/sol.hpp>

#include "portModule/basePort.h"
#include "portModule/portModule.h"


LuaPort::LuaPort(QObject *parent)
    : QObject(parent) {
}

std::vector<std::string> LuaPort::list() {
    std::vector<std::string> portList{};
    emit listPort(portList);
    return portList;
}

std::unordered_map<std::string, std::string> LuaPort::info(const std::string &portName) {
    std::unordered_map<std::string, std::string> portInfo{};
    emit infoPort(QString::fromStdString(portName), portInfo);
    return portInfo;
}

void LuaPort::open(const std::string &portName) {
    bool status = false;
    emit openPort(QString::fromStdString(portName), status);
    if (!status) {
        throw sol::error("failed to open port: " + portName);
    }
}

void LuaPort::close(const std::string &portName) {
    bool status = false;
    emit closePort(QString::fromStdString(portName), status);
    if (!status) {
        throw sol::error("failed to close port: " + portName);
    }
}

void LuaPort::write(const std::string &portName, const std::string_view &data, const std::string &peerIp) {
    const QByteArray txData(data.data(), static_cast<qsizetype>(data.size()));
    bool status = false;
    emit writePort(QString::fromStdString(portName), txData, QString::fromStdString(peerIp), status);
    if (!status) {
        throw sol::error("failed to write port: " + portName);
    }
}

std::string_view LuaPort::read(const std::string &portName, int timeout, int length, const std::string &peerIp) {
    bool status = false;
    QByteArray rxData{};
    emit readPort(QString::fromStdString(portName), timeout, length, QString::fromStdString(peerIp), status, rxData);
    if (!status) {
        throw sol::error("failed to write port: " + portName);
    }
    const std::string_view data(rxData.constData(), rxData.size());
    return data;
}

// int lua_portRead(lua_State *L) {
//     // check arguments
//     if (lua_gettop(L) != 1 && lua_gettop(L) != 2 && lua_gettop(L) != 3 && lua_gettop(L) != 4)
//         luaL_error(L, "unexpected number of arguments");
//     // extract arguments
//     const char *param1 = luaL_checkstring(L, 1);
//     const int param2 = static_cast<int>(luaL_optinteger(L, 2, 0));
//     const int param3 = static_cast<int>(luaL_optinteger(L, 3, 0));
//     const char *param4 = luaL_optstring(L, 4, nullptr);
//     // start operation
//     const QString portName = QString::fromUtf8(param1);
//     if (!g_port->m_portHash.contains(portName)) {
//         luaL_error(L, "port '%s' does not exist", portName.toUtf8().constData());
//     } else {
//         QByteArray rxData;
//         auto *portObject = g_port->m_portHash[portName];
//         const int timeout = param2;
//         const int length = param3;
//         if (param4) {
//             const QString peerIp = QString::fromUtf8(param4);
//             QMetaObject::invokeMethod(portObject, [&rxData, portObject, timeout, length, peerIp] {
//                 rxData = portObject->read(timeout, length, peerIp, "");
//             }, Qt::BlockingQueuedConnection);
//         } else {
//             QMetaObject::invokeMethod(portObject, [&rxData, portObject, timeout, length] {
//                 rxData = portObject->read(timeout, length, "");
//             }, Qt::BlockingQueuedConnection);
//         }
//         if (timeout != 0 && length != 0 && rxData.isEmpty()) {
//             luaL_error(L, "port read timeout");
//             return 0;
//         }
//         lua_pushlstring(L, rxData.constData(), rxData.size());
//         return 1;
//     }
// }
