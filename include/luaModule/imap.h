#ifndef UNICOMM_IMAP_H
#define UNICOMM_IMAP_H

#include <QObject>

class Imap final : public QObject {
    Q_OBJECT

public:
    explicit Imap(QObject *parent = nullptr);

    ~Imap() override = default;

    void login(const std::string &portName, const std::string &username, const std::string &password, int timeout);

    void select(const std::string &portName, const std::string &mailbox, int timeout);

    bool idle(const std::string &portName, int timeout);

private:
    [[nodiscard]] std::string parse(const QByteArray &command, const QByteArray &rxData);

    int m_count{};
};

#endif //UNICOMM_IMAP_H
