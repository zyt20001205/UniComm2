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

    QVariantHash m_options{};

private:
    std::string m_portName{};
    int m_timeout{};
    BasePort *m_port{};
};

#endif //UNICOMM_MQTT_H
