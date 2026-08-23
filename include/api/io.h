#ifndef UNICOMM_IO_H
#define UNICOMM_IO_H

#include <cstdio>

#include <QObject>

class QEventLoop;
class QThread;

struct lua_State;

namespace sol {
    struct variadic_args;
}

class IO final : public QObject {
    Q_OBJECT

public:
    explicit IO(const QString &threadId, QObject *parent = nullptr);

    ~IO() override;

    void redirect(lua_State *L);

    void print(const sol::variadic_args &args) const;

    void message(const std::string &text) const;

    static void speak(const std::string &text);

signals:
    void writeTerminal(const QString &id, const QByteArray &data);

    void newMessageDialog(const QEventLoop *eventloop, const QString &text) const;

private:
    [[nodiscard]] static std::FILE *fileOpen(void *&handle);

    static void handleClose(void *&handle);

    QString m_threadId{};
    void *m_outputRead{};
    void *m_stdout{};
    void *m_stderr{};
    QThread *m_outputThread{};
};

#endif //UNICOMM_IO_H
