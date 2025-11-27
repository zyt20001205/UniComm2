#include "scriptModule/codeAssistant/signatureWidget.h"

#include <QEvent>
#include <QJsonArray>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>
#include <QVBoxLayout>

// SignatureWidget public
SignatureWidget::SignatureWidget(QWidget *parent)
    : QWidget(parent, Qt::ToolTip),
      m_label(new QLabel()) {
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("signatureWidget");
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addWidget(m_label);
    m_label->setFont(QFont("consolas", 12));
    setStyleSheet(
    "#signatureWidget { background-color: white; border: 1px solid #cccccc; border-radius: 10px; }");
}

void SignatureWidget::signatureShow(const QVariantMap &signatureSession, const QJsonObject &signature) {
    m_signatureSession = signatureSession;
    QString helpText;
    int index = 0;
    const int activeParameter = signature["activeParameter"].toInt();
    const QString label = signature["label"].toString();
    const QJsonArray parameters = signature["parameters"].toArray();
    if (parameters.isEmpty()) return;
    for (const QJsonValue &value: parameters) {
        const QJsonObject parameter = value.toObject();
        const QJsonArray range = parameter["label"].toArray();
        const int startIndex = range[0].toInt();
        const int endIndex = range[1].toInt();
        QString param = label.mid(startIndex, endIndex - startIndex);
        if (index == activeParameter) {
            param = QString("<span style='color: orange;'>%1</span>").arg(param);
        }
        helpText += param;
        helpText += ", ";
        index++;
    }
    helpText.chop(2);
    m_label->setText(helpText);
    show();
    move(m_signatureSession["position"].toPoint());
}

void SignatureWidget::signatureHide() {
    hide();
}
