#ifndef UNICOMM_THREADPOOL_H
#define UNICOMM_THREADPOOL_H

#include <QUrl>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QTableWidget;

class LuaInterpreter;

class ThreadpoolModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit ThreadpoolModule();

    ~ThreadpoolModule() override = default;

    QString threadExec(const QString &scriptPath, const QString &mode);

    QString threadRun(const QUrl &scriptUrl, const QString &script);

    QString threadDebug(const QUrl &scriptUrl, const QString &script);

    bool threadStop(const QString &threadId);

    bool threadWait(const QString &threadId);

signals:
    void startDebug(const QString &threadId, LuaInterpreter *interpreter);

    void threadStopped(const QString &threadId);

private:
    void threadAppend(int status, const QString &name, const QString &threadId, QThread *worker);

    void timeRefresh() const;

    QTimer *m_runtimeTimer{};
    QTableWidget *m_threadpoolTableWidget{};
    QHash<int, QColor> m_threadpoolColor{};
    QHash<QString, QThread *> m_threadHash{};

    enum {
        STATUS_COL,
        TIMER_COL,
        SOURCE_COL,
        THREAD_COL,
        STOP_COL
    };
};

#endif //UNICOMM_THREADPOOL_H
