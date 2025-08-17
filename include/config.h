#ifndef CONFIG_H
#define CONFIG_H

#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

extern QJsonObject g_config;

class Config {
public:
    Config() : m_configFile(QDir::current().filePath("config.json")) {
    }

    void configInit();

    void configSave();

private:
    void configGenerate();

    void configLoad();

    QFile m_configFile;
};

#endif //CONFIG_H
