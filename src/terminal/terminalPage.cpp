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
    m_terminalWidget->forceActiveFocus();

    terminalRefresh();
}

bool TerminalPage::terminalInput(const int key, const int modifiers, const QString &text) const {
    if (!m_vtermWidget) return false;

    int vtermModifiers = VTERM_MOD_NONE;
    if (modifiers & Qt::ShiftModifier) vtermModifiers |= VTERM_MOD_SHIFT;
    if (modifiers & Qt::AltModifier) vtermModifiers |= VTERM_MOD_ALT;
    if (modifiers & Qt::ControlModifier) vtermModifiers |= VTERM_MOD_CTRL;

    int vtermKey = VTERM_KEY_NONE;
    switch (key) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            vtermKey = VTERM_KEY_ENTER;
            break;
        case Qt::Key_Tab:
            vtermKey = VTERM_KEY_TAB;
            break;
        case Qt::Key_Backspace:
            vtermKey = VTERM_KEY_BACKSPACE;
            break;
        case Qt::Key_Escape:
            vtermKey = VTERM_KEY_ESCAPE;
            break;
        case Qt::Key_Up:
            vtermKey = VTERM_KEY_UP;
            break;
        case Qt::Key_Down:
            vtermKey = VTERM_KEY_DOWN;
            break;
        case Qt::Key_Left:
            vtermKey = VTERM_KEY_LEFT;
            break;
        case Qt::Key_Right:
            vtermKey = VTERM_KEY_RIGHT;
            break;
        case Qt::Key_Insert:
            vtermKey = VTERM_KEY_INS;
            break;
        case Qt::Key_Delete:
            vtermKey = VTERM_KEY_DEL;
            break;
        case Qt::Key_Home:
            vtermKey = VTERM_KEY_HOME;
            break;
        case Qt::Key_End:
            vtermKey = VTERM_KEY_END;
            break;
        case Qt::Key_PageUp:
            vtermKey = VTERM_KEY_PAGEUP;
            break;
        case Qt::Key_PageDown:
            vtermKey = VTERM_KEY_PAGEDOWN;
            break;
        default:
            if (key >= Qt::Key_F1 && key <= Qt::Key_F35) {
                vtermKey = VTERM_KEY_FUNCTION(key - Qt::Key_F1 + 1);
            }
            break;
    }

    if (vtermKey != VTERM_KEY_NONE) {
        terminalWrite(m_vtermWidget->keyboardKey(vtermKey, vtermModifiers));
        return true;
    }

    if (!text.isEmpty()) {
        const auto bytes = m_vtermWidget->keyboardUnichar(text, vtermModifiers);
        terminalWrite(bytes);
        return true;
    }
    return false;
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
            return terminalInput(Qt::Key_Tab, keyEvent->modifiers(), "\t");
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

void TerminalPage::terminalWrite(const QByteArray &bytes) const {
    if (m_conptyWidget) m_conptyWidget->write(bytes);
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
