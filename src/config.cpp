#include "../include/config.h"

QJsonObject g_config;

void Config::configInit() {
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

void Config::configGenerate() {
    if (m_configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        const QJsonObject json{
            {"version", "1.0.0"},
            {
                "shortcutConfig", QJsonObject{
                    {"save", "Ctrl+S"}
                },
            },
            {
                "portConfig", QJsonArray{
                },
            },
            {
                "sendConfig", ""
            },
            {
                "scriptConfig", ""
            },
            {
                "logConfig", QJsonObject{
                    {"timestamp", true},
                    {"scrollLock", false},
                    {"wrap", "no"},
                    {"length", 1000}
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

void Config::configLoad() {
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

void Config::configSave() {
    m_configFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    const QJsonDocument doc(g_config);
    m_configFile.write(doc.toJson());
    m_configFile.close();
    // logging
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "config saved");
}
