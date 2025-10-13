#ifndef UNICOMM_DATABASE_H
#define UNICOMM_DATABASE_H

#include <QJsonArray>
#include "kddockwidgets/qtwidgets/views/DockWidget.h"

class QTableWidget;

class DatabaseModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit DatabaseModule();

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
