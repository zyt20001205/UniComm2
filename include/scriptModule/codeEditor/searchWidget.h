#ifndef UNICOMM_SEARCHWIDGET_H
#define UNICOMM_SEARCHWIDGET_H

#include <QQuickWidget>

class SearchWidget final : public QQuickWidget {
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = nullptr);

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    Q_INVOKABLE void textSearch(const QString &text);

    Q_INVOKABLE void searchFlagsSet(bool matchCase, bool wholeWord, bool wordStart, bool regExp);

signals:
    void setSearchFlags(bool matchCase, bool wholeWord, bool wordStart, bool regExp);

protected:
    void showEvent(QShowEvent *event) override;

private:
    QObject *m_searchTextField{};
};

#endif //UNICOMM_SEARCHWIDGET_H
