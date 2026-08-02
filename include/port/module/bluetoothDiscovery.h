#ifndef UNICOMM_BLUETOOTHDISCOVERY_H
#define UNICOMM_BLUETOOTHDISCOVERY_H

#include <QObject>
#include <QVariantList>
#include <simpleble/SimpleBLE.h>

class BluetoothDiscovery final : public QObject {
    Q_OBJECT

public:
    explicit BluetoothDiscovery(QObject *parent = nullptr);

    ~BluetoothDiscovery() override = default;

    void adaptersRefresh();

    void scan(const QString &adapterAddress);

    void discover(const QString &adapterAddress, const QString &peripheralAddress);

signals:
    void adaptersUpdated(const QVariantList &adapters);

    void peripheralsUpdated(const QVariantList &peripherals);

    void servicesUpdated(const QVariantList &services);

    void statusUpdated(const QString &status);

    void busyChanged(bool busy);

private:
    [[nodiscard]] SimpleBLE::Adapter *adapterGet(const QString &adapterAddress);

    std::vector<SimpleBLE::Adapter> m_adapters{};
    std::vector<SimpleBLE::Peripheral> m_peripherals{};
};

#endif //UNICOMM_BLUETOOTHDISCOVERY_H
