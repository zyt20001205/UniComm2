#include "scriptModule/signatureHelpTooltip.h"

#include <QEvent>
#include <QJsonArray>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>
#include <QVBoxLayout>

// SignatureHelpTooltip public
SignatureHelpTooltip::SignatureHelpTooltip(QWidget *parent)
    : QWidget(parent, Qt::ToolTip),
      m_label(new QLabel()) {
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_label);
    m_label->setFont(QFont("consolas", 12));
    m_label->setStyleSheet("QLabel{background-color: white; border: 1px solid #d0d0d0;}");
}

void SignatureHelpTooltip::showTooltip(const QJsonObject &signature) {
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
}

void SignatureHelpTooltip::hideTooltip() {
    hide();
}

// SignatureHelpTooltip protected
bool SignatureHelpTooltip::eventFilter(QObject *obj, QEvent *event) {
    if (!this->isVisible()) {
        return QWidget::eventFilter(obj, event);
    }
    if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Escape:
                hideTooltip();
                return false;
            default:
                return false;
        }
    }
    return QWidget::eventFilter(obj, event);
}
