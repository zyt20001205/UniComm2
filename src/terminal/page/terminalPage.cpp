#include "terminal/page/terminalPage.h"

#include <QProcess>
#include <QQmlContext>
#include <QQuickWidget>
#include <QTextDocument>

#include "globals.h"
#include "terminal/logModule.h"

// public
TerminalPage::TerminalPage(const QString &uniqueName, const QJsonObject &config)
    : DockWidget(uniqueName),
      m_config(config),
      m_widget(new QQuickWidget()),
      m_textDocument(new QTextDocument()) {
    setWidget(m_widget);
}

TerminalPage::~TerminalPage() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 destructed").arg(timestamp, uniqueName());
}

void TerminalPage::propertySet(const QVariantMap &objects) {
    m_widget->rootContext()->setContextProperty("terminalPage", this);
    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/terminal/page/terminalPage.qml"));
}

void TerminalPage::propertyGet(const QVariantMap &objects) {
    m_textArea = qvariant_cast<QObject *>(objects["textArea"]);
    const auto font = QFont(m_config["fontFamily"].toString(), m_config["fontSize"].toInt());
    m_textArea->setProperty("font", font);
    m_textField = qvariant_cast<QObject *>(objects["textField"]);
}

void TerminalPage::start() {
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &TerminalPage::terminalOutput);
    connect(m_process, &QProcess::finished, this, &TerminalPage::close);
    m_process->start(m_processName, QStringList());
    g_log->addDockWidgetAsTab(this);
    open();
}

void TerminalPage::terminalInput(const QString &command) const {
    m_process->write((command + '\n').toLocal8Bit());
}

void TerminalPage::closeEvent(QCloseEvent *event) {
    if (m_process->state() == QProcess::Running) {
        terminalInput("exit");
        m_process->waitForFinished();
    }
    deleteLater();
    event->accept();
}

// private
void TerminalPage::terminalOutput() const {
    const auto text = QString::fromLocal8Bit(m_process->readAllStandardOutput());
    QMetaObject::invokeMethod(m_textArea, "append", Q_ARG(QString, text));
}
