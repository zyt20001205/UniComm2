#ifndef UNICOMM_SIGNATUREWIDGET_H
#define UNICOMM_SIGNATUREWIDGET_H

#include <QMap>
#include <QObject>

class QLabel;

class SignatureWidget final : public QObject {
    Q_OBJECT

public:
    explicit SignatureWidget(QWidget *parent = nullptr);

    ~SignatureWidget() override = default;

    void propertySet(const QVariantMap &objects);

    void fontSet(const QString &family, int pointSize) const;

    bool isVisible() const;

    void signatureShow(const QVariantMap &signatureSession, const QJsonObject &signature);

    void signatureHide() const;

private:
    QObject *m_tooltip{};
    QObject *m_label{};
    QVariantMap m_signatureSession{};
};

#endif //UNICOMM_SIGNATUREWIDGET_H