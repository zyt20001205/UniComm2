#include "scriptModule/codeAssistant/signatureWidget.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>

// SignatureWidget public
SignatureWidget::SignatureWidget(QWidget *parent)
    : QObject(parent) {
}

void SignatureWidget::propertySet(const QVariantMap &objects) {
    m_tooltip = qvariant_cast<QObject *>(objects["scriptModuleSignatureToolTip"]);
    m_label = qvariant_cast<QObject *>(objects["scriptModuleSignatureLabel"]);;
}

void SignatureWidget::fontSet(const QString &family, const int pointSize) const {
    m_label->setProperty("font.family", family);
    m_label->setProperty("font.pointSize", pointSize);
}

bool SignatureWidget::isVisible() const {
    if (!m_tooltip) return false;
    return m_tooltip->property("visible").toBool();
}

void SignatureWidget::signatureShow(const QVariantMap &signatureSession, const QJsonObject &signature) const {
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
            param = QString("<span style='color: orange; font-weight: 600;'>%1</span>").arg(param);
        }
        helpText += param;
        helpText += ", ";
        index++;
    }
    helpText.chop(2);
    m_label->setProperty("text", helpText);
    if (activeParameter == 0) {
        const auto position = signatureSession["position"].toPoint();
        m_tooltip->setProperty("position", position);
    }
    QMetaObject::invokeMethod(m_tooltip, "open");
}

void SignatureWidget::signatureHide() const {
    QMetaObject::invokeMethod(m_tooltip, "close");
}
