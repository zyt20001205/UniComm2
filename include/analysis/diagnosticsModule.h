#ifndef UNICOMM_DIAGNOSTICS_H
#define UNICOMM_DIAGNOSTICS_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QJsonObject>
#include <QStandardItemModel>

class QQuickWidget;
class DiagnosticsModel;
class QTableWidget;
class QTabWidget;

class DiagnosticsModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit DiagnosticsModule();

    ~DiagnosticsModule() override;

    void propertySet(const QVariantHash &objects);

    void diagnosticsNotification(const QUrl &documentUrl, const QJsonArray &diagnostics);

    Q_INVOKABLE void indicatorFill(const QVariantHash &position);

signals:
    void setIndex(const QUrl &documentUrl, int startLine, int startCharacter);

    void fillIndicator(const QUrl &documentUrl, int type, int startLine, int startCharacter, int endLine, int endCharacter, int time);

private:
    QQuickWidget *m_widget{};
    QQuickItem *m_root{};
    QHash<QUrl, DiagnosticsModel *> m_diagnosticsModelHash{};
    QHash<QUrl, QTableWidget *> m_diagnosticsTableHash{};

    enum {
        LEVEL_ERROR = 1,
        LEVEL_WARNING,
        LEVEL_INFO,
        LEVEL_HINT
    };
};

class DiagnosticsModel final : public QStandardItemModel {
    Q_OBJECT

public:
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
};

#endif //UNICOMM_DIAGNOSTICS_H
