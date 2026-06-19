#ifndef UNICOMM_RESOLVEWIDGET_H
#define UNICOMM_RESOLVEWIDGET_H

#include <QQuickWidget>

class ResolveWidget final : public QQuickWidget {
    Q_OBJECT

public:
    explicit ResolveWidget(QWidget *parent = nullptr);

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    Q_INVOKABLE void resolvePrev();

    Q_INVOKABLE void resolveNext();

signals:
    void prevResolve();

    void nextResolve();

private:
    QObject *m_resolvePrevButton{};
    QObject *m_resolveNextButton{};
    QObject *m_resolveStatLabel{};
};

#endif //UNICOMM_RESOLVEWIDGET_H
