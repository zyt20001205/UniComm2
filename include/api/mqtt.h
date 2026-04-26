#ifndef UNICOMM_MQTT_H
#define UNICOMM_MQTT_H

#include <QHash>
#include <QObject>
#include <sol/object.hpp>

class BasePort;

class Mqtt final : public QObject {
    Q_OBJECT

public:
    explicit Mqtt(QObject *parent = nullptr);

    ~Mqtt() override = default;

    [[nodiscard]] sol::object optionLog(sol::this_state ts) const;

    [[nodiscard]] sol::object optionGet(sol::this_state ts, const std::string &key) const;

    void optionSet(const std::string &key, const sol::object &value);

    [[nodiscard]] sol::table optionsProxy(sol::this_state ts) const;

    void init(const std::string &portName, int timeout);

    void connect();

    QVariantHash m_options{};

private:
    [[nodiscard]] static QString parser(int controlPacket, int reasonCode);

    [[nodiscard]] int sizeGet(QString &exception) const;

    static void stringAppend(QByteArray &ba, const QString &s);

    std::string m_portName{};
    int m_timeout{};
    BasePort *m_port{};

    enum ControlPacket: int {
        CONNECT
    };

    enum ConnectReasonCode: int {
        SUCCESS = 0x00,
        UNSPECIFIED_ERROR = 0x80,
        MALFORMED_PACKET,
        PROTOCOL_ERROR,
        IMPLEMENTATION_SPECIFIC_ERROR,
        UNSUPPORTED_PROTOCOL_VERSION,
        CLIENT_IDENTIFIER_NOT_VALID,
        BAD_USER_NAME_OR_PASSWORD,
        NOT_AUTHORIZED,
        SERVICE_UNAVAILABLE,
        SERVER_BUSY,
        BANNED,
        BAD_AUTHENTICATION_METHOD = 0x8C,
        TOPIC_NAME_INVALID = 0x90,
        PACKET_TOO_LARGE = 0x95,
        QUOTA_EXCEEDED = 0x97,
        PAYLOAD_FORMAT_INVALID = 0x99,
        RETAIN_NOT_SUPPORTED,
        QOS_NOT_SUPPORTED,
        USE_ANOTHER_SERVER,
        SERVER_METHOD,
        CONNECTION_RATE_EXCEEDED = 0x9F
    };
};

#endif //UNICOMM_MQTT_H
