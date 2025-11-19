#ifndef UNICOMM_SIGNATUREHELPTOOLTIP_H
#define UNICOMM_SIGNATUREHELPTOOLTIP_H

#include <QWidget>

class QLabel;

class SignatureHelpTooltip final : public QWidget {
    Q_OBJECT

public:
    explicit SignatureHelpTooltip(QWidget *parent = nullptr);

    ~SignatureHelpTooltip() override = default;

    void tooltipShow(const QJsonObject &signature);

    void tooltipHide();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QLabel *m_label = nullptr;
};

#endif //UNICOMM_SIGNATUREHELPTOOLTIP_H