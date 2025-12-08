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

    void threadStart(const QUrl &scriptUrl, int mode, QString &threadId);

    void threadStart(const QString &scriptPath, int mode, QString &threadId);

    Q_INVOKABLE bool threadStop(const QString &threadId);

    bool threadWait(const QString &threadId);

    Q_INVOKABLE QString lifetimeCalc(int row) const;

signals:
    void appendLog(const QString &message, const QString &level);

    void insertMarker(const QUrl &scriptUrl, int type, int line, int time);

    void removeMarker(const QUrl &scriptUrl, int type, int line);

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
