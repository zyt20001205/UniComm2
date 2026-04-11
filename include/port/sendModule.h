#ifndef UNICOMM_SENDMODULE_H
#define UNICOMM_SENDMODULE_H

#include <QJsonArray>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QLineEdit;
class QQuickWidget;
class QTableWidget;

class PortModule;

class SendModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit SendModule();

    ~SendModule() override;

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void sendConfigSave() const;

    Q_INVOKABLE void configLoad() const;

    Q_INVOKABLE void commandSend() const;

private:

    QJsonArray m_sendConfig{};
    QQuickWidget *m_sendWidget{};
    QObject *m_nameComboBox{};
    QObject *m_overrideSwitch{};
    QObject *m_formatComboBox{};
    QObject *m_suffixComboBox{};
    QObject *m_sendTextField{};
};

#endif //UNICOMM_SENDMODULE_H
