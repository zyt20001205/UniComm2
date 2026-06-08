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
    auto mainFile = QFile(QDir::current().filePath("config.json"));
    if (!mainFile.open(QIODevice::ReadOnly | QIODevice::Text)) return 1;
    auto mainData = mainFile.readAll();
    mainFile.close();
    auto mainDoc = QJsonDocument::fromJson(mainData);
    auto mainConfig = mainDoc.object();
    const auto workspaceUrlStr = mainConfig["workspace"].toString();
    auto workspaceUrl = QUrl(workspaceUrlStr);
    if (workspaceUrlStr.isEmpty() || !QFileInfo::exists(workspaceUrl.toLocalFile())) {
        const auto workspaceDir = QFileDialog::getExistingDirectory(
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
    mainConfig["workspace"] = workspaceUrl.toString();
    mainDoc = QJsonDocument(mainConfig);
    if (!mainFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) return 1;
    mainData = mainDoc.toJson(QJsonDocument::Indented);
    mainFile.write(mainData);
    mainFile.close();

    g_mainConfig = mainConfig;
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
        // validation
        {
            QFile config(dstConfigPath);
            if (!config.open(QIODevice::ReadOnly | QIODevice::Text)) return;
            const QByteArray jsonData = config.readAll();
            config.close();
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
            if (!jsonDoc.isObject()) return;
            const QJsonObject jsonObject = jsonDoc.object();
            // validation document config
            {
                QJsonObject documentConfig = jsonObject["documentConfig"].toObject();
                // remove invalid document url in document list
                QJsonArray validDocumentList;
                for (const auto &value: documentConfig["documentList"].toArray()) {
                    if (const auto documentUrl = QUrl(value.toString()); QFileInfo::exists(documentUrl.toLocalFile())) {
                        validDocumentList.append(value);
                    } else {
                        qDebug() << "invalid document url found in document list:" << documentUrl;
                    }
                }
                documentConfig["documentList"] = validDocumentList;
                // remove invalid document url in breakpoint hash
                QJsonObject breakpointHash = documentConfig["breakpointHash"].toObject();
                QJsonObject validBreakpointHash;
                for (auto it = breakpointHash.begin(); it != breakpointHash.end(); ++it) {
                    if (const auto documentUrl = QUrl(it.key()); QFileInfo::exists(documentUrl.toLocalFile())) {
                        validBreakpointHash.insert(it.key(), it.value());
                    } else {
                        qDebug() << "invalid document url found in breakpoint hash:" << documentUrl;
                    }
                }
                documentConfig["breakpointHash"] = validBreakpointHash;
                // write back
                jsonObject["documentConfig"] = documentConfig;
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
        // validation
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
            emit appendLog(LogLevel::Info, "lib dir created", "");
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
    // 4: llm dir
    {
        const auto llmDirPath = QDir(workspacePath).filePath("llm");
        // mkdir if not found
        if (QDir().mkdir(llmDirPath)) {
            emit appendLog(LogLevel::Info, "llm dir created", "");
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
        emit appendLog(LogLevel::Info, "workspace saved to", QString("<a href='%1'>%2</a>").arg(fileUrl.toString(), fileUrl.toString()));
    } else {
        emit appendLog(LogLevel::Info, "workspace save failed", "");
    }
}

// private
void ConfigManager::mainConfigGenerate() {
    if (QFile mainConfig(QDir::current().filePath("config.json")); mainConfig.open(QIODevice::WriteOnly | QIODevice::Text)) {
        const QJsonObject json{
            {"theme", 0},
            {"version", "0.2.0-alpha2"},
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
