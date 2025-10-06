#ifndef DEBUG_H
#define DEBUG_H

#include <QComboBox>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <qtableview.h>
#include <QVBoxLayout>

class LuaInterpreter;

class BreakpointsProxyModel;

class Debug final : public QDockWidget {
    Q_OBJECT

public:
    explicit Debug(QWidget *parent = nullptr);

    ~Debug() override = default;

    void breakpointInsert(const QUrl &scriptUrl, int line) const;

    void breakpointRemove(const QUrl &scriptUrl, int line) const;

    void debugStart(const QString &threadId, LuaInterpreter *interpreter);

    void debugEnd(const QString &threadId);

signals:
    void resume(const QString &threadId);

private:
    QHash<QString, LuaInterpreter *> m_interpreterHash{};
    QComboBox *m_debugThreadCombobox{};
    QStandardItemModel *m_debugBreakpointsTableModel{};
    BreakpointsProxyModel *m_debugBreakpointsProxyModel{};
    QTableView *m_debugBreakpointsTableView{};

    enum {
        DEBUG_RUN,
        DEBUG_PAUSE,
        DEBUG_STEPOVER,
        DEBUG_STEPINTO,
        DEBUG_STEPOUT
    };
};

class BreakpointsProxyModel final : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit BreakpointsProxyModel(QObject *parent = nullptr);

    ~BreakpointsProxyModel() override = default;

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};

#endif //DEBUG_H
