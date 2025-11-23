#ifndef UNICOMM_DIAGNOSTICS_H
#define UNICOMM_DIAGNOSTICS_H

#include <QJsonObject>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QTableWidget;
class QTabWidget;

class DiagnosticsModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit DiagnosticsModule();

    ~DiagnosticsModule() override = default;

    void diagnosticsNotification(const QUrl &scriptUrl, const QJsonArray &diagnostics);

signals:
    void openScript(const QUrl &scriptUrl);

    void setCursorPosition(const QUrl &scriptUrl, int startLine, int startCharacter);

    void insertIndicator(const QUrl &scriptUrl, int type, int startLine, int startCharacter, int endLine, int endCharacter, int time);

private:
    void diagnosticsClose(int index);

    void diagnosticsPublish(const QUrl &scriptUrl, const QJsonArray &diagnostics);

    void diagnosticsRemove(const QUrl &scriptUrl);

    QTabWidget *m_diagnosticsTabWidget{};
    QHash<int, QColor> m_diagnosticsColor{};
    QHash<QUrl, QTableWidget *> m_diagnosticsTableHash{};

    enum {
        LEVEL_ERROR = 1,
        LEVEL_WARNING,
        LEVEL_INFO,
        LEVEL_HINT
    };
};

#endif //UNICOMM_DIAGNOSTICS_H
