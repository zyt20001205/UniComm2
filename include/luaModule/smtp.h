#ifndef UNICOMM_SMTP_H
#define UNICOMM_SMTP_H

#include <QObject>

class Smtp final : public QObject {
    Q_OBJECT

public:
    explicit Smtp(QObject *parent = nullptr);

    ~Smtp() override = default;

    static void ehlo(const std::string &portName);

    static void authLogin(const std::string &portName, const std::string &username, const std::string &password);

    static void mail(const std::string &portName, const std::string &from, const std::string &to, const std::string &subject, const std::string &body, const std::string &attachment);

private:
    static std::string parse(const QByteArray &rxData);
};

#endif //UNICOMM_SMTP_H