#ifndef UNICOMM_SIGNATUREWIDGET_H
#define UNICOMM_SIGNATUREWIDGET_H

#include <QWidget>

class SignatureWidget final : public QObject {
    Q_OBJECT

public:
    explicit SignatureWidget(QWidget *parent = nullptr);

    ~SignatureWidget() override = default;

    void propertySet(const QVariantMap &objects);

    void fontSet(const QString &family, int pointSize) const;

    [[nodiscard]] bool isVisible() const;

    void signatureShow(const QVariantHash &signatureSession, const QJsonArray &signatures);

    void signatureHide() const;

    void signatureNext();

signals:
    void setTextSelected(const QUrl &documentUrl, const QString &text);

    void addChar(const QUrl &documentUrl, QChar character);

private:
    QObject *m_tooltip{};
    QObject *m_label{};
    QVariantHash m_signatureSession{};
};

#endif //UNICOMM_SIGNATUREWIDGET_H
