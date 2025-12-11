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

std::string LuaPort::read(const std::string &portName, int timeout, int length, const std::string &peerIp) {
    bool status = false;
    QByteArray rxData{};
    emit readPort(QString::fromStdString(portName), timeout, length, QString::fromStdString(peerIp), status, rxData);
    if (!status) {
        throw sol::error("failed to read port: " + portName);
    }
    return std::string(rxData.constData(), rxData.size());
}
