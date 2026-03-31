#ifndef UNICOMM_REPLACEWIDGET_H
#define UNICOMM_REPLACEWIDGET_H

#include <QQuickWidget>

class ReplaceWidget final : public QQuickWidget {
    Q_OBJECT

public:
    explicit ReplaceWidget(QWidget *parent = nullptr);

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    Q_INVOKABLE void textReplace(const QString &text);

    Q_INVOKABLE void textReplaceAll(const QString &text);

private:
};

#endif //UNICOMM_REPLACEWIDGET_H
