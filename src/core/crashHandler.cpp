#include "core/crashHandler.h"

#include <exchndl.h>
#include <QDir>

// public
void CrashHandler::init() {
    // SEH
    ExcHndlInit();
    // check if crash report dir exists
    const QString crashReportDirPath = QDir::current().filePath("crashReport");
    if (QDir().mkdir(crashReportDirPath)) {
        // logging
        const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] crash report dir created").arg(timestamp);
    }
    // joint crash report path
    const std::string crashReportPath = QDir(crashReportDirPath).filePath("crash_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")).toLocal8Bit().constData();
    ExcHndlSetLogFileNameA(crashReportPath.c_str());
}
