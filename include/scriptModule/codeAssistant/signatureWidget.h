#ifndef UNICOMM_SIGNATUREWIDGET_H
#define UNICOMM_SIGNATUREWIDGET_H

#include <QWidget>

class QLabel;

class SignatureWidget final : public QWidget {
    Q_OBJECT

public:
    explicit SignatureWidget(QWidget *parent = nullptr);

    ~SignatureWidget() override = default;

    void signatureShow(const QVariantMap &signatureSession, const QJsonObject &signature);

    void signatureHide();

private:
    QVariantMap m_signatureSession{};
    QLabel *m_label{};
};

#endif //UNICOMM_SIGNATUREWIDGET_H