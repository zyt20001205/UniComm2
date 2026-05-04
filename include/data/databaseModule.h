#ifndef UNICOMM_DATABASE_H
#define UNICOMM_DATABASE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QJsonArray>
#include <QStandardItemModel>

class DatabaseModel;
class QQuickWidget;
class QTableWidget;

class DatabaseModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit DatabaseModule();

    ~DatabaseModule() override;

    void propertySet(const QVariantMap &objects);

    static void databaseConfigSave();

    [[nodiscard]] QSet<QString> databaseList() const;

    Q_INVOKABLE void databaseInsert(int index, const QString &key);

    Q_INVOKABLE void databaseRemove(int index);

    Q_INVOKABLE void databaseRename(int index, const QString &key);

    Q_INVOKABLE void databaseSwap(int src, int dst);

    Q_INVOKABLE static void databaseClear(int index);

    [[nodiscard]] bool databaseWrite(const QString &key, const QString &value);

signals:
    void appendLog(int type, const QString &prefix, const QString &message);

private:
    void databaseIndex();

    QQuickWidget *m_widget{};
    QQuickItem *m_root{};
    QHash<QString, int> m_databaseHash{};
};

class DatabaseModel final : public QStandardItemModel {
    Q_OBJECT

public:
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
};

#endif //UNICOMM_DATABASE_H
