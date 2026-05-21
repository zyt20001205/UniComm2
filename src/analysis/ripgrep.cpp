#include "analysis/ripgrep.h"

#include <QJsonArray>
#include <QProcess>

#include <globals.h>

// public
Ripgrep::Ripgrep(QWidget *parent)
    : QObject(parent) {
}

QJsonArray Ripgrep::grep(const QString &pattern) {
    QProcess rg{};
    QStringList args{};
    args << "--json" << pattern << g_workspaceUrl.toLocalFile();
    rg.start(QCoreApplication::applicationDirPath() + "/ripgrep/rg.exe", args);
    if (!rg.waitForFinished() || (rg.exitCode() != 0 && rg.exitCode() != 1)) return {};
    const auto out = rg.readAllStandardOutput();
    if (out.isEmpty()) return {};
    constexpr int MAX_MATCH = 50;
    int count = 0;
    QJsonArray result{};
    for (const auto &value : out.split('\n')) {
        if (value.isEmpty()) continue;
        const auto json = QJsonDocument::fromJson(value).object();
        if (json.value("type").toString() != "match") continue;
        count++;
        if (count > MAX_MATCH) return {QString("Too many matches (> %1). Please refine your search pattern.").arg(MAX_MATCH)};
        auto data = json.value("data").toObject();
        const auto path = data.value("path").toObject()["text"].toString();
        data.remove("path");
        data["url"] = QUrl::fromLocalFile(path).toString();
        result.append(data);
    }
    return result;
}
