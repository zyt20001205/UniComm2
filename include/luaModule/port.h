#ifndef UNICOMM_PORT_H
#define UNICOMM_PORT_H

#include <QObject>
#include <sol/object.hpp>

class Port final : public QObject {
    Q_OBJECT

public:
    explicit Port(QObject *parent = nullptr);

    ~Port() override = default;

    [[nodiscard]] static sol::table list(sol::this_state ts);

    [[nodiscard]] static sol::object info(sol::this_state ts, const std::string &portName);

    static void open(const std::string &portName);

    static void close(const std::string &portName);

    static void clear(const std::string &portName);

    static void write(const std::string &portName, const std::string_view &data, const std::string &peerIp);

    [[nodiscard]] static sol::object read(sol::this_state ts, const std::string &portName, int length, int timeout, const std::string &peerIp);
};

#endif //UNICOMM_PORT_H
