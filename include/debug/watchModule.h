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

    static void watchConfigSave() ;

    Q_INVOKABLE static void watchInsert(int index, const QUrl &documentUrl, const QString &expression);

    Q_INVOKABLE static void watchRemove(int index);

    Q_INVOKABLE static void watchRename(int index, const QUrl &documentUrl, const QString &expression);

    Q_INVOKABLE void watchSwap(int src, int dst) const;

    Q_INVOKABLE static void watchClear(int index);

private:
    QQuickWidget *m_widget{};
    QQuickItem *m_item{};
};

#endif //UNICOMM_WATCHMODULE_H