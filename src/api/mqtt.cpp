#include "api/mqtt.h"

#include <sol/state_view.hpp>
#include <sol/table_core.hpp>

#include "globals.h"
#include "port/basePort.h"
#include "port/portModule.h"
#include "util/uniCast.h"

// public
Mqtt::Mqtt(QObject *parent)
    : QObject(parent) {
    m_options = {
        {"protocol", "MQTT 3.1"},
        {"clientId", "anything"},
        {"cleanStart", true},
        {"username", ""},
        {"password", ""},
        {"keepAlive", 60},
    };
}

sol::object Mqtt::optionLog(const sol::this_state ts) const {
    return uni_cast<sol::object>(ts, m_options);
}

sol::object Mqtt::optionGet(const sol::this_state ts, const std::string &key) const {
    const auto _key = QString::fromStdString(key);
    if (!m_options.contains(_key)) throw sol::error("invalid mqtt option: " + key);
    return uni_cast<sol::object>(ts, m_options[_key]);
}

void Mqtt::optionSet(const std::string &key, const sol::object &value) {
    const auto _key = QString::fromStdString(key);
    if (!m_options.contains(_key)) throw sol::error("invalid mqtt option: " + key);
    if (_key == "protocol") {
        if (!value.is<std::string>()) throw sol::error("invalid mqtt protocol");
        const auto _value = value.as<std::string>();
        if (_value != "MQTT 3.1" && _value != "MQTT 3.1.1" && _value != "MQTT 5.0") throw sol::error("invalid mqtt protocol");
    }
    m_options[_key] = uni_cast<QVariant>(value);
}

sol::table Mqtt::optionsProxy(const sol::this_state ts) const {
    sol::state_view lua(ts);
    sol::table table = lua.create_table();
    sol::table metatable = lua.create_table();
    metatable[sol::meta_function::pairs] = [this](const sol::this_state _ts) {
        return std::make_tuple(sol::state_view(_ts)["next"].get<sol::function>(), this->optionLog(_ts).as<sol::table>(), sol::nil);
    };
    metatable[sol::meta_function::index] = [this](const sol::table &, const std::string &key, const sol::this_state _ts) {
        return this->optionGet(_ts, key);
    };
    metatable[sol::meta_function::new_index] = [this](const sol::table &, const std::string &key, const sol::object &val) {
        const_cast<Mqtt *>(this)->optionSet(key, val);
    };
    table[sol::metatable_key] = metatable;
    return table;
}

void Mqtt::init(const std::string &portName, const int timeout) {
    const auto _portName = QString::fromStdString(portName);
    const auto port = g_port->m_portHash.constFind(_portName);
    if (port == g_port->m_portHash.constEnd()) throw sol::error(portName + " does not exist");
    m_portName = portName;
    m_timeout = timeout;
    m_port = port.value();

    QString exception{};

    QMetaObject::invokeMethod(m_port, [&exception, this] {
        if (!m_port->open()) {
            exception = "open failed";
            return;
        }
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
}

void Mqtt::connect() {
    QString exception{};
    QByteArray rxData{};
    QByteArray variableHeader{};
    {
        QByteArray protocolName{};
        if (m_options["protocol"] == "MQTT 3.1") stringAppend(protocolName, "MQIsdp");
        else if (m_options["protocol"] == "MQTT 3.1.1" || m_options["protocol"] == "MQTT 5.0") stringAppend(protocolName, "MQTT");
        else throw sol::error("invalid mqtt protocol");
        variableHeader += protocolName;

        qint8 protocolVersion{};
        if (m_options["protocol"] == "MQTT 3.1") protocolVersion = 3;
        else if (m_options["protocol"] == "MQTT 3.1.1") protocolVersion = 4;
        else if (m_options["protocol"] == "MQTT 5.0") protocolVersion = 5;
        else throw sol::error("invalid mqtt protocol");
        variableHeader += protocolVersion;

        quint8 connectFlags{};
        if (m_options["cleanStart"] == true) connectFlags |= 1 << 1;
        // TODO: willQoS
        // TODO: willRetain
        if (m_options["password"] != "") connectFlags |= 1 << 6;
        if (m_options["username"] != "") connectFlags |= 1 << 7;
        variableHeader.append(static_cast<qint8>(connectFlags));

        const auto keepAlive = m_options["keepAlive"].toUInt();
        variableHeader.append(static_cast<qint8>(keepAlive >> 8 & 0xFF));
        variableHeader.append(static_cast<qint8>(keepAlive & 0xFF));

        // TODO: properties
    }
    QByteArray payload{};
    {
        stringAppend(payload, m_options["clientId"].toString());
        if (m_options["username"] != "") stringAppend(payload, m_options["username"].toString());
        if (m_options["password"] != "") stringAppend(payload, m_options["password"].toString());
    }
    QByteArray fixedHeader = "\x10";
    auto size = variableHeader.size() + payload.size();
    do {
        quint8 byte = size % 128;
        size /= 128;
        if (size > 0) byte |= 128;
        fixedHeader.append(static_cast<char>(byte));
    } while (size > 0);
    const auto txData = fixedHeader + variableHeader + payload;

    QMetaObject::invokeMethod(m_port, [&exception, &rxData, this, &txData] {
        if (!m_port->write(txData, "raw", "null")) {
            exception = "write failed";
            return;
        }

        rxData = m_port->read(1, m_timeout, "raw");
        if (rxData.isEmpty()) {
            exception = "read timeout";
            return;
        }
        if (rxData != '\x20') {
            exception = "mqtt control packet type mismatch";
            return;
        }
        rxData = m_port->read(sizeGet(exception), m_timeout, "raw");
        if (rxData.isEmpty()) {
            exception = "read timeout";
            return;
        }
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());

    if (m_options["protocol"] == "MQTT 3.1") {
        if (static_cast<quint8>(rxData.at(0)) != 0)
            exception = "mqtt connect acknowledge flags error";
            // TODO
        else;
    }
    exception = parser(CONNECT, static_cast<quint8>(rxData.at(1)));
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
}

// private
QString Mqtt::parser(const int controlPacket, const int reasonCode) {
    switch (controlPacket) {
        case CONNECT: {
            switch (reasonCode) {
                case Success: return "";
                case UnspecifiedError: return "The Server does not wish to reveal the reason for the failure, or none of the other Reason Codes apply.";
                case MalformedPacket: return "Data within the CONNECT packet could not be correctly parsed.";
                case ProtocolError: return "Data in the CONNECT packet does not conform to this specification.";
                case ImplementationSpecificError: return "The CONNECT is valid but is not accepted by this Server";
                case UnsupportedProtolVersion: return "The Server does not support the version of the MQTT protocol requested by the Client.";
                case ClientIdentifierNotValid: return "The Client Identifier is a valid string but is not allowed by the Server.";
                case BadUserNameOrPassword: return "The Server does not accept the User Name or Password specified by the Client.";
                case NotAuthorized: return "The Client is not authorized to connect.";
                case ServiceUnavailable: return "The MQTT Server is not available.";
                case ServerBusy: return "The Server is busy. Try again later.";
                case Banned: return "This Client has been banned by administrative action. Contact the server administrator.";
                case BadAuthenticationMethod: return "The authentication method is not supported or does not match the authentication method currently in use.";
                case TopicNameInvalid: return "The Will Topic Name is not malformed, but is not accepted by this Server.";
                case PacketTooLarge: return "The CONNECT packet exceeded the maximum permissible size.";
                case QuotaExceeded: return "An implementation or administrative imposed limit has been exceeded.";
                case PayloadFormatInvalid: return "The Will Payload does not match the specified Payload Format ScintillaIndicator.";
                case RetainNotSupported: return "The Server does not support retained messages, and Will Retain was set to 1.";
                case QosNotSupported: return "The Server does not support the QoS set in Will QoS.";
                case UseAnotherServer: return "The Client should temporarily use another server.";
                case ServerMethod: return "The Client should permanently use another server.";
                case ConnectionRateExceeded: return "The connection rate limit has been exceeded.";
                default: return "mqtt connect reason code unknown";
            }
        }
        default: return "mqtt control packet type unknown";
    }
}

int Mqtt::sizeGet(QString &exception) const {
    QByteArray rxData{};
    int size{};
    int index = 1;
    quint8 byte{};
    do {
        rxData = m_port->read(1, m_timeout, "raw");
        if (rxData.isEmpty()) {
            exception = "read timeout";
            return 0;
        }
        byte = static_cast<quint8>(rxData[0]);
        size += (byte & 0x07) * index;
        index *= 128;
        if (index > 128 * 128 * 128) {
            exception = "invalid size";
            return 0;
        }
    } while (byte & 0x80);
    return size;
}

void Mqtt::stringAppend(QByteArray &ba, const QString &s) {
    const auto _s = s.toUtf8();
    ba.append(static_cast<qint8>(_s.size() >> 8));
    ba.append(static_cast<qint8>(_s.size() & 0xFF));
    ba.append(_s);
}
