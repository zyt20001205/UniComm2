#include "luaModule/luaDataProcess.h"

#include "globals.h"
#include "dataModule/databaseModule.h"
#include "dataModule/datatableModule.h"
#include "utils/luaUtils.h"

LuaDataProcess::LuaDataProcess(QObject *parent)
    : QObject(parent) {
}

std::vector<std::string> LuaDataProcess::databaseList() {
    QSet<QString> databaseSet{};
    QMetaObject::invokeMethod(g_database, "databaseList", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(QSet<QString>, databaseSet));
    std::vector<std::string> databaseList{};
    for (const auto &key: databaseSet) {
        databaseList.push_back(key.toStdString());
    }
    return databaseList;
}

void LuaDataProcess::databaseWrite(const std::string &key, const sol::object &value) {
    QString valueStr{};
    switch (value.get_type()) {
        case sol::type::boolean: {
            valueStr = value.as<bool>() ? "true" : "false";
        }
        break;
        case sol::type::number: {
            if (value.is<int>()) {
                valueStr = QString::number(value.as<int>());
            } else {
                valueStr = QString::number(value.as<double>());
            }
        }
        break;
        case sol::type::string: {
            valueStr = QString::fromStdString(value.as<std::string>());
        }
        break;
        default: {
            valueStr = "nil";
        }
        break;
    }
    bool status = false;
    QMetaObject::invokeMethod(g_database, "databaseWrite", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(bool, status),
                              Q_ARG(QString, QString::fromStdString(key)),
                              Q_ARG(QString, valueStr));
    if (!status) {
        throw sol::error("failed to write to database key: " + key);
    }
}

std::vector<std::string> LuaDataProcess::datatableList() {
    QSet<QString> datatableSet{};
    QMetaObject::invokeMethod(g_datatable, "datatableList", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(QSet<QString>, datatableSet));
    std::vector<std::string> datatableList{};
    for (const auto &key: datatableSet) {
        datatableList.push_back(key.toStdString());
    }
    return datatableList;
}

void LuaDataProcess::datatableWrite(const std::string &key, const sol::object &value) {
    QString valueStr{};
    switch (value.get_type()) {
        case sol::type::boolean: {
            valueStr = value.as<bool>() ? "true" : "false";
        }
        break;
        case sol::type::number: {
            if (value.is<int>()) {
                valueStr = QString::number(value.as<int>());
            } else {
                valueStr = QString::number(value.as<double>());
            }
        }
        break;
        case sol::type::string: {
            valueStr = QString::fromStdString(value.as<std::string>());
        }
        break;
        default: {
            valueStr = "nil";
        }
        break;
    }
    bool status = false;
    QMetaObject::invokeMethod(g_datatable, "datatableWrite", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(bool, status),
                              Q_ARG(QString, QString::fromStdString(key)),
                              Q_ARG(QString, valueStr));
    if (!status) {
        throw sol::error("failed to write to datatable key: " + key);
    }
}

void LuaDataProcess::datatableExport(const std::string &fileName) {
    QMetaObject::invokeMethod(g_datatable, "datatableExport", Qt::QueuedConnection,
                              Q_ARG(QString, QString::fromStdString(fileName)));
}
