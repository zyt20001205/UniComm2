#include "terminal/page/terminalPage.h"

#include <QProcess>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QTextDocument>

#include "globals.h"
#include "core/globalManager.h"

// public
TerminalPage::TerminalPage(const QString &uniqueName, const QJsonObject &config)
    : DockWidget(uniqueName),
      m_config(config),
      m_widget(new QQuickWidget()),
      m_textDocument(new QTextDocument()) {
    setWidget(m_widget);
    m_widget->installEventFilter(this);
}

TerminalPage::~TerminalPage() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 destructed").arg(timestamp, uniqueName());
}

void TerminalPage::propertySet(const QVariantHash &objects) {
    m_widget->rootContext()->setContextProperty("global", g_globalManager);
    m_widget->rootContext()->setContextProperty("terminalPage", this);
    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/terminal/page/terminalPage.qml"));
    m_root = m_widget->rootObject();
}

void TerminalPage::propertyGet(const QVariantMap &objects) {
    m_textArea = qvariant_cast<QObject *>(objects["textArea"]);
    const auto font = QFont(m_config["fontFamily"].toString(), m_config["fontSize"].toInt());
    m_textArea->setProperty("font", font);
    processStart();
}

bool TerminalPage::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_widget && event->type() == QEvent::KeyPress) {
        const auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab) {
            return true;
        }
    }
    return DockWidget::eventFilter(watched, event);
}

// protected
void TerminalPage::processStart() {
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    m_process->setWorkingDirectory(g_workspaceUrl.toLocalFile());
    connect(m_process, &QProcess::readyReadStandardOutput, this, &TerminalPage::terminalOutput);
    connect(m_process, &QProcess::finished, this, &TerminalPage::close);
    m_process->start(m_name, m_arguments);
}

void TerminalPage::terminalInput(const QString &input) const {
    m_process->write(input.toLocal8Bit());
}

// private
void TerminalPage::terminalOutput() const {
    const auto text = QString::fromLocal8Bit(m_process->readAllStandardOutput());
    QMetaObject::invokeMethod(m_root, "terminalOutput", Q_ARG(QVariant, QVariant::fromValue(text)));
}
