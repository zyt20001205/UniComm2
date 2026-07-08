#include "terminal/terminalPage.h"

#include <QCloseEvent>
#include <QKeyEvent>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>

#include "globals.h"
#include "core/globalManager.h"
#include "terminal/module/conptyWidget.h"
#include "terminal/module/ghosttyWidget.h"
#include "terminal/module/terminalWidget.h"

// public
TerminalPage::TerminalPage(const QString &uniqueName, const QVariantHash &session, const QJsonObject &config)
    : DockWidget(uniqueName),
      m_config(config),
      m_session(session),
      m_widget(new QQuickWidget()),
      m_conptyWidget(new ConptyWidget(this)),
      m_ghosttyWidget(new GhosttyWidget(1, 1, this)) {
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

    connect(m_terminalWidget, &TerminalWidget::keyPressed, m_ghosttyWidget, &GhosttyWidget::keyPressed);
    connect(m_terminalWidget, &TerminalWidget::mousePressed, m_ghosttyWidget, &GhosttyWidget::mousePressed);
    connect(m_terminalWidget, &TerminalWidget::mouseReleased, m_ghosttyWidget, &GhosttyWidget::mouseReleased);
    connect(m_terminalWidget, &TerminalWidget::mouseMoved, m_ghosttyWidget, &GhosttyWidget::mouseMoved);
    connect(m_terminalWidget, &TerminalWidget::mouseWheeled, m_ghosttyWidget, &GhosttyWidget::mouseWheeled);
    connect(m_terminalWidget, &TerminalWidget::mouseScrolled, m_ghosttyWidget, &GhosttyWidget::mouseScrolled);
    connect(m_ghosttyWidget, &GhosttyWidget::outputWrite, m_conptyWidget, &ConptyWidget::inputWrite);
    connect(m_conptyWidget, &ConptyWidget::outputWrite, m_ghosttyWidget, &GhosttyWidget::inputWrite);
    connect(m_ghosttyWidget, &GhosttyWidget::setScreen, m_terminalWidget, &TerminalWidget::screenSet);
    connect(m_ghosttyWidget, &GhosttyWidget::setCursorPosition, m_terminalWidget, &TerminalWidget::cursorPositionSet);
    connect(m_ghosttyWidget, &GhosttyWidget::setCursorVisible, m_terminalWidget, &TerminalWidget::cursorVisibleSet);
    connect(m_ghosttyWidget, &GhosttyWidget::setCursorBlink, m_terminalWidget, &TerminalWidget::cursorBlinkSet);
    connect(m_ghosttyWidget, &GhosttyWidget::setTitle, this, &TerminalPage::titleSet);
    connect(m_ghosttyWidget, &GhosttyWidget::setCursorShape, m_terminalWidget, &TerminalWidget::cursorShapeSet);
    connect(m_ghosttyWidget, &GhosttyWidget::setCursorMode, m_terminalWidget, &TerminalWidget::cursorModeSet);

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
            if (m_ghosttyWidget) m_ghosttyWidget->keyPressed(keyEvent->key(), keyEvent->modifiers(), "\t");
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
    if (!m_conptyWidget || !m_ghosttyWidget) return;
    const bool started = m_conptyWidget->start(
        m_session["program"].toUrl(),
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
    if (m_ghosttyWidget) m_ghosttyWidget->resize(rows, cols);
    if (m_conptyWidget) m_conptyWidget->resize(rows, cols);
}

void TerminalPage::stop() const {
    if (m_conptyWidget) m_conptyWidget->stop();
}

void TerminalPage::titleSet(const QString &title) {
    setTitle(title);
}
