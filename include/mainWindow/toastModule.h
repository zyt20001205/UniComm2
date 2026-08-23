#ifndef UNICOMM_TOASTMODULE_H
#define UNICOMM_TOASTMODULE_H

#include <functional>

#include <QHash>
#include <QList>
#include <QQuickView>
#include <QString>

class QQmlEngine;

struct ToastAction {
    QString text;
    std::function<void()> callback;
};

class ToastModule final : public QQuickView {
    Q_OBJECT

public:
    explicit ToastModule(QQmlEngine *engine, QWindow &owner);

    void show(int level, const QString &title, const QString &text = QString(), QList<ToastAction> actions = {}, int duration = 0);

    Q_INVOKABLE void actionTrigger(int actionGroupId, int actionIndex);

    Q_INVOKABLE void actionRemove(int actionGroupId);

private:
    void geometryUpdate();

    QWindow &m_owner;
    QObject *m_root{};
    int m_actionGroupId{};
    QHash<int, QList<std::function<void()> > > m_callbackGroups{};
};

#endif //UNICOMM_TOASTMODULE_H
