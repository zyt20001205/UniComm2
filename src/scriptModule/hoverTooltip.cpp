#include "scriptModule/hoverTooltip.h"

#include <QEvent>
#include <QTextBrowser>
#include <QTimer>
#include <QVBoxLayout>

// HoverTooltip public
HoverTooltip::HoverTooltip(QWidget *parent)
    : QWidget(parent),
      m_diagnosticTextBrowser(new QTextBrowser(this)),
      m_hoverTextBrowser(new QTextBrowser(this)){
    setWindowFlags(Qt::ToolTip);
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_diagnosticTextBrowser);
    m_diagnosticTextBrowser->setFixedWidth(600);
    m_diagnosticTextBrowser->setFont(QFont("Consolas", 10));
    m_diagnosticTextBrowser->setOpenExternalLinks(true);
    m_diagnosticTextBrowser->hide();
    layout->addWidget(m_hoverTextBrowser);
    m_hoverTextBrowser->setFixedWidth(600);
    m_hoverTextBrowser->setFont(QFont("Consolas", 10));
    m_hoverTextBrowser->setOpenExternalLinks(true);
    m_hoverTextBrowser->hide();
}

void HoverTooltip::tooltipLeave() {
    QTimer::singleShot(100, this,[this] {
        if (isVisible() && !geometry().contains(QCursor::pos())) tooltipHide();
    });
}

void HoverTooltip::tooltipShowDiagnostic(const QString &message) {
    m_diagnosticTextBrowser->setMarkdown(message);
    m_diagnosticTextBrowser->show();
    adjustSize();
    show();
}

void HoverTooltip::tooltipShowHover(const QString &message) {
    m_hoverTextBrowser->setMarkdown(message);
    m_hoverTextBrowser->show();
    adjustSize();
    show();
}

void HoverTooltip::tooltipHide() {
    hide();
}

// HoverTooltip protected
void HoverTooltip::enterEvent(QEnterEvent *event) {
    QWidget::enterEvent(event);
}

void HoverTooltip::hideEvent(QHideEvent *event) {
    m_diagnosticTextBrowser->hide();
    m_hoverTextBrowser->hide();
    QWidget::hideEvent(event);
}

void HoverTooltip::leaveEvent(QEvent *event) {
    tooltipHide();
    QWidget::leaveEvent(event);
}
