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
}

void ConfigModule::configInit() {
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
                    {
                        "scriptList", QJsonArray{
                        },
                    },
                    {"formatting", "Ctrl+Alt+L"},
                    {"fontFamily", "consolas"},
                    {"fontSize", "12"}
                },
            },
            {
                "logConfig", QJsonObject{
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
    if (const QJsonDocument doc = QJsonDocument::fromJson(jsonData); doc.isObject()) {
        g_config = doc.object();
    }
    m_configFile.close();
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "config loaded");
}
