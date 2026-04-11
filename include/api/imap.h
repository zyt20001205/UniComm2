#ifndef UNICOMM_IMAP_H
#define UNICOMM_IMAP_H

#include <QObject>
#include <sol/table.hpp>

class Imap final : public QObject {
    Q_OBJECT

public:
    explicit Imap(QObject *parent = nullptr);

    ~Imap() override = default;

    void login(const std::string &portName, const std::string &username, const std::string &password, int timeout);

    void select(const std::string &portName, const std::string &mailbox, int timeout);

    sol::table fetch(const std::string &portName, int timeout);

    int idle(const std::string &portName, int timeout);

private:
    [[nodiscard]] QVariantHash parser(const QByteArray &command, const QByteArray &rxData);

    static QString continuationParser(const QByteArray &command, const QByteArray &rxData);

    [[nodiscard]] QString taggedParser(const QByteArray &command, const QByteArray &rxData);

    static QVariantHash untaggedParser(const QByteArray &command, const QByteArray &rxData);

    int m_count{};
};

#endif //UNICOMM_IMAP_H
