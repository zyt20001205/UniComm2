#include "scriptModule/hoverTooltip.h"

#include <QEvent>
#include <QTextBrowser>
#include <QTimer>
#include <QVBoxLayout>

// HoverTooltip public
HoverTooltip::HoverTooltip(QWidget *parent)
    : QWidget(parent),
      m_textBrowser(new QTextBrowser(this)) {
    setWindowFlags(Qt::ToolTip);
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_textBrowser);
    m_textBrowser->setFixedWidth(600);
    m_textBrowser->setFont(QFont("Consolas", 10));
    m_textBrowser->setOpenExternalLinks(true);
    m_textBrowser->installEventFilter(this);
}

void HoverTooltip::leaveRequest() {
    if (isVisible() && !geometry().contains(QCursor::pos())) hideTooltip();
}

void HoverTooltip::showTooltip(const QString &message) {
    m_textBrowser->setMarkdown(message);
    adjustSize();
    move(QCursor::pos() + QPoint(10, 10));
    show();
}

void HoverTooltip::hideTooltip() {
    hide();
}

// HoverTooltip protected
void HoverTooltip::enterEvent(QEnterEvent *event) {
    QWidget::enterEvent(event);
}

void HoverTooltip::leaveEvent(QEvent *event) {
    hideTooltip();
    QWidget::leaveEvent(event);
}
