#ifndef SEND_H
#define SEND_H

#include <QDockWidget>
#include <QEvent>
#include <QHBoxLayout>
#include <QHeaderView>
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

    void commandSend(const QString &command);

signals:
    void writePort(int index, const QString &command, const QString &peerIp);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void shortcutRename(int row, int column);

    void shortcutInsert(int index);

    void shortcutRemove(int index);

    QJsonArray m_sendConfig = g_config["sendConfig"].toArray();

    QLineEdit *m_lineEdit = nullptr;
    QTableWidget *m_tableWidget = nullptr;
    int m_previousIndex = -1;
    int m_currentIndex;
    QString m_sourceKey;
    QString m_sourceValue;
    int m_sourceIndex;
};

#endif //SEND_H
