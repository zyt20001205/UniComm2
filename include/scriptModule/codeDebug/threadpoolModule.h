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

    void propertySet(const QVariantMap &objects);

    void threadStart(const QUrl &scriptUrl, int mode, QString &threadId);

    void threadStart(const QString &scriptPath, int mode, QString &threadId);

    Q_INVOKABLE void threadStop(const QString &threadId);

    Q_INVOKABLE QString lifetimeCalc(int row) const;

    void stateSet(const QString &threadId, int state);

signals:
    void openScript(const QUrl &scriptUrl);

    void insertMarker(const QUrl &scriptUrl, int type, int line, int time);

    void removeMarker(const QUrl &scriptUrl, int type, int line);

    void insertCallStack(const QString &threadId, QStandardItemModel *callStackModel);

    void startDebug(const QString &threadId);

    void stopDebug(const QString &threadId);

    void appendLog(const QString &message, const QString &level);

    void listPort(std::vector<std::string> &portList);

private:
    void threadAppend(int status, const QString &name, const QString &threadId);

    QHash<QString, QThread *> m_threadHash{};
    QHash<QString, LuaInterpreter *> m_interpreterHash{};
    QQuickWidget *m_threadpoolWidget{};
    QStandardItemModel *m_threadpoolStandardItemModel{};
    QString m_lifetime{};

    enum {
        ICON_COL,
        NAME_COL,
        SPAWN_COL,
        THREADID_COL
    };
};

#endif //UNICOMM_THREADPOOL_H
