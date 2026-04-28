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
        Success = 0x00,
        UnspecifiedError = 0x80,
        MalformedPacket,
        ProtocolError,
        ImplementationSpecificError,
        UnsupportedProtolVersion,
        ClientIdentifierNotValid,
        BadUserNameOrPassword,
        NotAuthorized,
        ServiceUnavailable,
        ServerBusy,
        Banned,
        BadAuthenticationMethod = 0x8C,
        TopicNameInvalid = 0x90,
        PacketTooLarge = 0x95,
        QuotaExceeded = 0x97,
        PayloadFormatInvalid = 0x99,
        RetainNotSupported,
        QosNotSupported,
        UseAnotherServer,
        ServerMethod,
        ConnectionRateExceeded = 0x9F
    };
};

#endif //UNICOMM_MQTT_H
