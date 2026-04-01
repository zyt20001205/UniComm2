#ifndef UNICOMM_REPLACEWIDGET_H
#define UNICOMM_REPLACEWIDGET_H

#include <QQuickWidget>

class ReplaceWidget final : public QQuickWidget {
    Q_OBJECT

public:
    explicit ReplaceWidget(QWidget *parent = nullptr);

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    Q_INVOKABLE void textReplace();

    Q_INVOKABLE void textReplaceAll();

    void replaceEnable(bool status) const;

signals:
    void replaceText(const QString &text);

private:
    QObject *m_textField{};
    QObject *m_replaceButton{};
    QObject *m_replaceAllButton{};
};

#endif //UNICOMM_REPLACEWIDGET_H
