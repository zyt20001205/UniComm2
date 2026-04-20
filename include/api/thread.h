#ifndef UNICOMM_THREAD_H
#define UNICOMM_THREAD_H

#include <QObject>

namespace sol {
    struct this_state;
}

class Thread final : public QObject {
    Q_OBJECT

public:
    explicit Thread(QObject *parent = nullptr);

    ~Thread() override = default;

    [[nodiscard]] std::string start(sol::this_state ts, const std::string &documentPath);

    void stop(const std::string &threadId);

    void sleep(int ms);

signals:
    void startThread(const QUrl &documentUrl, int mode, QString &threadId, int startLine, int startCharacter, int endLine, int endCharacter);

    void stopThread(const QString &threadId);
};

#endif //UNICOMM_THREAD_H
