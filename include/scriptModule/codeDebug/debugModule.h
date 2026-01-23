#ifndef UNICOMM_DEBUG_H
#define UNICOMM_DEBUG_H

#include <QSortFilterProxyModel>
#include <qstandarditemmodel.h>
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

    ~DebugModule() override;

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void debugStart(const QString &threadId) const;

    void debugStop(const QString &threadId);

    Q_INVOKABLE void stateSet(const QString &threadId, int state);

    Q_INVOKABLE QString threadGet() const;

    void callStackInsert(const QString &threadId, QStandardItemModel *callStackModel);

    Q_INVOKABLE void callStackSwitch(const QString &threadId) const;

    Q_INVOKABLE void markerInsert(const QVariantHash &position);

signals:
    void openScript(const QUrl &scriptUrl);

    void getCursorPosition();

    void insertMarker(const QUrl &scriptUrl, int type, int line, int time);

    void setState(const QString &threadId, int state);

private:
    QQuickWidget *m_debugWidget{};
    QObject *m_errorDialog{};
    QObject *m_threadComboBox{};
    QStringListModel *m_threadStringListModel{};
    QHash<QString, QStandardItemModel *> m_callStackModelHash{};
};

#endif //UNICOMM_DEBUG_H
