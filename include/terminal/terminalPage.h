#ifndef UNICOMM_TERMINALPAGE_H
#define UNICOMM_TERMINALPAGE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QJsonObject>

class ConptyWidget;
class QQuickWidget;
class TerminalWidget;
class VtermWidget;

class TerminalPage final: public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit TerminalPage(const QString &uniqueName, const QVariantHash &session, const QJsonObject &config);

    ~TerminalPage() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    Q_INVOKABLE bool terminalInput(int key, int modifiers, const QString &text) const;

    Q_INVOKABLE void terminalResize(int rows, int cols) const;

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void processStart();

    void terminalWrite(const QByteArray &bytes) const;

    [[nodiscard]] bool terminalRunning() const;

    void terminalRefresh() const;

    void processStop();

    QJsonObject m_config{};
    QVariantHash m_session{};
    QQuickWidget *m_widget{};
    QObject *m_root{};
    QObject *m_messageDialog{};
    QObject *m_terminalItem{};
    ConptyWidget *m_conptyWidget{};
    TerminalWidget *m_terminalWidget{};
    VtermWidget *m_vtermWidget{};
};

#endif //UNICOMM_TERMINALPAGE_H
