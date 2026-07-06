#include "document/assistant/signatureWidget.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>

#include "util/uniCast.h"

// public
SignatureWidget::SignatureWidget(QWidget *parent)
    : QObject(parent) {
}

void SignatureWidget::propertySet(const QVariantHash &objects) {
    m_tooltip = qvariant_cast<QObject *>(objects["documentModuleSignatureToolTip"]);
    m_tooltip->setProperty("signatureWidget", QVariant::fromValue(this));
    m_label = qvariant_cast<QObject *>(objects["documentModuleSignatureLabel"]);
}

void SignatureWidget::fontSet(const QString &family, const int pointSize) const {
    if (!m_label) return;
    auto font = m_label->property("font").value<QFont>();
    font.setFamily(family);
    font.setPointSize(pointSize);
    m_label->setProperty("font", font);
}

bool SignatureWidget::isVisible() const {
    if (!m_tooltip) return false;
    return m_tooltip->property("visible").toBool();
}

void SignatureWidget::signatureShow(const QVariantHash &signatureSession, const QJsonArray &signatures) {
    m_signatureSession = signatureSession;
    QString helpText{};
    bool reposition = false;
    for (const auto &value: signatures) {
        const auto signature = value.toObject();
        helpText += "(";
        int index = 0;
        const int activeParameter = signature["activeParameter"].toInt();
        if (activeParameter == 0) reposition = true;
        const QString label = signature["label"].toString();
        const QJsonArray parameters = signature["parameters"].toArray();
        if (parameters.isEmpty()) {
            helpText += ")<br>";
            continue;
        }
        for (const QJsonValue &_value: parameters) {
            const QJsonObject parameter = _value.toObject();
            const QJsonArray range = parameter["label"].toArray();
            const int startIndex = range[0].toInt();
            const int endIndex = range[1].toInt();
            QString param = label.mid(startIndex, endIndex - startIndex);
            if (index == activeParameter) {
                const auto html = uni_cast<QHtmlString>(param).value;
                // remove <p> & </p>\n
                param = QString("<span style='color: #115ea3; font-weight: 600;'>%1</span>").arg(html.mid(3, html.size() - 8));
            }
            helpText += param;
            helpText += ", ";
            index++;
        }
        helpText.chop(2);
        helpText += ")<br>";
    }
    helpText.chop(4);
    m_label->setProperty("text", helpText);
    if (reposition) {
        const auto position = signatureSession["position"].toPoint();
        m_tooltip->setProperty("position", position);
    }
    QMetaObject::invokeMethod(m_tooltip, "open");
}

void SignatureWidget::signatureHide() const {
    QMetaObject::invokeMethod(m_tooltip, "close");
    m_label->setProperty("text", "");
}

void SignatureWidget::signatureNext() {
    emit setTextSelected(
        m_signatureSession["documentUrl"].toUrl(),
        ", ");
    emit addChar(
        m_signatureSession["documentUrl"].toUrl(),
        ',');
}
