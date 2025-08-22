#ifndef DATABASE_H
#define DATABASE_H

#include <QDockWidget>
#include <QEvent>
#include <QHeaderView>
#include <QJsonArray>
#include <QKeyEvent>
#include <QTableWidget>
#include "config.h"

class Database final : public QDockWidget {
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);

    ~Database() override = default;

    void databaseConfigSave() const;

    void databaseWrite(const QString &key, const QString &value);

signals:
    void appendLog(const QString &message, const QString &level);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void databaseRename(int index);

    void databaseInsert(int index);

    void databaseRemove(int index);

    QJsonArray m_databaseConfig = g_config["databaseConfig"].toArray();
    QTableWidget *m_tableWidget = nullptr;
    int m_previousIndex = -1;
    int m_currentIndex;
    QString m_sourceKey;
    int m_sourceIndex;
};

#endif //DATABASE_H
