#ifndef DATABASE_H
#define DATABASE_H

#include <QDockWidget>
#include <QEvent>
#include <QHeaderView>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QKeyEvent>
#include <QMenu>
#include <QTableWidget>

extern QJsonObject g_config;

class Database final : public QDockWidget {
    Q_OBJECT

public:
    explicit Database(QWidget *parent = nullptr);

    ~Database() override = default;

    void databaseConfigSave() const;

    void databaseWrite(const QString &key, const QString &value) const;

    void databaseClear() const;

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void databaseRename(int visualIndex);

    void databaseInsert(int visualIndex);

    void databaseRemove(int visualIndex);

    QJsonArray m_databaseConfig = g_config["databaseConfig"].toArray();
    QTableWidget *m_tableWidget = nullptr;
};

#endif //DATABASE_H
