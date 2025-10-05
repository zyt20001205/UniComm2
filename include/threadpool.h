#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <QDockWidget>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QHeaderView>
#include <QJsonObject>
#include <QMessageBox>
#include <QTableWidget>
#include <QThread>
#include <QUrl>

extern QJsonObject g_config;

class LuaInterpreter;

class Threadpool final : public QDockWidget {
    Q_OBJECT

public:
    explicit Threadpool(QWidget *parent = nullptr);

    ~Threadpool() override = default;

    void workspaceOpen(const QUrl &rootUrl);

    QString threadExec(const QString &scriptPath);

    QString threadRun(const QUrl &scriptUrl, const QString &script);

    void threadDebug(const QUrl &scriptUrl, const QString &script, QHash<QUrl, QSet<int> > *breakpoints);

    bool threadStop(const QString &threadId);

    bool threadWait(const QString &threadId);

signals:
    void startDebug(const QString &threadId, LuaInterpreter *interpreter);

    void threadStopped(const QString &threadId);

private:
    void threadAppend(int status, const QString &name, const QString &threadId, QThread *worker);

    QUrl m_rootUrl{};
    QTableWidget *m_threadpoolTableWidget{};
    QHash<int, QColor> m_threadpoolColor{};
    QHash<QString, QThread *> m_threadHash{};

    enum {
        THREAD_RUN,
        THREAD_DEBUG,
        THREAD_STOP
    };

    enum {
        DEBUG_RUN,
        DEBUG_PAUSE,
        DEBUG_TERMINATE,
        DEBUG_STEPOVER,
        DEBUG_STEPINTO,
        DEBUG_STEPOUT
    };
};

#endif //THREADPOOL_H
