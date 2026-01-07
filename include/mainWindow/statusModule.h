#ifndef UNICOMM_STATUSMODULE_H
#define UNICOMM_STATUSMODULE_H

#include <QQuickWidget>

class StatusModule final : public QQuickWidget {
    Q_OBJECT

public:
    explicit StatusModule(QWidget *parent = nullptr);

    ~StatusModule() override;

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void scriptFocus(const QUrl &scriptUrl) const;

    void scriptPosition(int row, int column) const;

    void threadRefresh(int run, int debug) const;

private:
    QQuickItem *m_rootItem{};
    QObject *m_positionButton{};
    QObject *m_threadButton{};
};

#endif //UNICOMM_STATUSMODULE_H