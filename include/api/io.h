#ifndef UNICOMM_IO_H
#define UNICOMM_IO_H

#include <QObject>

class QEventLoop;

namespace sol {
    struct variadic_args;
}

class IO final : public QObject {
    Q_OBJECT

public:
    explicit IO(QObject *parent = nullptr);

    ~IO() override = default;

    void log(const sol::variadic_args &args);

    void message(const std::string &text) const;

    static void speak(const std::string &text);

signals:
    void appendLog(const QString &message, int type);

    void newMessageDialog(const QEventLoop *eventloop, const QString &text) const;
};

#endif //UNICOMM_IO_H
