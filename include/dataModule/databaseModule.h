#ifndef UNICOMM_DATABASE_H
#define UNICOMM_DATABASE_H

#include <QDockWidget>
#include <QJsonArray>

class QTableWidget;

class DatabaseModule final : public QDockWidget {
    Q_OBJECT

public:
    explicit DatabaseModule(QWidget *parent = nullptr);

    ~DatabaseModule() override = default;

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

    QJsonArray m_databaseConfig{};
    QTableWidget *m_tableWidget{};
};

#endif //UNICOMM_DATABASE_H
