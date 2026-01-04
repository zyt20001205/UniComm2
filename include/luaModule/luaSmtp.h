#ifndef UNICOMM_LUASMTP_H
#define UNICOMM_LUASMTP_H

#include <QObject>

class LuaSMTP final : public QObject {
    Q_OBJECT

public:
    explicit LuaSMTP(QObject *parent = nullptr);

    ~LuaSMTP() override = default;

    void ehlo(const std::string &portName);

    void authLogin(const std::string &portName, const std::string &username, const std::string &password);

    void mail(const std::string &portName, const std::string &from, const std::string &to, const std::string &subject, const std::string &body);
};

#endif //UNICOMM_LUASMTP_H