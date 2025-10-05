#ifndef SEND_H
#define SEND_H

#include <QDockWidget>
#include <QEvent>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

extern QJsonObject g_config;

class Port;

class Send final : public QDockWidget {
    Q_OBJECT

public:
    explicit Send(QWidget *parent = nullptr);

    ~Send() override = default;

    void setPort(Port *port) { m_port = port; }

    void sendConfigSave() const;

    void commandSend(const QString &txText) const;

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void shortcutRename(int visualIndex, int type);

    void shortcutInsert(int visualIndex);

    void shortcutRemove(int visualIndex);

    QJsonArray m_sendConfig = g_config["sendConfig"].toArray();
    Port *m_port = nullptr;
    QLineEdit *m_lineEdit = nullptr;
    QTableWidget *m_tableWidget = nullptr;
};

#endif //SEND_H
