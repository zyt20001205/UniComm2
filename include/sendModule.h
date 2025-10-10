#ifndef UNICOMM_SENDMODULE_H
#define UNICOMM_SENDMODULE_H

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

class PortModule;

class SendModule final : public QDockWidget {
    Q_OBJECT

public:
    explicit SendModule(QWidget *parent = nullptr);

    ~SendModule() override = default;

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
    QLineEdit *m_lineEdit{};
    QTableWidget *m_tableWidget{};
};

#endif //UNICOMM_SENDMODULE_H
