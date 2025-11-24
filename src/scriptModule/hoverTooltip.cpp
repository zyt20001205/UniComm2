#include "scriptModule/hoverTooltip.h"

#include <QEvent>
#include <QMenu>
#include <QTextBrowser>
#include <QTimer>
#include <QVBoxLayout>

#include "globals.h"
#include "scriptModule/nuspellModule.h"

// HoverTooltip public
HoverTooltip::HoverTooltip(QWidget *parent)
    : QWidget(parent),
      m_diagnosticTextBrowser(new QTextBrowser(this)),
      m_hoverTextBrowser(new QTextBrowser(this)),
      m_suggestionMenu(new QMenu(this)) {
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("hoverTooltip");
    setWindowFlags(Qt::ToolTip);
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);
    layout->addWidget(m_diagnosticTextBrowser);
    m_diagnosticTextBrowser->setFont(QFont("Segoe UI", 10));
    m_diagnosticTextBrowser->setObjectName("diagnosticTextBrowser");
    m_diagnosticTextBrowser->setOpenExternalLinks(false);
    m_diagnosticTextBrowser->setOpenLinks(false);
    m_diagnosticTextBrowser->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_diagnosticTextBrowser->hide();
    connect(m_diagnosticTextBrowser, &QTextBrowser::anchorClicked, this, [this](const QUrl &commandLine) {
        const QString command = commandLine.scheme();
        if (command == "requestspellsuggest") {
            const QString word = commandLine.host();
            const QStringList arguments = commandLine.path().split('/');
            m_lineFrom = arguments[1].toInt();
            m_indexFrom = arguments[2].toInt();
            m_lineTo = arguments[3].toInt();
            m_indexTo = arguments[4].toInt();
            const QStringList suggestions = g_nuspell->spellSuggestRequest(word);
            toolTipShowSuggestions(suggestions);
        }
    });
    layout->addWidget(m_hoverTextBrowser);
    m_hoverTextBrowser->setFont(QFont("Segoe UI", 10));
    m_hoverTextBrowser->setObjectName("hoverTextBrowser");
    m_hoverTextBrowser->setOpenExternalLinks(true);
    m_hoverTextBrowser->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_hoverTextBrowser->hide();
    // stylesheets
    setStyleSheet(
        "#hoverTooltip { background-color: white; border: 1px solid #cccccc; border-radius: 10px; }"
        "#diagnosticTextBrowser { background-color: white; border: none; border-top-left-radius: 9px; border-top-right-radius: 9px; padding: 10px;}"
        "#hoverTextBrowser { background-color: #fafafa; border: none; border-bottom-left-radius: 9px; border-bottom-right-radius: 9px; padding: 10px; }");
}

void HoverTooltip::tooltipLeave() {
    QTimer::singleShot(200, this, [this] {
        if (isVisible() && !geometry().contains(QCursor::pos())) tooltipHide();
    });
}

void HoverTooltip::tooltipShowDiagnostic(const QString &message) {
    m_diagnosticTextBrowser->setHtml(message);
    m_diagnosticTextBrowser->document()->setTextWidth(600);
    m_diagnosticTextBrowser->setFixedWidth(600 + 20);
    m_diagnosticTextBrowser->show();
    show();
    QTimer::singleShot(0, this, [this] {
        m_diagnosticTextBrowser->setFixedHeight(m_diagnosticTextBrowser->document()->size().height() + 20);
        adjustSize();
        move(QCursor::pos() + QPoint(10, 10));
    });
}

void HoverTooltip::tooltipShowHover(const QString &message) {
    m_hoverTextBrowser->setMarkdown(message);
    m_hoverTextBrowser->document()->setTextWidth(600);
    m_hoverTextBrowser->setFixedWidth(600 + 20);
    m_hoverTextBrowser->show();
    show();
    QTimer::singleShot(0, this, [this] {
        m_hoverTextBrowser->setFixedHeight(m_hoverTextBrowser->document()->size().height() + 20);
        adjustSize();
        move(QCursor::pos() + QPoint(10, 10));
    });
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
    if (m_suggestionMenu->isVisible()) return;
    tooltipHide();
    QWidget::leaveEvent(event);
}

// HoverTooltip private
void HoverTooltip::toolTipShowSuggestions(const QStringList &suggestions) {
    m_suggestionMenu->clear();
    for (const auto &suggestion: suggestions) {
        const auto suggestionAction = new QAction(suggestion, m_suggestionMenu); // NOLINT
        connect(suggestionAction, &QAction::triggered, this, [this, suggestion] {
            emit replaceText(suggestion, m_lineFrom, m_indexFrom, m_lineTo, m_indexTo);
            tooltipHide();
        });
        m_suggestionMenu->addAction(suggestionAction);
    }
    m_suggestionMenu->exec(mapToGlobal(QPoint(width(), 0)));
}
