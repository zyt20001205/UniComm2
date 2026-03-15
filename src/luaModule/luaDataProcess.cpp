#include "luaModule/luaDataProcess.h"

#include <QtConcurrent/QtConcurrent>
#include <QFuture>

#include "globals.h"
#include "dataModule/databaseModule.h"
#include "dataModule/datatableModule.h"
#include "utils/luaUtils.h"

LuaDataProcess::LuaDataProcess(QObject *parent)
    : QObject(parent) {
}

std::vector<std::string> LuaDataProcess::databaseList() {
    const auto future = QtConcurrent::run([] {
        return g_database->databaseList();
    });
    auto databaseSet = future.result();
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
    const auto future = QtConcurrent::run([&key, &valueStr] {
        return g_database->databaseWrite(QString::fromStdString(key), valueStr);
    });
    auto status = future.result();
    if (!status) {
        throw sol::error("failed to write to database key: " + key);
    }
}

std::vector<std::string> LuaDataProcess::datatableList() {
    const auto future = QtConcurrent::run([] {
        return g_datatable->datatableList();
    });
    auto datatableSet = future.result();
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
    const auto future = QtConcurrent::run([&key, &valueStr] {
        return g_datatable->datatableWrite(QString::fromStdString(key), valueStr);
    });
    auto status = future.result();
    if (!status) {
        throw sol::error("failed to write to datatable key: " + key);
    }
}

void LuaDataProcess::datatableExport(const std::string &fileName) {
    auto future = QtConcurrent::run([&fileName] {
        return g_datatable->datatableExport(QString::fromStdString(fileName));
    });
    future.waitForFinished();
}
