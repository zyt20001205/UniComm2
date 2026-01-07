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

    void watchConfigSave() const;

    Q_INVOKABLE void watchInsert(int index, const QUrl &scriptUrl, const QString &key);

    Q_INVOKABLE void watchRemove(int index);

    Q_INVOKABLE void watchRename(int index, const QUrl &scriptUrl, const QString &key);

    Q_INVOKABLE void watchSwap(int src, int dst);

    Q_INVOKABLE void watchClear(int index);

signals:

private:
    QQuickWidget *m_watchWidget{};
    QQuickItem *m_rootItem{};
};

#endif //UNICOMM_WATCHMODULE_H