#ifndef UNICOMM_WATCHMODULE_H
#define UNICOMM_WATCHMODULE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QQuickWidget;

class WatchModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit WatchModule();

    ~WatchModule() override;

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void watchInsert(const QUrl &scriptUrl, const QString &name);

signals:


private:
    QQuickWidget *m_watchWidget{};
};

#endif //UNICOMM_WATCHMODULE_H