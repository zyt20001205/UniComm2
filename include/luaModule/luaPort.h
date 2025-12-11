#ifndef UNICOMM_LUAPORT_H
#define UNICOMM_LUAPORT_H

#include <QObject>

namespace sol {
    struct this_state;
}

class LuaPort final : public QObject {
    Q_OBJECT

public:
    explicit LuaPort(QObject *parent = nullptr);

    ~LuaPort() override = default;

    std::vector<std::string> list();

    std::unordered_map<std::string, std::string> info(const std::string &portName);

    void open(const std::string &portName);

    void close(const std::string &portName);

signals:
    void listPort(std::vector<std::string> &portList);

    void infoPort(const QString &portName, std::unordered_map<std::string, std::string> &portInfo);

    void openPort(const QString &portName, bool &status);

    void closePort(const QString &portName, bool &status);
};

#endif //UNICOMM_LUAPORT_H
