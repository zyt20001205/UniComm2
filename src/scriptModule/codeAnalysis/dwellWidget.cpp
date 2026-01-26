#include "scriptModule/codeAnalysis/dwellWidget.h"

#include <QMenu>
#include <QTextBrowser>
#include <QTimer>
#include <QVBoxLayout>

#include "globals.h"
#include "scriptModule/nuspellModule.h"

// DwellWidget public
DwellWidget::DwellWidget(QWidget *parent)
    : QWidget(parent, Qt::ToolTip),
      m_diagnosticTextBrowser(new QTextBrowser(this)),
      m_hoverTextBrowser(new QTextBrowser(this)),
      m_suggestionMenu(new QMenu(this)) {
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("dwellWidget");
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);
    layout->addWidget(m_diagnosticTextBrowser);
    m_diagnosticTextBrowser->setFocusPolicy(Qt::NoFocus);
    m_diagnosticTextBrowser->setFont(QFont("Segoe UI", 10));
    m_diagnosticTextBrowser->setObjectName("diagnosticTextBrowser");
    m_diagnosticTextBrowser->setOpenExternalLinks(false);
    m_diagnosticTextBrowser->setOpenLinks(false);
    m_diagnosticTextBrowser->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_diagnosticTextBrowser->hide();
    connect(m_diagnosticTextBrowser, &QTextBrowser::anchorClicked, this, [this](const QUrl &commandLine) {
        const QString command = commandLine.scheme();
        if (command == "requestcodeaction") {
            const QStringList arguments = commandLine.path().split('/');
            m_diagnosticLineFrom = arguments[1].toInt();
            m_diagnosticIndexFrom = arguments[2].toInt();
            m_diagnosticLineTo = arguments[3].toInt();
            m_diagnosticIndexTo = arguments[4].toInt();
            emit requestCodeAction(m_scriptUrl, m_diagnosticLineFrom, m_diagnosticIndexFrom, m_diagnosticLineTo, m_diagnosticIndexTo);
        } else if (command == "requestspellsuggest") {
            const QString word = commandLine.host();
            const QStringList arguments = commandLine.path().split('/');
            m_typoLineFrom = arguments[1].toInt();
            m_typoIndexFrom = arguments[2].toInt();
            m_typoLineTo = arguments[3].toInt();
            m_typoIndexTo = arguments[4].toInt();
            const QStringList suggestions = g_nuspell->spellSuggestRequest(word);
            dwellShowSuggestions(suggestions);
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
        "#dwellWidget { background-color: white; border: 1px solid #cccccc; border-radius: 10px; }"
        "#diagnosticTextBrowser { background-color: white; border: none; border-top-left-radius: 9px; border-top-right-radius: 9px; padding: 10px;}"
        "#hoverTextBrowser { background-color: #fafafa; border: none; border-bottom-left-radius: 9px; border-bottom-right-radius: 9px; padding: 10px; }");
}

void DwellWidget::propertySet(const QVariantMap &objects) {
    m_tooltip = qvariant_cast<QObject *>(objects["scriptModuleDwellToolTip"]);
}

void DwellWidget::dwellLeave() {
    QTimer::singleShot(200, this, [this] {
        if (isVisible() && !geometry().contains(QCursor::pos())) dwellHide();
    });
}

void DwellWidget::dwellShowDiagnostic(const QUrl &scriptUrl, const QString &message) {
    m_scriptUrl = scriptUrl;
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

void DwellWidget::hoverShow(const QVariantHash &hoverSession, const QString &message) {
    const auto position = hoverSession["position"].toPoint();
    m_tooltip->setProperty("position", position);
    QMetaObject::invokeMethod(m_tooltip, "open");

    m_hoverTextBrowser->setMarkdown(message);
    m_hoverTextBrowser->document()->setTextWidth(600);
    m_hoverTextBrowser->setFixedWidth(600 + 20);
    m_hoverTextBrowser->show();
    show();
    move(QCursor::pos() + QPoint(10, 10));
    QTimer::singleShot(0, this, [this] {
        m_hoverTextBrowser->setFixedHeight(m_hoverTextBrowser->document()->size().height() + 20);
        adjustSize();
    });
}

void DwellWidget::dwellShowCodeAction(const QUrl &scriptUrl, const QJsonArray &result) const {
    qDebug() << result;
}

void DwellWidget::dwellHide() {
    hide();
}

// DwellWidget protected
void DwellWidget::enterEvent(QEnterEvent *event) {
    QWidget::enterEvent(event);
}

void DwellWidget::hideEvent(QHideEvent *event) {
    m_diagnosticTextBrowser->hide();
    m_hoverTextBrowser->hide();
    QWidget::hideEvent(event);
}

void DwellWidget::leaveEvent(QEvent *event) {
    if (m_suggestionMenu->isVisible()) return;
    dwellHide();
    QWidget::leaveEvent(event);
}

// DwellWidget private
void DwellWidget::dwellShowSuggestions(const QStringList &suggestions) {
    m_suggestionMenu->clear();
    for (const auto &suggestion: suggestions) {
        const auto suggestionAction = new QAction(suggestion, m_suggestionMenu); // NOLINT
        connect(suggestionAction, &QAction::triggered, this, [this, suggestion] {
            emit replaceText(m_scriptUrl, suggestion, m_typoLineFrom, m_typoIndexFrom, m_typoLineTo, m_typoIndexTo);
            dwellHide();
        });
        m_suggestionMenu->addAction(suggestionAction);
    }
    m_suggestionMenu->exec(mapToGlobal(QPoint(width(), 0)));
}
