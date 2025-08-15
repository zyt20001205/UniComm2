#include "../include/config.h"

QJsonObject g_config;

void Config::configInit() {
    if (configFile.exists()) {
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
    if (configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        const QJsonObject json{
            {"version", "1.0.0"},
            {
                "logConfig", QJsonObject{
                    {"timestamp", true},
                    {"scrollLock", false},
                    {"wrap", "no"},
                    {"length", 1000}
                },
            },
            {
                "portConfig", QJsonArray{},
            },
        };
        // load to g_config
        g_config = json;
        const QJsonDocument doc(json);
        configFile.write(doc.toJson(QJsonDocument::Indented));
        configFile.close();
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
    configFile.open(QIODevice::ReadOnly | QIODevice::Text);
    const QByteArray jsonData = configFile.readAll();
    if (const QJsonDocument doc = QJsonDocument::fromJson(jsonData); doc.isObject()) {
        g_config = doc.object();
    }
    configFile.close();
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "config loaded");
}
