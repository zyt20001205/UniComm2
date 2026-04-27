#include "terminal/gitModule.h"

#include <QFileInfo>
#include <QProcess>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QSettings>
#include <QTextDocument>

#include "globals.h"

// public
GitModule::GitModule()
    : DockWidget("Git"),
      m_config(g_workspaceConfig["gitConfig"].toObject()),
      m_widget(new QQuickWidget()),
      m_textDocument(new QTextDocument()),
      m_process(new QProcess(this)) {
    setWidget(m_widget);
    m_widget->installEventFilter(this);
}

GitModule::~GitModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void GitModule::propertySet(const QVariantMap &objects) {
    m_widget->rootContext()->setContextProperty("gitModule", this);
    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/terminal/gitModule.qml"));
    m_root = m_widget->rootObject();
}

void GitModule::propertyGet(const QVariantMap &objects) {
    m_textArea = qvariant_cast<QObject *>(objects["textArea"]);
    const auto font = QFont(m_config["fontFamily"].toString(), m_config["fontSize"].toInt());
    m_textArea->setProperty("font", font);
    processStart();
}

void GitModule::terminalInput(const QString &input) const {
    m_process->write(input.toLocal8Bit());
}

bool GitModule::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_widget && event->type() == QEvent::KeyPress) {
        const auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab) {
            return true;
        }
    }
    return DockWidget::eventFilter(watched, event);
}

// private
void GitModule::processStart() {
    QString installPath{};
    // x64
    installPath = QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\GitForWindows", QSettings::NativeFormat).value("installPath").toString();
    if (installPath.isEmpty()) {
        qDebug() << "git not found";
    } else {
        installPath += "\\bin\\bash.exe";
    }
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    m_process->setWorkingDirectory(g_workspaceUrl.toLocalFile());
    connect(m_process, &QProcess::readyRead, this, &GitModule::terminalOutput);
    m_process->start(installPath, {"--login"});
}

void GitModule::terminalOutput() const {
    const auto text = QString::fromLocal8Bit(m_process->readAllStandardOutput());
    QMetaObject::invokeMethod(m_root, "terminalOutput", Q_ARG(QVariant, QVariant::fromValue(text)));
}
