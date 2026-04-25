#ifndef UNICOMM_MQTT_H
#define UNICOMM_MQTT_H

#include <QObject>

class Mqtt final : public QObject {
    Q_OBJECT

public:
    explicit Mqtt(QObject *parent = nullptr);

    ~Mqtt() override = default;

    // [[nodiscard]] static std::string readHoldingRegisters(const std::string &portName, int slaveAddr, int startAddr, int quantity, int timeout);
    //
    // static void writeSingleRegister(const std::string &portName, int slaveAddr, int regAddr, const std::string &data, int timeout);
    //
    // static void writeMultipleRegisters(const std::string &portName, int slaveAddr, int startAddr, const std::string &data, int timeout);
};

#endif //UNICOMM_MQTT_H
