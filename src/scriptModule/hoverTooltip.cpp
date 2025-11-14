#include "scriptModule/hoverTooltip.h"

#include <QEvent>
#include <QTextBrowser>
#include <QVBoxLayout>

// HoverTooltip public
HoverTooltip::HoverTooltip(QWidget *parent)
    : QWidget(parent),
      m_textBrowser(new QTextBrowser(this)) {
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_textBrowser);
    m_textBrowser->setFixedWidth(600);
    m_textBrowser->setFont(QFont("Consolas", 10));
    m_textBrowser->setOpenExternalLinks(true);
    m_textBrowser->installEventFilter(this);
}

// HoverTooltip private
void HoverTooltip::showTooltip(const QString &message) {
    m_textBrowser->setMarkdown(message);
    this->adjustSize();
    this->move(QCursor::pos() + QPoint(15, 15));
    this->show();
}

void HoverTooltip::hideTooltip() {
    this->hide();
}
