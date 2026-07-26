#ifndef UNICOMM_IMAP_H
#define UNICOMM_IMAP_H

#include <QObject>
#include <QVariant>
#include <sol/object.hpp>
#include <variant>

class BasePort;

class Imap final : public QObject {
    Q_OBJECT

public:
    explicit Imap(QObject *parent = nullptr);

    ~Imap() override = default;

    void init(const std::string &portName, int timeout);

    void login(const std::string &username, const std::string &password);

    [[nodiscard]] sol::object receive(sol::this_state ts, const sol::optional<std::string> &from,
                                      const sol::optional<std::string> &path, sol::optional<int> timeout);

    void logout();

signals:
    void appendLog(int type, const QString &prefix, const QString &message);

private:
    struct Untagged {
        QByteArray value{};
    };

    struct Continuation {
        QByteArray value{};
    };

    struct Tagged {
        QByteArray tag{};
        QByteArray code{};
        QByteArray text{};
    };

    struct Result {
        std::variant<std::monostate, Untagged, Continuation, Tagged> value{};
        QString exception{};
    };

    [[nodiscard]] static Result parser(const QByteArray &rxData);

    [[nodiscard]] QByteArray nextTag();

    [[nodiscard]] static QVariantHash mailParser(const QByteArray &rxData);

    [[nodiscard]] QByteArray rfc2047Parser(const QByteArray &text);

    std::string m_portName{};
    int m_timeout{};
    BasePort *m_port{};
    int m_count{};
};

#endif //UNICOMM_IMAP_H
