#include "configModule.h"

#include <QDir>
#include <QFileDialog>
#include <QJsonArray>
#include <QStandardPaths>

#include "globals.h"

// ConfigModule public
ConfigModule::ConfigModule(QWidget *parent)
    : QObject(parent),
      m_configFile(QDir::current().filePath("config.json")) {
    if (m_configFile.exists()) {
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "config found");
        configLoad();
    } else {
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "config not found");
        configGenerate();
    }
}

void ConfigModule::configSave(const QString &filePath) {
    if (filePath.isEmpty()) {
        m_configFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        const QJsonDocument doc(g_config);
        m_configFile.write(doc.toJson());
        m_configFile.close();
        emit appendLog("workspace saved", "info");
        // logging
        const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] workspace saved").arg(timestamp);
    } else {
        if (QFile file(filePath); file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            const QJsonDocument doc(g_config);
            file.write(doc.toJson());
            file.close();
            emit appendLog(QString("workspace saved to %1").arg(filePath), "info");
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] workspace saved to %2").arg(timestamp, filePath);
        } else {
            emit appendLog("workspace save failed", "info");
            // logging
            const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] workspace save failed").arg(timestamp);
        }
    }
}

// ConfigModule private
void ConfigModule::configGenerate() {
    if (m_configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        const QJsonObject json{
            {
                "mainConfig", QJsonObject{
                    {"version", "1.0.0"},
                    {"workspace", ""},
                    {"geometry", ""},
                    {"state", ""}
                },
            },
            {
                "shortcutConfig", QJsonObject{
                    {"openWorkspace", "Ctrl+O"},
                    {"saveWorkspace", "Ctrl+S"},
                    {"saveWorkspaceAs", "Ctrl+Shift+S"}
                },
            },
            {
                "portConfig", QJsonArray{
                },
            },
            {
                "sendConfig", QJsonArray{
                },
            },
            {
                "databaseConfig", QJsonArray{
                },
            },
            {
                "datatableConfig", QJsonArray{
                },
            },
            {
                "scriptConfig", QJsonObject{
                    {"scriptList", QJsonArray{}},
                    {"breakpointHash", QJsonObject{}},
                    {"formatting", "Ctrl+Alt+L"},
                    {"fontFamily", "Consolas"},
                    {"fontSize", 12},
                    {"backgroundError", "#ffe6e6"},
                    {"backgroundWarning", "#fff5e6"},
                    {"backgroundInfo", "#e6f0fa"},
                    {"backgroundHint", "#f5f5f5"}
                },
            },
            {
                "logConfig", QJsonObject{
                    {"fontFamily", "Segoe UI"},
                    {"fontSize", 9},
                    {"timestamp", true},
                    {"height", 1000}
                },
            },
        };
        // load to g_config
        g_config = json;
        const QJsonDocument doc(json);
        m_configFile.write(doc.toJson(QJsonDocument::Indented));
        m_configFile.close();
    } else {
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "config generation failed");
    }
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "config generated");
}

void ConfigModule::configLoad() {
    m_configFile.open(QIODevice::ReadOnly | QIODevice::Text);
    const QByteArray jsonData = m_configFile.readAll();
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (!jsonDoc.isObject()) return;
    const QJsonObject jsonObject = jsonDoc.object();
    g_config = configValidate(jsonObject);
    m_configFile.close();
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "config loaded");
}

QJsonObject ConfigModule::configValidate(QJsonObject jsonObject) {
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
    // return validated json object
    return jsonObject;
}
