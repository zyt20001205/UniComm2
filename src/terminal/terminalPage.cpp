#include "terminal/terminalPage.h"

#include <QCloseEvent>
#include <QKeyEvent>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>

#include "globals.h"
#include "core/globalManager.h"
#include "terminal/module/conptyWidget.h"
#include "terminal/module/terminalWidget.h"
#include "terminal/module/vtermWidget.h"

// public
TerminalPage::TerminalPage(const QString &uniqueName, const QVariantHash &session, const QJsonObject &config)
    : DockWidget(uniqueName),
      m_config(config),
      m_session(session),
      m_widget(new QQuickWidget()),
      m_conptyWidget(new ConptyWidget(this)),
      m_vtermWidget(new VtermWidget(1, 1, this)) {
    setWidget(m_widget);
    m_widget->installEventFilter(this);
}

TerminalPage::~TerminalPage() {
    stop();
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 destructed").arg(timestamp, uniqueName());
}

void TerminalPage::propertySet(const QVariantHash &objects) {
    m_widget->rootContext()->setContextProperty("global", g_globalManager);
    m_widget->rootContext()->setContextProperty("terminalPage", this);

    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/terminal/terminalPage.qml"));
    m_root = m_widget->rootObject();
}

void TerminalPage::propertyGet(const QVariantMap &objects) {
    m_terminalItem = qvariant_cast<QObject *>(objects["terminalItem"]);
    auto *terminalItem = qobject_cast<QQuickItem *>(m_terminalItem);

    auto font = QFont(m_config["fontFamily"].toString(), m_config["fontSize"].toInt());
    font.setFixedPitch(true);
    font.setStyleHint(QFont::Monospace);

    m_terminalWidget = new TerminalWidget(terminalItem);
    m_terminalWidget->setParentItem(terminalItem);

    connect(m_terminalWidget, &TerminalWidget::keyPressed, m_vtermWidget, &VtermWidget::keyPressed);
    connect(m_vtermWidget, &VtermWidget::outputWrite, m_conptyWidget, &ConptyWidget::inputWrite);
    connect(m_conptyWidget, &ConptyWidget::outputWrite, m_vtermWidget, &VtermWidget::inputWrite);
    connect(m_vtermWidget, &VtermWidget::setScreen, m_terminalWidget, &TerminalWidget::screenSet);

    connect(m_conptyWidget, &ConptyWidget::quit, this, &TerminalPage::close);

    connect(terminalItem, &QQuickItem::widthChanged, m_terminalWidget, [this, terminalItem] {m_terminalWidget->setWidth(terminalItem->width());});
    connect(terminalItem, &QQuickItem::heightChanged, m_terminalWidget, [this, terminalItem] {m_terminalWidget->setHeight(terminalItem->height());});
    connect(m_terminalWidget, &TerminalWidget::resize, this, &TerminalPage::_resize);

    m_terminalWidget->setWidth(terminalItem->width());
    m_terminalWidget->setHeight(terminalItem->height());
    m_terminalWidget->fontSet(font);

    start();
}

bool TerminalPage::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_widget && event->type() == QEvent::KeyPress) {
        const auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab) {
            if (m_vtermWidget) m_vtermWidget->keyPressed(keyEvent->key(), keyEvent->modifiers(), "\t");
            return true;
        }
    }
    return DockWidget::eventFilter(watched, event);
}

// protected
void TerminalPage::closeEvent(QCloseEvent *event) {
    stop();
    deleteLater();
    event->accept();
}

// private
void TerminalPage::start() {
    if (!m_conptyWidget || !m_vtermWidget) return;
    const bool started = m_conptyWidget->start(
        m_session["program"].toString(),
        m_session["arguments"].toString(),
        g_workspaceUrl.toLocalFile(),
        m_rows,
        m_cols
    );
    if (!started) close();
}

void TerminalPage::_resize(const int rows, const int cols) {
    if (m_rows == rows && m_cols == cols) return;
    m_rows = rows;
    m_cols = cols;
    if (m_vtermWidget) m_vtermWidget->resize(rows, cols);
    if (m_conptyWidget) m_conptyWidget->resize(rows, cols);
}

void TerminalPage::stop() const {
    if (m_conptyWidget) m_conptyWidget->stop();
}
