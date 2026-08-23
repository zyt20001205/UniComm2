#ifndef UNICOMM_DATABASE_H
#define UNICOMM_DATABASE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QJsonArray>
#include <QStandardItemModel>

class DatabaseModel;
class QQuickWidget;
class QTableWidget;
class ToastModule;

class DatabaseModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit DatabaseModule();

    ~DatabaseModule() override;

    void propertySet(const QVariantHash &objects);

    static void databaseConfigSave();

    [[nodiscard]] QSet<QString> databaseList() const;

    Q_INVOKABLE int databaseInsert(int index, const QString &key = QString());

    Q_INVOKABLE void databaseRemove(int index);

    [[nodiscard]] Q_INVOKABLE bool databaseRename(int index, const QString &key);

    Q_INVOKABLE void databaseSwap(int src, int dst);

    Q_INVOKABLE static void databaseClear(int index);

    [[nodiscard]] bool databaseWrite(const QString &key, const QString &value);

signals:
    void appendLog(int type, const QString &prefix, const QString &message);

private:
    [[nodiscard]] bool _databaseInsert(int index, const QString &key);

    [[nodiscard]] bool _databaseRemove(const QString &key);

    [[nodiscard]] bool _databaseRename(const QString &oldKey, const QString &newKey);

    void databaseCache();

    QQuickWidget *m_widget{};
    QObject *m_root{};
    ToastModule *m_toast{};
    QHash<QString, int> m_databaseHash{};
};

class DatabaseModel final : public QStandardItemModel {
    Q_OBJECT
    Q_PROPERTY(bool empty READ emptyGet NOTIFY emptyChanged)

public:
    explicit DatabaseModel(QObject *parent = nullptr);

    enum Role {
        KeyRole = Qt::UserRole + 1
    };

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    [[nodiscard]] bool emptyGet() const {
        return rowCount() == 0;
    }

signals:
    void emptyChanged();
};

#endif //UNICOMM_DATABASE_H
