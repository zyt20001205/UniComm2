#ifndef UNICOMM_BREAKPOINTMODULE_H
#define UNICOMM_BREAKPOINTMODULE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QQuickWidget;
class QStandardItemModel;

class BreakpointModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit BreakpointModule();

    ~BreakpointModule() override = default;

    void breakpointInsert(const QUrl &scriptUrl, int line) const;

    Q_INVOKABLE void breakpointRemove(const QUrl &scriptUrl, int line) const;

    Q_INVOKABLE void scriptOpen(const QUrl &scriptUrl);

    Q_INVOKABLE void markerInsert(const QUrl &scriptUrl, int line);

    Q_INVOKABLE void markerRemove(const QUrl &scriptUrl, int line);

signals:
    void openScript(const QUrl &scriptUrl);

    void insertMarker(const QUrl &scriptUrl, int type, int line, int time);

    void removeMarker(const QUrl &scriptUrl, int type, int line);

private:
    QQuickWidget *m_breakpointWidget{};
    QStandardItemModel *m_breakpointStandardModel{};
};

#endif //UNICOMM_BREAKPOINTMODULE_H
