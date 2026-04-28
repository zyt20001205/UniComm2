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
    m_root->setProperty("gitEnabled", g_gitEnabled);
}

void GitModule::propertyGet(const QVariantMap &objects) {
    m_textArea = qvariant_cast<QObject *>(objects["textArea"]);
    const auto font = QFont(m_config["fontFamily"].toString(), m_config["fontSize"].toInt());
    m_textArea->setProperty("font", font);
    processStart();
}

void GitModule::terminalStdin(const QString &input) const {
    m_process->write(input.toLocal8Bit());
}

void GitModule::gitInit() {
    m_command = Init;
    QMetaObject::invokeMethod(m_root, "terminalStdin", Q_ARG(QVariant, "git init"));
}

void GitModule::gitCommit() {
    m_command = Commit;
    QMetaObject::invokeMethod(m_root, "terminalStdin", Q_ARG(QVariant, "git commit"));
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
    m_process->setWorkingDirectory(g_workspaceUrl.toLocalFile());
    connect(m_process, &QProcess::readyReadStandardOutput, this, &GitModule::terminalStdout);
    connect(m_process, &QProcess::readyReadStandardError, this, &GitModule::terminalStderr);
    m_process->start(installPath, {"--login"});
}

void GitModule::terminalStdout() {
    parser(true);
    const auto output = QString::fromLocal8Bit(m_process->readAllStandardOutput());
    QMetaObject::invokeMethod(m_root, "terminalStdout", Q_ARG(QVariant, output));
}

void GitModule::terminalStderr() {
    parser(false);
    const auto error = QString::fromLocal8Bit(m_process->readAllStandardError());
    QMetaObject::invokeMethod(m_root, "terminalStderr", Q_ARG(QVariant, error));
}

void GitModule::parser(const bool status) {
    switch (m_command) {
        case Init: {
            m_root->setProperty("gitEnabled", status);
            if (status) emit initGit(status);
        }
        break;
        default: break;
    }
}
