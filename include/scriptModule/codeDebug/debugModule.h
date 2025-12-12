#ifndef UNICOMM_DEBUG_H
#define UNICOMM_DEBUG_H

#include <QSortFilterProxyModel>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QQuickWidget;
class QStandardItemModel;
class QStringListModel;
class QTableView;
class QTabWidget;
class QTreeView;

class BreakpointsProxyModel;
class DebugPage;
class LuaInterpreter;

class DebugModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit DebugModule();

    ~DebugModule() override = default;

    void debugStart(const QString &threadId) const;

    void debugStop(const QString &threadId) const;

    Q_INVOKABLE void stateSet(const QString &threadId, int state);

    void callStackInsert(const QString &threadId, QStandardItemModel *callStackModel);

signals:
    void openScript(const QUrl &scriptUrl);

    void insertMarker(const QUrl &scriptUrl, int type, int line, int time);

    void setState(const QString &threadId, int state);

private:
    QQuickWidget *m_debugWidget{};
    QStringListModel *m_threadStringListModel{};
    QHash<QString, const QStandardItemModel *> m_callStackModelHash{};
};

#endif //UNICOMM_DEBUG_H
