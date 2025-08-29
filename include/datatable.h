#ifndef DATATABLE_H
#define DATATABLE_H

#include <QDockWidget>
#include <QEvent>
#include <QHeaderView>
#include <QInputDialog>
#include <QJsonArray>
#include <QKeyEvent>
#include <QMenu>
#include <QTableWidget>
#include "config.h"

class Datatable final : public QDockWidget {
    Q_OBJECT

public:
    explicit Datatable(QObject *parent = nullptr);

    ~Datatable() override = default;

    void datatableConfigSave() const;

    void datatableWrite(const QString &key, const QString &value);

signals:
    void appendLog(const QString &message, const QString &level);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void datatableRename(int visualColumn);

    void datatableInsert(int visualColumn);

    void datatableRemove(int visualColumn);

    QJsonArray m_datatableConfig = g_config["datatableConfig"].toArray();
    QTableWidget *m_tableWidget = nullptr;
    bool m_dragging = false;
    int m_srcIndex;
    int m_dstIndex;
};

#endif //DATATABLE_H
