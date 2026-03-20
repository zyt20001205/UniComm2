#ifndef UNICOMM_LUAPORT_H
#define UNICOMM_LUAPORT_H

#include <QObject>
#include <sol/object.hpp>

namespace sol {
    struct this_state;
}

class LuaPort final : public QObject {
    Q_OBJECT

public:
    explicit LuaPort(QObject *parent = nullptr);

    ~LuaPort() override = default;

    [[nodiscard]] static std::vector<std::string> list();

    [[nodiscard]] static std::unordered_map<std::string, std::string> info(const std::string &portName);

    static void open(const std::string &portName);

    static void close(const std::string &portName);

    static void clear(const std::string &portName);

    static void write(const std::string &portName, const std::string_view &data, const std::string &peerIp);

    [[nodiscard]] static sol::object read(sol::this_state ts, const std::string &portName, int length, int timeout, const std::string &peerIp);
};

#endif //UNICOMM_LUAPORT_H
