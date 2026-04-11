#include "core/configManager.h"

#include <QFileDialog>
#include <QJsonArray>
#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>

#include "globals.h"
#include "util/qtUtils.h"

// public
ConfigManager::ConfigManager(QWidget *parent)
    : QObject(parent) {
    workspaceInit();
}

int ConfigManager::mainConfigLoad() {
    if (const auto rootPath = QDir::current().filePath("config.json"); !QFile::exists(rootPath)) {
        mainConfigGenerate();
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "main config not found");
    }
    QFile mainConfig(QDir::current().filePath("config.json"));
    mainConfig.open(QIODevice::ReadOnly | QIODevice::Text);
    const QByteArray jsonData = mainConfig.readAll();
    mainConfig.close();
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (!jsonDoc.isObject()) return 1;
    const QJsonObject jsonObject = jsonDoc.object();
    auto workspaceUrlStr = jsonObject.value("workspace").toString();
    auto workspaceUrl = QUrl(workspaceUrlStr);
    if (workspaceUrlStr.isEmpty() || !QFileInfo::exists(workspaceUrl.toLocalFile())) {
        QString workspaceDir = QFileDialog::getExistingDirectory(
            nullptr,
            tr("Open Workspace"),
            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
        if (workspaceDir.isEmpty()) {
            return 1;
        }
        workspaceUrl = QUrl::fromLocalFile(workspaceDir);
    }
    const QJsonObject json{
        {"version", "1.0.0"},
        {"workspace", workspaceUrl.toString()},
    };
    const QJsonDocument doc(json);
    mainConfig.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    mainConfig.write(doc.toJson(QJsonDocument::Indented));
    mainConfig.close();

    g_workspaceUrl = workspaceUrl;
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "main config loaded");

    return 0;
}

void ConfigManager::workspaceInit() {
    const auto workspacePath = g_workspaceUrl.toLocalFile();
    // 1: config.json
    {
        const auto srcConfigPath = ":/config/config.json";
        const auto dstConfigPath = QDir(workspacePath).filePath("config.json");
        // copy if not found
        if (!QFile::exists(dstConfigPath)) {
            QFile::copy(srcConfigPath, dstConfigPath);
            QFile::setPermissions(dstConfigPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                              | QFileDevice::ReadUser | QFileDevice::WriteUser
                                              | QFileDevice::ReadGroup | QFileDevice::ReadOther);
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] .config.json copied").arg(timestamp);
        }
        // validate
        else {
            QFile config(dstConfigPath);
            if (!config.open(QIODevice::ReadOnly | QIODevice::Text)) return;
            const QByteArray jsonData = config.readAll();
            config.close();
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
            if (!jsonDoc.isObject()) return;
            const QJsonObject jsonObject = jsonDoc.object();
            // validate script config
            {
                QJsonObject scriptConfig = jsonObject["scriptConfig"].toObject();
                // clear invalid script url in script list
                QJsonArray validScriptList;
                for (const auto &value: scriptConfig["scriptList"].toArray()) {
                    if (const auto scriptUrl = QUrl(value.toString()); QFileInfo::exists(scriptUrl.toLocalFile())) {
                        validScriptList.append(value);
                    } else {
                        qDebug() << "invalid script url found in script list:" << scriptUrl;
                    }
                }
                scriptConfig["scriptList"] = validScriptList;
                // clear invalid script url in breakpoint hash
                QJsonObject breakpointHash = scriptConfig["breakpointHash"].toObject();
                QJsonObject validBreakpointHash;
                for (auto it = breakpointHash.begin(); it != breakpointHash.end(); ++it) {
                    if (const auto scriptUrl = QUrl(it.key()); QFileInfo::exists(scriptUrl.toLocalFile())) {
                        validBreakpointHash.insert(it.key(), it.value());
                    } else {
                        qDebug() << "invalid script url found in breakpoint hash:" << scriptUrl;
                    }
                }
                scriptConfig["breakpointHash"] = validBreakpointHash;
                // write valid script config back
                jsonObject["scriptConfig"] = scriptConfig;
            }
            g_workspaceConfig = jsonObject;
            const QJsonDocument doc(jsonObject);
            if (!config.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) return;
            config.write(doc.toJson(QJsonDocument::Indented));
            config.close();
        }
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] config loaded").arg(timestamp);
    }
    // 2: .luarc.json
    {
        const auto srcLuarcPath = ":/config/.luarc.json";
        const auto dstLuarcPath = QDir(workspacePath).filePath(".luarc.json");
        // copy if not found
        if (!QFile::exists(dstLuarcPath)) {
            QFile::copy(srcLuarcPath, dstLuarcPath);
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] .luarc.json copied").arg(timestamp);
        }
        // validate
        else if (fileHashCalc(srcLuarcPath) != fileHashCalc(dstLuarcPath)) {
            QFile::setPermissions(dstLuarcPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                             | QFileDevice::ReadUser | QFileDevice::WriteUser
                                             | QFileDevice::ReadGroup | QFileDevice::ReadOther);
            QFile::remove(dstLuarcPath);
            QFile::copy(srcLuarcPath, dstLuarcPath);
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] .luarc.json updated").arg(timestamp);
        }
    }
    // 3: lib dir
    {
        const auto srcLibDirPath = ":/lib";
        const auto dstLibDirPath = QDir(workspacePath).filePath("lib");
        // mkdir if not found
        if (QDir().mkdir(dstLibDirPath)) {
            emit appendLog("lib dir created", LOG_INFO);
            // logging
            const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] lib dir created").arg(timestamp);
        }

        for (const auto &fileName: QDir(srcLibDirPath).entryList(QDir::Files | QDir::NoDotAndDotDot)) {
            const auto srcLibFilePath = QDir(srcLibDirPath).filePath(fileName);
            const auto dstLibFilePath = QDir(dstLibDirPath).filePath(fileName);
            if (!QFile::exists(dstLibFilePath)) {
                QFile::copy(srcLibFilePath, dstLibFilePath);
                // logging
                QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
                qDebug() << QString("[%1] %2 copied").arg(timestamp, fileName);
            } else if (fileHashCalc(srcLibFilePath) != fileHashCalc(dstLibFilePath)) {
                QFile::setPermissions(dstLibFilePath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                                | QFileDevice::ReadUser | QFileDevice::WriteUser
                                                | QFileDevice::ReadGroup | QFileDevice::ReadOther);
                QFile::remove(dstLibFilePath);
                QFile::copy(srcLibFilePath, dstLibFilePath);
                // logging
                QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
                qDebug() << QString("[%1] %2 updated").arg(timestamp, fileName);
            }
        }
    }
}

void ConfigManager::workspaceConfigSave(const QUrl &configUrl) {
    QString configPath{};
    if (configUrl.isEmpty()) {
        const auto workspacePath = g_workspaceUrl.toLocalFile();
        configPath = QDir(workspacePath).filePath("config.json");
    } else {
        configPath = configUrl.toLocalFile();
    }
    if (QFile workspaceConfig(configPath); workspaceConfig.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        const auto fileUrl = QUrl::fromLocalFile(configPath);
        const QJsonDocument doc(g_workspaceConfig);
        workspaceConfig.write(doc.toJson(QJsonDocument::Indented));
        workspaceConfig.close();
        emit appendLog(QString("workspace saved to <a href='%1'>%2</a>").arg(fileUrl.toString(), fileUrl.toString()), LOG_INFO);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] workspace saved to %2").arg(timestamp, fileUrl.toString());
    } else {
        emit appendLog("workspace save failed", LOG_INFO);
        // logging
        const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] workspace save failed").arg(timestamp);
    }
}

// private
void ConfigManager::mainConfigGenerate() {
    if (QFile mainConfig(QDir::current().filePath("config.json")); mainConfig.open(QIODevice::WriteOnly | QIODevice::Text)) {
        const QJsonObject json{
            {"version", "1.0.0"},
            {"workspace", ""}
        };
        const QJsonDocument doc(json);
        mainConfig.write(doc.toJson(QJsonDocument::Indented));
        mainConfig.close();
    } else {
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "main config generation failed");
    }
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "main config generated");
}
