#ifndef UNICOMM_TERMINALPAGE_H
#define UNICOMM_TERMINALPAGE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QJsonObject>

class ConptyWidget;
class GhosttyWidget;
class QCloseEvent;
class QQuickWidget;
class TerminalWidget;

class TerminalPage final: public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit TerminalPage(const QString &uniqueName, const QVariantHash &session, const QJsonObject &config);

    ~TerminalPage() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void start();

    void _resize(int rows, int cols);

    void stop() const;

    void titleSet(const QString &title);

    QJsonObject m_config{};
    QVariantHash m_session{};
    QQuickWidget *m_widget{};
    QObject *m_root{};
    QObject *m_messageDialog{};
    QObject *m_terminalItem{};
    ConptyWidget *m_conptyWidget{};
    TerminalWidget *m_terminalWidget{};
    GhosttyWidget *m_vtermWidget{};
    int m_rows{1};
    int m_cols{1};
};

#endif //UNICOMM_TERMINALPAGE_H
