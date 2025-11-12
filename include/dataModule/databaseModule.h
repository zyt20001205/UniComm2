#ifndef UNICOMM_DATABASE_H
#define UNICOMM_DATABASE_H

#include <QJsonArray>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QTableWidget;

class DatabaseModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit DatabaseModule();

    ~DatabaseModule() override = default;

    void workspaceOpen();

    void databaseConfigSave() const;

    QVariantList databaseList() const;

    void databaseInsert(int visualIndex, QString key = QString());

    bool databaseWrite(const QString &key, const QString &value) const;

    void databaseClear() const;

    QHash<QString, int> m_databaseHash{};

signals:
    void appendLog(const QString &message, const QString &level);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void databaseRemove(int visualIndex);

    void databaseRename(int visualIndex);

    void databaseSwap(int logicalIndex, int oldVisualIndex, int newVisualIndex);

    void databaseAnnotate() const;

    QJsonArray m_databaseConfig{};
    QTableWidget *m_tableWidget{};
    int m_version = 1;
};

#endif //UNICOMM_DATABASE_H
