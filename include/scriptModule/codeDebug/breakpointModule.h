#ifndef UNICOMM_BREAKPOINTMODULE_H
#define UNICOMM_BREAKPOINTMODULE_H

#include <QJsonObject>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QQuickWidget;
class QStandardItemModel;

class BreakpointModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit BreakpointModule();

    ~BreakpointModule() override = default;

    void breakpointConfigSave();

    void propertySet(const QVariantMap &objects);

    void breakpointInsert(const QUrl &scriptUrl, int line, const QVariantHash &session) const;

    void breakpointRemove(const QUrl &scriptUrl, int line) const;

    Q_INVOKABLE void scriptOpen(const QUrl &scriptUrl);

    Q_INVOKABLE void markerInsert(const QUrl &scriptUrl, int line);

    Q_INVOKABLE void breakpointDelete(const QUrl &scriptUrl, int line);

    Q_INVOKABLE void breakpointsDelete(const QUrl &scriptUrl);

    Q_INVOKABLE static QString conditionGet(const QUrl &scriptUrl, int line);

    Q_INVOKABLE void conditionSet(const QUrl &scriptUrl, int line, const QString &condition);

    Q_INVOKABLE void allDelete();

signals:
    void openScript(const QUrl &scriptUrl);

    void insertMarker(const QUrl &scriptUrl, int type, int line, int time);

    void removeMarker(const QUrl &scriptUrl, int type, int line);

private:
    QJsonObject m_breakpointConfig{};
    QQuickWidget *m_breakpointWidget{};
    QStandardItemModel *m_breakpointStandardItemModel{};
};

#endif //UNICOMM_BREAKPOINTMODULE_H
