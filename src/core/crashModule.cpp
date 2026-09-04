#include "core/crashModule.h"

#include <exchndl.h>
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QProcess>
#include <QQmlContext>
#include <QQuickView>
#include <QStyleHints>
#include <QUrl>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// public
CrashModule::CrashModule() {
    ExcHndlInit();

    const QString crashReportDirPath = QDir::current().filePath("crashReport");
    if (!QDir().mkpath(crashReportDirPath)) return;

    const QString reportName = QString("crash_%1_%2")
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz"))
            .arg(QCoreApplication::applicationPid());
    const QString reportPath = QDir(crashReportDirPath).filePath(reportName);
    const std::wstring nativeReportPath = reportPath.toStdWString();
    if (!ExcHndlSetLogFileNameW(nativeReportPath.c_str())) return;

    QProcess::startDetached(
        QCoreApplication::applicationFilePath(),
        {ReporterArgument, QString::number(QCoreApplication::applicationPid()), reportPath},
        QDir::currentPath()
    );
}

bool CrashModule::reporterMode(const QStringList &arguments) {
    return arguments.size() == 4 && arguments[1] == ReporterArgument;
}

int CrashModule::reporterExec(const QStringList &arguments) {
    bool validPid{};
    const auto parentPid = arguments[2].toULongLong(&validPid);
    if (!validPid) return 1;

#ifdef Q_OS_WIN
    if (const HANDLE parentProcess = OpenProcess(SYNCHRONIZE, FALSE, parentPid)) {
        WaitForSingleObject(parentProcess, INFINITE);
        CloseHandle(parentProcess);
    }
#endif

    QFile reportFile(arguments[3]);
    if (!reportFile.open(QIODevice::ReadOnly | QIODevice::Text)) return 0;

    const QString report = QString::fromUtf8(reportFile.readAll());
    if (!report.contains("Error occurred on")) return 0;

    bool darkTheme{};
    QFile configFile(QDir::current().filePath("config.json"));
    if (configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const auto config = QJsonDocument::fromJson(configFile.readAll()).object();
        darkTheme = config["theme"].toInt() == DarkTheme;
    }

    if (darkTheme) {
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Dark);
    } else {
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
    }

    QQuickView window;
    window.setTitle(QCoreApplication::translate("CrashModule", "UniComm Crash Report"));
    window.setResizeMode(QQuickView::SizeRootObjectToView);
    window.setColor(darkTheme ? QColor("#292929") : QColor("#ffffff"));
    window.rootContext()->setContextProperty("crashReport", report);
    window.setSource(QUrl("qrc:/qml/core/crashWindow.qml"));
    window.resize(900, 600);
    window.show();
    return QCoreApplication::exec();
}
