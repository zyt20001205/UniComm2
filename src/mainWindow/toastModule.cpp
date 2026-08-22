#include "mainWindow/toastModule.h"

#include <QQmlContext>
#include <QQuickItem>
#include <QScreen>
#include <QTimer>
#include <utility>

ToastModule::ToastModule(QQmlEngine *engine, QWindow &owner)
    : QQuickView(engine, nullptr),
      m_owner(owner) {
    setResizeMode(SizeViewToRootObject);
    setColor(Qt::transparent);
    setFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
    setTransientParent(&m_owner);

    connect(this, &QWindow::widthChanged, this, &ToastModule::geometryUpdate);
    connect(this, &QWindow::heightChanged, this, &ToastModule::geometryUpdate);
    connect(&m_owner, &QWindow::screenChanged, this, &ToastModule::geometryUpdate);
    connect(&m_owner, &QWindow::visibilityChanged, this, &ToastModule::geometryUpdate);

    rootContext()->setContextProperty("toastModule", this);
    setSource(QUrl("qrc:/qml/mainWindow/Toast.qml"));
    m_root = rootObject();
    hide();

    geometryUpdate();
    QTimer::singleShot(0, this, &ToastModule::geometryUpdate);
}

void ToastModule::show(const int level, const QString &title, const QString &text, const int duration,
                       const QString &actionText, std::function<void()> callback) {
    if (!m_root) return;

    int actionId = -1;
    if (!actionText.isEmpty() && callback) {
        actionId = m_actionId++;
        m_callbacks.insert(actionId, std::move(callback));
    }
    const bool invoked = QMetaObject::invokeMethod(
        m_root,
        "show",
        Q_ARG(int, level),
        Q_ARG(QString, title),
        Q_ARG(QString, text),
        Q_ARG(int, duration),
        Q_ARG(QString, actionText),
        Q_ARG(int, actionId)
    );
    if (!invoked) m_callbacks.remove(actionId);
}

void ToastModule::actionTrigger(const int actionId) {
    const auto callback = m_callbacks.take(actionId);
    if (callback) callback();
}

void ToastModule::actionRemove(const int actionId) {
    m_callbacks.remove(actionId);
}

void ToastModule::geometryUpdate() {
    auto *screen = m_owner.screen();
    if (!screen) {
        hide();
        return;
    }

    setScreen(screen);
    const auto geometry = screen->geometry();
    setPosition(
        geometry.x() + geometry.width() - width() - 20,
        geometry.y() + geometry.height() - height() - 20
    );

    const auto visibility = m_owner.visibility();
    const bool shouldShow = height() > 0
                            && visibility != Minimized
                            && visibility != Hidden;
    if (shouldShow) QQuickView::show();
    else hide();
}
