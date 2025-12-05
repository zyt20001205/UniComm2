#ifndef UNICOMM_THREADPOOL_H
#define UNICOMM_THREADPOOL_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QStandardItemModel;
class QTableWidget;
class QQuickWidget;

class LuaInterpreter;

class ThreadpoolModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit ThreadpoolModule();

    ~ThreadpoolModule() override = default;

    QString threadExec(const QString &scriptPath, const QString &mode);

    QString threadRun(const QUrl &scriptUrl, const QString &script);

    QString threadDebug(const QUrl &scriptUrl, const QString &script);

    Q_INVOKABLE bool threadStop(const QString &threadId);

    bool threadWait(const QString &threadId);

    Q_INVOKABLE QString lifetimeCalc(int row) const;

signals:
    void startDebug(const QString &threadId, LuaInterpreter *interpreter);

    void threadStopped(const QString &threadId);

private:
    void threadAppend(int status, const QString &name, const QString &threadId, QThread *worker);

    QHash<QString, QThread *> m_threadHash{};
    QQuickWidget *m_threadpoolWidget{};
    QStandardItemModel *m_threadpoolModel{};
    QString m_lifetime{};

    enum {
        ICON_COL,
        NAME_COL,
        SPAWN_COL,
        THREADID_COL
    };
};

#endif //UNICOMM_THREADPOOL_H
