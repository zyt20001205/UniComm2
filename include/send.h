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

    QJsonArray m_sendConfig{};
    Port *m_port{};
    QLineEdit *m_lineEdit{};
    QTableWidget *m_tableWidget{};
};

#endif //SEND_H
