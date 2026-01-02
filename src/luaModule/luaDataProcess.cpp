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
    for (const auto& key:databaseSet) {
        databaseList.push_back(key.toStdString());
    }
    return databaseList;
}

void LuaDataProcess::databaseWrite(const std::string &key, const std::string &value) {
    bool status = false;
    emit writeDatabase(QString::fromStdString(key), QString::fromStdString(value), status);
    if (!status) {
        throw sol::error("failed to write to database key: " + key);
    }
}

std::vector<std::string> LuaDataProcess::datatableList() {
    QSet<QString> datatableSet{};
    emit listDatatable(datatableSet);
    std::vector<std::string> datatableList{};
    for (const auto& key:datatableSet) {
        datatableList.push_back(key.toStdString());
    }
    return datatableList;
}

void LuaDataProcess::datatableWrite(const std::string &key, const std::string &value) {
    bool status = false;
    emit writeDatatable(QString::fromStdString(key), QString::fromStdString(value), status);
    if (!status) {
        throw sol::error("failed to write to datatable key: " + key);
    }
}
