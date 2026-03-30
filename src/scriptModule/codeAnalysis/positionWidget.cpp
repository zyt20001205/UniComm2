#include "scriptModule/codeAnalysis/positionWidget.h"

#include <QCoreApplication>
#include <QLabel>
#include <QMouseEvent>
#include <QTimer>
#include <QVBoxLayout>
#include <windows.h>

#include "globals.h"

// public
PositionWidget::PositionWidget(QWidget *parent)
    : QWidget(parent, Qt::ToolTip),
      m_timer(new QTimer(this)),
      m_label(new QLabel(this)) {
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("positionWidget");
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addWidget(m_label);
    m_label->setFont(QFont("consolas", 12));
    m_timer->setInterval(30);
    connect(m_timer, &QTimer::timeout, [this] {
        const QPoint logicalPos = QCursor::pos();
        this->move(logicalPos + QPoint(15, 15));
        POINT physicalPos;
        GetCursorPos(&physicalPos);
        m_label->setText(QString("%1, %2").arg(QString::number(physicalPos.x), QString::number(physicalPos.y)));
    });
    setStyleSheet(
    "#positionWidget { background-color: white; border: 1px solid #cccccc; border-radius: 10px; }");
}

void PositionWidget::positionShow(const QVariantMap &positionSession) {
    m_positionSession = positionSession;
    show();
    m_timer->start();
}

void PositionWidget::positionHide() {
    hide();
}

void PositionWidget::textReplace() {
    emit insertText(
    m_positionSession["scriptUrl"].toUrl(),
    m_label->text(),
    m_positionSession["line"].toInt(),
    m_positionSession["index"].toInt());
}

// PositionWidget protected
void PositionWidget::hideEvent(QHideEvent *event) {
    m_timer->stop();
    QWidget::hideEvent(event);
}
