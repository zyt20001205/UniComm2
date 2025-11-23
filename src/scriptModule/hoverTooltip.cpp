#include "scriptModule/hoverTooltip.h"

#include <QEvent>
#include <QTextBrowser>
#include <QTimer>
#include <QVBoxLayout>

// HoverTooltip public
HoverTooltip::HoverTooltip(QWidget *parent)
    : QWidget(parent),
      m_diagnosticTextBrowser(new QTextBrowser(this)),
      m_hoverTextBrowser(new QTextBrowser(this)) {
    setWindowFlags(Qt::ToolTip);
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_diagnosticTextBrowser);
    m_diagnosticTextBrowser->setFont(QFont("Consolas", 10));
    m_diagnosticTextBrowser->setOpenExternalLinks(true);
    m_diagnosticTextBrowser->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_diagnosticTextBrowser->hide();
    layout->addWidget(m_hoverTextBrowser);
    m_hoverTextBrowser->setFont(QFont("Consolas", 10));
    m_hoverTextBrowser->setOpenExternalLinks(true);
    m_hoverTextBrowser->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_hoverTextBrowser->hide();
}

void HoverTooltip::tooltipLeave() {
    QTimer::singleShot(200, this, [this] {
        if (isVisible() && !geometry().contains(QCursor::pos())) tooltipHide();
    });
}

void HoverTooltip::tooltipShowDiagnostic(const QString &message) {
    m_diagnosticTextBrowser->setMarkdown(message);
    m_diagnosticTextBrowser->show();
    tooltipResize();
    show();
}

void HoverTooltip::tooltipShowHover(const QString &message) {
    m_hoverTextBrowser->setMarkdown(message);
    m_hoverTextBrowser->show();
    tooltipResize();
    show();
}

void HoverTooltip::tooltipHide() {
    hide();
}

void HoverTooltip::tooltipResize() {
    int maxWidth = 0;
    if (m_diagnosticTextBrowser->isVisible()) {
        m_diagnosticTextBrowser->document()->adjustSize();
        const int height = m_diagnosticTextBrowser->document()->size().height() + 10;
        const int width = m_diagnosticTextBrowser->document()->size().width() + 10;
        maxWidth = qMax(maxWidth, width);
        m_diagnosticTextBrowser->setFixedHeight(height);
    }
    if (m_hoverTextBrowser->isVisible()) {
        m_hoverTextBrowser->document()->adjustSize();
        const int height = m_hoverTextBrowser->document()->size().height() + 10;
        const int width = m_hoverTextBrowser->document()->size().width() + 10;
        maxWidth = qMax(maxWidth, width);
        m_hoverTextBrowser->setFixedHeight(height);
    }

    if (m_diagnosticTextBrowser->isVisible()) {
        m_diagnosticTextBrowser->setFixedWidth(maxWidth);
    }
    if (m_hoverTextBrowser->isVisible()) {
        m_hoverTextBrowser->setFixedWidth(maxWidth);
    }
    adjustSize();
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
