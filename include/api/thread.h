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

    std::string start(sol::this_state ts, const std::string &scriptPath);

    void stop(const std::string &threadId);

    void sleep(int ms);

signals:
    void startThread(const QString &scriptPath, int mode, QString &threadId);

    void stopThread(const QString &threadId);
};

#endif //UNICOMM_THREAD_H
