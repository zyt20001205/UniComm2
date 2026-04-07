#ifndef UNICOMM_IMAP_H
#define UNICOMM_IMAP_H

#include <QObject>

class Imap final : public QObject {
    Q_OBJECT

public:
    explicit Imap(QObject *parent = nullptr);

    ~Imap() override = default;

    void login(const std::string &portName, const std::string &username, const std::string &password, int timeout);

private:
    [[nodiscard]] std::string parse(const QByteArray &command, const QByteArray &rxData) const;

    int m_count{};
};

#endif //UNICOMM_IMAP_H
