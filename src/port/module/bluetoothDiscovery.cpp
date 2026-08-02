#include "port/module/bluetoothDiscovery.h"

#include <stdexcept>

// public
BluetoothDiscovery::BluetoothDiscovery(QObject *parent)
    : QObject(parent) {
}

void BluetoothDiscovery::adaptersRefresh() {
    emit busyChanged(true);
    emit statusUpdated("Loading adapters");
    try {
        m_adapters = SimpleBLE::Adapter::get_adapters();
        QVariantList adapters{};
        for (auto &adapter: m_adapters) {
            adapters.append(QVariantHash{
                {"name", QString::fromStdString(adapter.identifier())},
                {"address", QString::fromStdString(adapter.address())}
            });
        }
        emit adaptersUpdated(adapters);
        emit statusUpdated(QString("%1 adapter(s)").arg(adapters.size()));
    } catch (const std::exception &exception) {
        m_adapters.clear();
        emit statusUpdated(exception.what());
    }
    emit busyChanged(false);
}

void BluetoothDiscovery::scan(const QString &adapterAddress) {
    emit busyChanged(true);
    emit statusUpdated("Scanning");
    try {
        auto *adapter = adapterGet(adapterAddress);
        if (adapter == nullptr) throw std::runtime_error("adapter not found");
        adapter->scan_for(5000);
        m_peripherals = adapter->scan_get_results();
        QVariantList peripherals{};
        for (auto &peripheral: m_peripherals) {
            peripherals.append(QVariantHash{
                {"name", QString::fromStdString(peripheral.identifier())},
                {"address", QString::fromStdString(peripheral.address())},
                {"rssi", peripheral.rssi()}
            });
        }
        emit peripheralsUpdated(peripherals);
        emit statusUpdated(QString("%1 device(s)").arg(peripherals.size()));
    } catch (const std::exception &exception) {
        m_peripherals.clear();
        emit peripheralsUpdated({});
        emit statusUpdated(exception.what());
    }
    emit busyChanged(false);
}

void BluetoothDiscovery::discover(const QString &adapterAddress, const QString &peripheralAddress) {
    emit busyChanged(true);
    emit statusUpdated("Discovering services");
    SimpleBLE::Peripheral *selected{};
    bool disconnect{};
    try {
        if (adapterGet(adapterAddress) == nullptr) throw std::runtime_error("adapter not found");
        for (auto &peripheral: m_peripherals) {
            if (QString::fromStdString(peripheral.address()).compare(peripheralAddress, Qt::CaseInsensitive) == 0) {
                selected = &peripheral;
                break;
            }
        }
        if (selected == nullptr) throw std::runtime_error("peripheral not found");
        if (!selected->is_connected()) {
            selected->connect();
            disconnect = true;
        }

        QVariantList services{};
        for (auto &service: selected->services()) {
            QVariantList characteristics{};
            for (auto &characteristic: service.characteristics()) {
                characteristics.append(QVariantHash{
                    {"uuid", QString::fromStdString(characteristic.uuid())},
                    {"read", characteristic.can_read()},
                    {"writeRequest", characteristic.can_write_request()},
                    {"writeCommand", characteristic.can_write_command()},
                    {"notify", characteristic.can_notify()},
                    {"indicate", characteristic.can_indicate()}
                });
            }
            services.append(QVariantHash{
                {"uuid", QString::fromStdString(service.uuid())},
                {"characteristics", characteristics}
            });
        }
        if (disconnect) selected->disconnect();
        emit servicesUpdated(services);
        emit statusUpdated(QString("%1 service(s)").arg(services.size()));
    } catch (const std::exception &exception) {
        if (disconnect && selected) {
            try {
                selected->disconnect();
            } catch (...) {
            }
        }
        emit servicesUpdated({});
        emit statusUpdated(exception.what());
    }
    emit busyChanged(false);
}

// private
SimpleBLE::Adapter *BluetoothDiscovery::adapterGet(const QString &adapterAddress) {
    if (m_adapters.empty()) m_adapters = SimpleBLE::Adapter::get_adapters();
    for (auto &adapter: m_adapters) {
        if (QString::fromStdString(adapter.address()).compare(adapterAddress, Qt::CaseInsensitive) == 0) return &adapter;
    }
    return nullptr;
}
