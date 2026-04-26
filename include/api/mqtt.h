#ifndef UNICOMM_MQTT_H
#define UNICOMM_MQTT_H

#include <QHash>
#include <QObject>
#include <sol/object.hpp>

class Mqtt final : public QObject {
    Q_OBJECT

public:
    explicit Mqtt(QObject *parent = nullptr);

    ~Mqtt() override = default;

    [[nodiscard]] sol::object optionLog(sol::this_state ts) const;

    [[nodiscard]] sol::object optionGet(sol::this_state ts, const std::string &key) const;

    void optionSet(const std::string &key, const sol::object &value);

    [[nodiscard]] sol::table optionsProxy(sol::this_state ts) const;

    // [[nodiscard]] static std::string readHoldingRegisters(const std::string &portName, int slaveAddr, int startAddr, int quantity, int timeout);
    //
    // static void writeSingleRegister(const std::string &portName, int slaveAddr, int regAddr, const std::string &data, int timeout);
    //
    // static void writeMultipleRegisters(const std::string &portName, int slaveAddr, int startAddr, const std::string &data, int timeout);

    QVariantHash m_options{};
};

#endif //UNICOMM_MQTT_H
