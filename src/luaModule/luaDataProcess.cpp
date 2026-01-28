#include "luaModule/luaDataProcess.h"

#include "globals.h"
#include "utils/luaUtils.h"

LuaDataProcess::LuaDataProcess(QObject *parent)
    : QObject(parent) {
}

std::vector<std::string> LuaDataProcess::databaseList() {
    QSet<QString> databaseSet{};
    emit listDatabase(databaseSet);
    std::vector<std::string> databaseList{};
    for (const auto &key: databaseSet) {
        databaseList.push_back(key.toStdString());
    }
    return databaseList;
}

void LuaDataProcess::databaseWrite(const std::string &key, const sol::object &value) {
    const auto eventloop = std::make_unique<QEventLoop>();
    const auto status = std::make_unique<bool>();
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
    emit writeDatabase(eventloop.get(), status.get(), QString::fromStdString(key), valueStr);
    eventloop->exec();
    if (!*status) {
        throw sol::error("failed to write to database key: " + key);
    }
}

std::vector<std::string> LuaDataProcess::datatableList() {
    QSet<QString> datatableSet{};
    emit listDatatable(datatableSet);
    std::vector<std::string> datatableList{};
    for (const auto &key: datatableSet) {
        datatableList.push_back(key.toStdString());
    }
    return datatableList;
}

void LuaDataProcess::datatableWrite(const std::string &key, const sol::object &value) {
    const auto eventloop = std::make_unique<QEventLoop>();
    const auto status = std::make_unique<bool>();
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
    emit writeDatatable(eventloop.get(), status.get(), QString::fromStdString(key), valueStr);
    eventloop->exec();
    if (!*status) {
        throw sol::error("failed to write to datatable key: " + key);
    }
}

void LuaDataProcess::datatableExport(const std::string &fileName) {
    emit exportDatatable(QString::fromStdString(fileName));
}
