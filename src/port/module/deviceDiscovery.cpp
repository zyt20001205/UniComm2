#include "port/module/deviceDiscovery.h"

#include <QCameraDevice>
#include <QMediaDevices>
#include <QScreen>
#include <QSerialPortInfo>

QStringList DeviceDiscovery::serialPorts() {
    QStringList devices{};
    for (const auto &port: QSerialPortInfo::availablePorts()) devices.append(port.portName());
    return devices;
}

QStringList DeviceDiscovery::screens() {
    QStringList devices{};
    for (const auto *screen: QGuiApplication::screens()) devices.append(screen->name());
    return devices;
}

QStringList DeviceDiscovery::cameras() {
    QStringList devices{};
    for (const auto &camera: QMediaDevices::videoInputs()) devices.append(camera.description());
    return devices;
}
