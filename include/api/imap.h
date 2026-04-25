#ifndef UNICOMM_IMAP_H
#define UNICOMM_IMAP_H

#include <QObject>
#include <sol/object.hpp>

class BasePort;

class Imap final : public QObject {
    Q_OBJECT

public:
    explicit Imap(QObject *parent = nullptr);

    ~Imap() override = default;

    void init(const std::string &portName, int timeout);

    [[nodiscard]] int _idle(int timeout);

    [[nodiscard]] int idle(sol::optional<int> timeout);

    void login(const std::string &username, const std::string &password);

    void select(const std::string &mailbox);

    [[nodiscard]] sol::object fetch(sol::this_state ts, int sequenceNumber);

    void _receive(const std::string &from, const std::string &path, int timeout);

    void receive(const sol::optional<std::string> &from, const sol::optional<std::string> &path, sol::optional<int> timeout);

signals:
    void appendLog(int type, const QString &prefix, const QString &message);

private:
    [[nodiscard]] QVariantHash parser(const QByteArray &command, const QByteArray &rxData);

    [[nodiscard]] static QString continuationParser(const QByteArray &command, const QByteArray &rxData);

    [[nodiscard]] QString taggedParser(const QByteArray &command, const QByteArray &rxData);

    [[nodiscard]] static QVariantHash untaggedParser(const QByteArray &command, const QByteArray &rxData);

    [[nodiscard]] static QVariantHash fetchParser(const QByteArray &rxData);

    [[nodiscard]] QByteArray rfc2047Parser(const QByteArray &text);

    std::string m_portName{};
    int m_timeout{};
    BasePort *m_port{};
    int m_count{};
};

#endif //UNICOMM_IMAP_H
