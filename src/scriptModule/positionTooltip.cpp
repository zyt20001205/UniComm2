#include "scriptModule/positionTooltip.h"

#include <QCoreApplication>
#include <QLabel>
#include <QMouseEvent>
#include <QTimer>
#include <QVBoxLayout>
#include <windows.h>

// PositionTooltip public
PositionTooltip::PositionTooltip(QWidget *parent)
    : QWidget(parent, Qt::ToolTip),
      m_timer(new QTimer(this)),
      m_label(new QLabel(this)) {
    qApp->installEventFilter(this);
    setWindowFlags(Qt::Popup);
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_label);
    m_label->setFont(QFont("consolas", 12));
    m_timer->setInterval(30);
    connect(m_timer, &QTimer::timeout, [this] {
        const QPoint logicalPos = QCursor::pos();
        this->move(logicalPos + QPoint(15, 15));
        POINT physicalPos;
        GetCursorPos(&physicalPos);
        m_label->setText(QString("X: %1, Y: %2").arg(QString::number(physicalPos.x), QString::number(physicalPos.y)));
    });
}

void PositionTooltip::tooltipShow() {
    this->show();
    m_timer->start();
}

void PositionTooltip::tooltipHide() {
    this->hide();
    m_timer->stop();
}

// PositionTooltip protected
bool PositionTooltip::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress && this->isVisible()) {
        const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            POINT physicalPos;
            GetCursorPos(&physicalPos);
            QString text = QString("%1, %2").arg(QString::number(physicalPos.x), QString::number(physicalPos.y));
            emit replaceText(text, "Text");
            tooltipHide();
        }
    }
    return QWidget::eventFilter(obj, event);
}
