#ifndef UNICOMM_TERMINALPAGE_H
#define UNICOMM_TERMINALPAGE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QJsonObject>

class ConptyWidget;
class QCloseEvent;
class QQuickWidget;
class TerminalWidget;
class VtermWidget;

class TerminalPage final: public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    struct Backend {
        enum {
            Conpty,
            Lua
        };
    };

    explicit TerminalPage(const QString &uniqueName, const QVariantHash &session, const QJsonObject &config, int backend = Backend::Conpty);

    ~TerminalPage() override;

    void propertySet(const QVariantHash &objects);

    void write(const QByteArray &data) const;

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void start();

    void _resize(int rows, int cols);

    void stop() const;

    void titleSet(const QString &title);

    int m_backend{};
    QJsonObject m_config{};
    QVariantHash m_session{};
    QQuickWidget *m_widget{};
    QObject *m_root{};
    QObject *m_messageDialog{};
    QObject *m_terminalItem{};
    ConptyWidget *m_conptyWidget{};
    TerminalWidget *m_terminalWidget{};
    VtermWidget *m_vtermWidget{};
    int m_rows{24};
    int m_cols{80};
};

#endif //UNICOMM_TERMINALPAGE_H
