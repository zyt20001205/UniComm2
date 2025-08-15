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
    Config(): configFile(QDir::current().filePath("config.json")) {
    }

    void configInit();

private:
    void configGenerate();

    void configLoad();

    QFile configFile;
};

#endif //CONFIG_H
