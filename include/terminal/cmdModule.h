#ifndef UNICOMM_CMDMODULE_H
#define UNICOMM_CMDMODULE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QJsonObject>

class QTextDocument;
class QQuickWidget;

class CmdModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit CmdModule();

    ~CmdModule() override;

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

private:
    QJsonObject m_config{};
    QQuickWidget *m_widget{};
    QObject *m_messageDialog{};
    QObject *m_textArea{};
    QTextDocument *m_textDocument{};
};

#endif //UNICOMM_CMDMODULE_H
