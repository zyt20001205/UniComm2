#ifndef UNICOMM_DATABASE_H
#define UNICOMM_DATABASE_H

#include <QJsonArray>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QStandardItemModel;
class QQuickWidget;
class QTableWidget;

class DatabaseModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit DatabaseModule();

    ~DatabaseModule() override;

    void propertySet(const QVariantMap &objects);

    void databaseConfigSave() const;

    void databaseList(QSet<QString> &databaseList) const;

    Q_INVOKABLE void databaseInsert(int index, const QString &key);

    Q_INVOKABLE void databaseRemove(int index);

    Q_INVOKABLE void databaseRename(int index, const QString &key);

    Q_INVOKABLE void databaseSwap(int src, int dst);

    Q_INVOKABLE void databaseClear(int index);

    void databaseWrite(const QString &key, const QString &value, bool &status);

signals:
    void appendLog(const QString &message, const QString &level);

private:
    void databaseIndex();

    QQuickWidget *m_databaseWidget{};
    QQuickItem *m_rootItem{};
    QHash<QString, int> m_databaseHash{};
};

#endif //UNICOMM_DATABASE_H
