#ifndef UNICOMM_SENDMODULE_H
#define UNICOMM_SENDMODULE_H

#include <QJsonArray>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QLineEdit;
class QTableWidget;

class PortModule;

class SendModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit SendModule();

    ~SendModule() override = default;

    void sendConfigSave() const;

    static void commandSend(const QString &txText);

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
