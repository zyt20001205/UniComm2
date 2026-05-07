#ifndef UNICOMM_REPLACEWIDGET_H
#define UNICOMM_REPLACEWIDGET_H

#include <QQuickWidget>

class ReplaceWidget final : public QQuickWidget {
    Q_OBJECT

public:
    explicit ReplaceWidget(QWidget *parent = nullptr);

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    Q_INVOKABLE void textReplace();

    Q_INVOKABLE void allReplace();

    void replaceEnable(bool status) const;

signals:
    void replaceText(const QString &text);

    void replaceAll(const QString &text);

private:
    QObject *m_textField{};
    QObject *m_replaceTextButton{};
    QObject *m_replaceAllButton{};
};

#endif //UNICOMM_REPLACEWIDGET_H
