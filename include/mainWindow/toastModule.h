#ifndef UNICOMM_TOASTMODULE_H
#define UNICOMM_TOASTMODULE_H

#include <functional>

#include <QHash>
#include <QQuickView>

class QQmlEngine;

class ToastModule final : public QQuickView {
    Q_OBJECT

public:
    explicit ToastModule(QQmlEngine *engine, QWindow &owner);

    void show(int level, const QString &title, const QString &text = QString(), int duration = 5000,
              const QString &actionText = QString(), std::function<void()> callback = {});

    Q_INVOKABLE void actionTrigger(int actionId);

    Q_INVOKABLE void actionRemove(int actionId);

private:
    void geometryUpdate();

    QWindow &m_owner;
    QObject *m_root{};
    int m_actionId{};
    QHash<int, std::function<void()>> m_callbacks{};
};

#endif //UNICOMM_TOASTMODULE_H
