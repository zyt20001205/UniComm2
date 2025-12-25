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

    void write(const std::string &portName, const std::string_view &data, const std::string &peerIp);

    std::string read(const std::string &portName, int timeout, int length, const std::string &peerIp);

signals:
    void listPort(QSet<QString> &portSet);
};

#endif //UNICOMM_LUAPORT_H
