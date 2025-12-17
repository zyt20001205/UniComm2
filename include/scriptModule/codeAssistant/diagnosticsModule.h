#ifndef UNICOMM_DIAGNOSTICS_H
#define UNICOMM_DIAGNOSTICS_H

#include <QJsonObject>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QQuickWidget;
class QStandardItemModel;
class QTableWidget;
class QTabWidget;

class DiagnosticsModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit DiagnosticsModule();

    ~DiagnosticsModule() override = default;

    void propertySet(const QVariantMap &objects);

    void diagnosticsNotification(const QUrl &scriptUrl, const QJsonArray &diagnostics);

    Q_INVOKABLE static void diagnosticCopy(const QString &diagnostic);

    Q_INVOKABLE void indicatorInsert(const QVariantHash &position);

signals:
    void openScript(const QUrl &scriptUrl);

    void setCursorPosition(const QUrl &scriptUrl, int startLine, int startCharacter);

    void insertIndicator(const QUrl &scriptUrl, int type, int startLine, int startCharacter, int endLine, int endCharacter, int time);

private:
    QQuickWidget *m_diagnosticsWidget{};
    QQuickItem *m_diagnosticsRoot{};
    QHash<QUrl, QStandardItemModel *> m_diagnosticsModelHash{};
    QHash<QUrl, QTableWidget *> m_diagnosticsTableHash{};

    enum {
        LEVEL_ERROR = 1,
        LEVEL_WARNING,
        LEVEL_INFO,
        LEVEL_HINT
    };
};

#endif //UNICOMM_DIAGNOSTICS_H
