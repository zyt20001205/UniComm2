#ifndef UNICOMM_SEARCHWIDGET_H
#define UNICOMM_SEARCHWIDGET_H

#include <QQuickWidget>

class SearchWidget final : public QQuickWidget {
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = nullptr);

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    Q_INVOKABLE void searchFlagsSet(bool matchCase, bool wholeWord, bool wordStart, bool regExp);

    Q_INVOKABLE void searchRequest(const QString &text);

    void searchResponse(const QString &text) const;

signals:
    void setSearchFlags(bool matchCase, bool wholeWord, bool wordStart, bool regExp);

    void requestSearch(const QString &text);

protected:
    void showEvent(QShowEvent *event) override;

private:
    QObject *m_textField{};
    QObject *m_label{};
};

#endif //UNICOMM_SEARCHWIDGET_H
