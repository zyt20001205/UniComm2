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

    Q_INVOKABLE [[nodiscard]] QString databaseInsert(const QString &key = {}, const QString &targetKey = {}, const QString &undoGroupId = {});

    Q_INVOKABLE [[nodiscard]] QString databaseRemove(const QString &key, const QString &undoGroupId = {});

    Q_INVOKABLE [[nodiscard]] QString databaseRename(const QString &key, const QString &newKey, const QString &undoGroupId = {});

    Q_INVOKABLE [[nodiscard]] QString databaseMove(const QString &key, const QString &targetKey, const QString &undoGroupId = {});

    Q_INVOKABLE void databaseClear(const QString &key = {});

    [[nodiscard]] bool databaseWrite(const QString &key, const QString &value);

signals:
    void appendLog(int type, const QString &prefix, const QString &message);

private:
    void _databaseInsert(int index, const QString &key);

    void _databaseRemove(const QString &key);

    void _databaseRename(const QString &oldKey, const QString &newKey);

    void _databaseMove(int src, int dst);

    int databaseIndex(const QString &key) const;

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

    [[nodiscard]] bool emptyGet() const {
        return rowCount() == 0;
    }

signals:
    void emptyChanged();
};

#endif //UNICOMM_DATABASE_H
