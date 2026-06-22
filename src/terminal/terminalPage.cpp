#include "terminal/terminalPage.h"

#include <QKeyEvent>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <vterm.h>

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
    connect(m_conptyWidget, &ConptyWidget::outputReady, this, [this](const QByteArray &bytes) {
        m_vtermWidget->inputWrite(bytes);
        terminalRefresh();
    });
    connect(m_vtermWidget, &VtermWidget::write, m_conptyWidget, &ConptyWidget::write);
    connect(m_conptyWidget, &ConptyWidget::closed, this, [this] {
        close();
    });
}

TerminalPage::~TerminalPage() {
    processStop();
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 destructed").arg(timestamp, uniqueName());
}

void TerminalPage::propertySet(const QVariantHash &objects) {
    m_widget->rootContext()->setContextProperty("global", g_globalManager);
    m_widget->rootContext()->setContextProperty("terminalPage", this);

    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/terminal/terminalPage.qml"));
    m_root = m_widget->rootObject();

    processStart();
}

void TerminalPage::propertyGet(const QVariantMap &objects) {
    m_terminalItem = qvariant_cast<QObject *>(objects["terminalItem"]);
    auto *terminalItem = qobject_cast<QQuickItem *>(m_terminalItem);

    auto font = QFont(m_config["fontFamily"].toString(), m_config["fontSize"].toInt());
    font.setFixedPitch(true);
    font.setStyleHint(QFont::Monospace);

    m_terminalWidget = new TerminalWidget(terminalItem);
    m_terminalWidget->setParentItem(terminalItem);
    m_terminalWidget->setWidth(terminalItem->width());
    m_terminalWidget->setHeight(terminalItem->height());
    m_terminalWidget->fontSet(font);
    connect(terminalItem, &QQuickItem::widthChanged, m_terminalWidget, [this, terminalItem] {
        m_terminalWidget->setWidth(terminalItem->width());
    });
    connect(terminalItem, &QQuickItem::heightChanged, m_terminalWidget, [this, terminalItem] {
        m_terminalWidget->setHeight(terminalItem->height());
    });
    connect(m_terminalWidget, &TerminalWidget::resizeRequest, this, [this](const int rows, const int cols) {
        terminalResize(rows, cols);
    });
    connect(m_terminalWidget, &TerminalWidget::keyPressed, m_vtermWidget, &VtermWidget::keyPress);
    m_terminalWidget->forceActiveFocus();

    terminalRefresh();
}

void TerminalPage::terminalResize(const int rows, const int cols) const {
    if (!m_vtermWidget || rows < 1 || cols < 1) return;
    if (m_vtermWidget->rows() == rows && m_vtermWidget->cols() == cols) return;
    m_vtermWidget->resize(rows, cols);
    if (m_conptyWidget) m_conptyWidget->resize(rows, cols);
    terminalRefresh();
}

bool TerminalPage::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_widget && event->type() == QEvent::KeyPress) {
        const auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab) {
            if (m_vtermWidget) m_vtermWidget->keyPress(keyEvent->key(), keyEvent->modifiers(), "\t");
            return true;
        }
    }
    return DockWidget::eventFilter(watched, event);
}

// private
void TerminalPage::processStart() {
    if (!m_conptyWidget || !m_vtermWidget) return;
    const bool started = m_conptyWidget->start(
        m_session["program"].toString(),
        m_session["arguments"].toString(),
        g_workspaceUrl.toLocalFile(),
        m_vtermWidget->rows(),
        m_vtermWidget->cols()
    );
    if (!started) close();
}

bool TerminalPage::terminalRunning() const {
    return m_conptyWidget && m_conptyWidget->running();
}

void TerminalPage::terminalRefresh() const {
    if (m_terminalWidget && m_vtermWidget) {
        m_terminalWidget->cellsSet(m_vtermWidget->cells(), m_vtermWidget->rows(), m_vtermWidget->cols());
        m_terminalWidget->cursorSet(m_vtermWidget->cursor());
    }
}

void TerminalPage::processStop() {
    if (m_conptyWidget) m_conptyWidget->stop();
}
