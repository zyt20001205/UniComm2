#ifndef UNICOMM_SCREEN_H
#define UNICOMM_SCREEN_H

#include <QJsonArray>
#include <QJsonObject>

#include "portModule/basePort.h"

class QScreen;

class Screen final : public BasePort {
    Q_OBJECT

public:
    explicit Screen(const QJsonObject &portConfig, QObject *parent = nullptr);

    void reload(const QJsonObject &portConfig) override;

    bool open() override;

    void close() override;

    std::unordered_map<std::string, std::string> info() override;

    QByteArray read(int timeout, int length, const QString &rxFormat) override;

private:
    QScreen *m_screen{};
    // port config
    QString m_portName{};
    QString m_charset{};
    QJsonObject m_process{};
    QJsonArray m_areaList{};
    //
    bool m_showPreview = false;
};

#endif //UNICOMM_SCREEN_H
