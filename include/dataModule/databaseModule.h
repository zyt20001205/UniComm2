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

    void workspaceOpen(const QUrl &rootUrl);

    void databaseConfigSave() const;

    bool databaseWrite(const QString &key, const QString &value) const;

    void databaseClear() const;

    QHash<QString, int> m_databaseHash{};

signals:
    void appendLog(const QString &message, const QString &level);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void databaseInsert(int visualIndex, QString key = QString());

    void databaseRemove(int visualIndex);

    void databaseRename(int visualIndex);

    void databaseSwap(int logicalIndex, int oldVisualIndex, int newVisualIndex);

    void databaseAnnotate() const;

    QJsonArray m_databaseConfig{};
    QTableWidget *m_tableWidget{};
    QUrl m_annotationUrl{};
    int m_version = 1;
};

#endif //UNICOMM_DATABASE_H
