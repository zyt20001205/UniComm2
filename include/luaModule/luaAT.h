#ifndef UNICOMM_LUAAT_H
#define UNICOMM_LUAAT_H

#include <QObject>

class LuaAT final : public QObject {
    Q_OBJECT

public:
    explicit LuaAT(QObject *parent = nullptr);

    ~LuaAT() override = default;

    void exec(const std::string &portName, const std::string &command, const std::string &peerIp);

    void read(const std::string &portName, const std::string &command, const std::string &peerIp);

    void set(const std::string &portName, const std::string &command, const std::string &value, const std::string &peerIp);

    void test(const std::string &portName, const std::string &command, const std::string &peerIp);
};

#endif //UNICOMM_LUAAT_H