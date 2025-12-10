#ifndef UNICOMM_LUAPORT_H
#define UNICOMM_LUAPORT_H

#include <QObject>

class LuaPort final : public QObject {
    Q_OBJECT

public:
    explicit LuaPort(QObject *parent = nullptr);

    ~LuaPort() override = default;

    std::vector<std::string> list();

signals:
    void listPort(std::vector<std::string> &portList);
};

#endif //UNICOMM_LUAPORT_H
