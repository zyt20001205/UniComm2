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

    std::string_view read(const std::string &portName, int timeout, int length, const std::string &peerIp);

signals:
    void listPort(std::vector<std::string> &portList);

    void infoPort(const QString &portName, std::unordered_map<std::string, std::string> &portInfo);

    void openPort(const QString &portName, bool &status);

    void closePort(const QString &portName, bool &status);

    void writePort(const QString &portName, const QByteArray &txData, const QString &peerIp, bool &status);

    void readPort(const QString &portName, int timeout, int length, const QString &peerIp, bool &status, QByteArray &rxData);
};

#endif //UNICOMM_LUAPORT_H
