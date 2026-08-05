#ifndef UNICOMM_DEVICEDISCOVERY_H
#define UNICOMM_DEVICEDISCOVERY_H

#include <QStringList>

namespace DeviceDiscovery {
[[nodiscard]] QStringList serialPorts();

[[nodiscard]] QStringList screens();

[[nodiscard]] QStringList cameras();
}

#endif //UNICOMM_DEVICEDISCOVERY_H
