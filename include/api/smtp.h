#ifndef UNICOMM_SMTP_H
#define UNICOMM_SMTP_H

#include <QObject>
#include <sol/object.hpp>

class Smtp final : public QObject {
    Q_OBJECT

public:
    explicit Smtp(QObject *parent = nullptr);

    ~Smtp() override = default;

    static void authLogin(const std::string &portName, const std::string &username, const std::string &password, int timeout);

    static void ehlo(const std::string &portName, int timeout);

    static void send(const std::string &portName, const std::string &from, const sol::object &to, const sol::object &cc, const sol::object &bcc, const std::string &subject,
                     const std::string &body, const std::string &attachment, int timeout);

    static void quit(const std::string &portName);

private:
    [[nodiscard]] static QString parser(const QByteArray &rxData);
};

#endif //UNICOMM_SMTP_H
