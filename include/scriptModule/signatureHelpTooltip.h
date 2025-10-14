#ifndef UNICOMM_SIGNATUREHELPTOOLTIP_H
#define UNICOMM_SIGNATUREHELPTOOLTIP_H

#include <QWidget>

class QLabel;

class SignatureHelpTooltip final : public QWidget {
    Q_OBJECT

public:
    explicit SignatureHelpTooltip(QWidget *parent = nullptr);

    ~SignatureHelpTooltip() override = default;

    void showTooltip(const QJsonObject &signature);

    void hideTooltip();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QLabel *m_label = nullptr;
};

#endif //UNICOMM_SIGNATUREHELPTOOLTIP_H