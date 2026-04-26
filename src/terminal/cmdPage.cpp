#include "terminal/cmdPage.h"

#include <QProcess>
#include <QQmlContext>
#include <QQuickWidget>
#include <QTextDocument>

#include "globals.h"

// public
CmdPage::CmdPage(const QString &uniqueName, const QJsonObject &config)
    : DockWidget(uniqueName),
      m_config(config),
      m_widget(new QQuickWidget()),
      m_textDocument(new QTextDocument()) {
    setWidget(m_widget);
}

CmdPage::~CmdPage() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 destructed").arg(timestamp, uniqueName());
}

void CmdPage::propertySet(const QVariantMap &objects) {
    m_widget->rootContext()->setContextProperty("cmdPage", this);
    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/terminal/cmdPage.qml"));
}

void CmdPage::propertyGet(const QVariantMap &objects) {
    m_textArea = qvariant_cast<QObject *>(objects["textArea"]);
    const auto font = QFont(m_config["fontFamily"].toString(), m_config["fontSize"].toInt());
    m_textArea->setProperty("font", font);
    m_textField = qvariant_cast<QObject *>(objects["textField"]);
}

void CmdPage::start() {
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &CmdPage::terminalOutput);
    m_process->start("cmd.exe", QStringList());
    open();
}

void CmdPage::terminalInput(const QString &command) const {
    m_process->write((command + '\n').toLocal8Bit());
}

// private
void CmdPage::terminalOutput() const {
    const auto text = QString::fromLocal8Bit(m_process->readAllStandardOutput());
    QMetaObject::invokeMethod(m_textArea, "append", Q_ARG(QString, text));
}
