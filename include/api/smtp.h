#ifndef UNICOMM_SMTP_H
#define UNICOMM_SMTP_H

#include <QObject>
#include <sol/object.hpp>

class BasePort;

class Smtp final : public QObject {
    Q_OBJECT

public:
    explicit Smtp(QObject *parent = nullptr);

    ~Smtp() override = default;

    void init(const std::string &portName, int timeout);

    void authLogin(const std::string &username, const std::string &password) const;

    void ehlo() const;

    void _send(const std::string &from, const sol::object &to, const sol::object &cc, const sol::object &bcc, const std::string &subject, const std::string &body,
              const std::string &attachment) const;

    void send(const std::string &from, const sol::object &to, const sol::object &cc, const sol::object &bcc, const std::string &subject, const std::string &body,
              const sol::optional<std::string> &attachment) const;

    void quit() const;

private:
    [[nodiscard]] static QString parser(const QByteArray &rxData);

    std::string m_portName{};
    int m_timeout{};
    BasePort *m_port{};
};

#endif //UNICOMM_SMTP_H
