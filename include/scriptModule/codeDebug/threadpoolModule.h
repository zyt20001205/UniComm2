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

    ~ThreadpoolModule() override;

    void propertySet(const QVariantMap &objects);

    void quit();

    void threadStart(const QUrl &scriptUrl, int mode, QString &threadId, int startLine = -1, int startCharacter = -1, int endLine = -1, int endCharacter = -1);

    Q_INVOKABLE void threadStart(const QUrl &scriptUrl, int mode, int startLine = -1, int startCharacter = -1, int endLine = -1, int endCharacter = -1);

    void threadStart(const QString &scriptPath, int mode, QString &threadId);

    Q_INVOKABLE void threadStop(const QString &threadId);

    Q_INVOKABLE [[nodiscard]] bool debugging() const;

    Q_INVOKABLE [[nodiscard]] QString lifetimeCalc(int row) const;

    void stateSet(const QString &threadId, int state);

    Q_INVOKABLE void valueSet(const QString &threadId, const QString &scriptUrl, const QString &expression, const QString &value, const QString &type);

signals:
    void trackQuit(float secondaryProgress, const QString &secondaryLog) const;

    void refreshThread(int run, int debug);

    void openScript(const QUrl &scriptUrl);

    void insertMarker(const QUrl &scriptUrl, int type, int line, int time);

    void removeMarker(const QUrl &scriptUrl, int type, int line);

    void insertCallStack(const QString &threadId, QStandardItemModel *callStackModel);

    void startDebug(const QString &threadId);

    void stopDebug(const QString &threadId);

    void appendLog(const QString &message, const QString &level);

    void listPort(QSet<QString> &portSet);

private:
    void threadAppend(int mode, const QString &name, const QString &threadId);

    void messageDialogNew(const QEventLoop *eventloop, const QString &threadId, const QString &text) const;

    QHash<QString, QThread *> m_threadHash{};
    QHash<QString, LuaInterpreter *> m_interpreterHash{};
    QQuickWidget *m_threadpoolWidget{};
    QQuickItem *m_mainItem{};
    QObject* m_errorDialog{};
    int m_run = 0;
    int m_debug = 0;
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
