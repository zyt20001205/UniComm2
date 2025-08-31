#ifndef SEND_H
#define SEND_H

#include <QDockWidget>
#include <QEvent>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>
#include "config.h"

class Send final : public QDockWidget {
    Q_OBJECT

public:
    explicit Send(QObject *parent = nullptr);

    ~Send() override = default;

    void sendConfigSave() const;

    void commandSend(const QString &txText);

signals:
    void writeTextPort(int index, const QString &txText);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void shortcutRename(int visualRow, int column);

    void shortcutInsert(int visualRow);

    void shortcutRemove(int visualRow);

    QJsonArray m_sendConfig = g_config["sendConfig"].toArray();
    QLineEdit *m_lineEdit = nullptr;
    QTableWidget *m_tableWidget = nullptr;
};

#endif //SEND_H
