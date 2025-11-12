#ifndef UNICOMM_DEBUG_H
#define UNICOMM_DEBUG_H

#include <QSortFilterProxyModel>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QStandardItemModel;
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

    void breakpointInsert(const QUrl &scriptUrl, int line) const;

    void breakpointRemove(const QUrl &scriptUrl, int line) const;

    void breakpointClear() const;

    void debugStart(const QString &threadId, LuaInterpreter *interpreter);

    void debugEnd(const QString &threadId, const DebugPage *debugPage);

    void varReturn(const QString &threadId, QStandardItemModel *varTree);

    void callReturn(const QString &threadId, QStandardItemModel *callTable);

signals:
    void resume(const QString &threadId);

    void openScript(const QUrl &scriptUrl);

    void insertMarker(const QUrl &scriptUrl, int type, int line, int time);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void overlayShow() const;

    void overlayHide() const;

    void overlayResize() const;

    QHash<QString, LuaInterpreter *> m_interpreterHash{};
    QStandardItemModel *m_debugBreakpointsTableModel{};
    BreakpointsProxyModel *m_debugBreakpointsProxyModel{};
    QTableView *m_debugBreakpointsTableView{};
    QTabWidget *m_debugTabWidget{};
    QWidget *m_debugTabOverlay{};
    QHash<QString, DebugPage *> m_debugPageHash{};
};

class BreakpointsProxyModel final : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit BreakpointsProxyModel(QObject *parent = nullptr);

    ~BreakpointsProxyModel() override = default;

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};

class DebugPage final : public QWidget {
    Q_OBJECT

public:
    explicit DebugPage(LuaInterpreter *interpreter, QWidget *parent = nullptr);

    ~DebugPage() override = default;

    void varLoad(QStandardItemModel *varTree) const;

    void callLoad(QStandardItemModel *callTable) const;

signals:
    void openScript(const QUrl &scriptUrl);

    void insertMarker(const QUrl &scriptUrl, int type, int line, int time);

private:
    LuaInterpreter *m_interpreter{};
    QTreeView *m_varTreeView{};
    QTableView *m_callTableView{};
};

#endif //UNICOMM_DEBUG_H
