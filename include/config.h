#ifndef CONFIG_H
#define CONFIG_H

#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

class Config {
public:
    Config();

    void configInit();

    void configSave();

private:
    void configGenerate();

    void configLoad();

    QFile m_configFile{};
};

#endif //CONFIG_H
