#ifndef DATABASE_H
#define DATABASE_H

#include <QDockWidget>
#include <QEvent>
#include <QHeaderView>
#include <QInputDialog>
#include <QJsonArray>
#include <QKeyEvent>
#include <QMenu>
#include <QTableWidget>
#include "config.h"

class Database final : public QDockWidget {
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);

    ~Database() override = default;

    void databaseConfigSave() const;

    bool databaseWrite(const QString &key, const QString &value) const;

    void databaseClear() const;

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void databaseRename(int visualRow);

    void databaseInsert(int visualRow);

    void databaseRemove(int visualRow);

    QJsonArray m_databaseConfig = g_config["databaseConfig"].toArray();
    QTableWidget *m_tableWidget = nullptr;
    bool m_dragging = false;
    int m_srcIndex{};
    int m_dstIndex{};
};

#endif //DATABASE_H
