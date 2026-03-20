#include "luaModule/luaDataProcess.h"

#include "globals.h"
#include "dataModule/databaseModule.h"
#include "dataModule/datatableModule.h"
#include "utils/uniCast.h"

LuaDataProcess::LuaDataProcess(QObject *parent)
    : QObject(parent) {
}

std::vector<std::string> LuaDataProcess::databaseList() {
    QSet<QString> databaseSet{};
    QMetaObject::invokeMethod(g_database, [&databaseSet] {
        databaseSet = g_database->databaseList();
    }, Qt::BlockingQueuedConnection);
    return uni_cast<QSet<QString>, std::vector<std::string>>(databaseSet);
}

void LuaDataProcess::databaseWrite(const std::string &key, const sol::object &value) {
    bool status = false;
    const QString valueStr = uni_cast<sol::object, QString>(value);
    QMetaObject::invokeMethod(g_database, [&status, &key, &valueStr] {
        status = g_database->databaseWrite(QString::fromStdString(key), valueStr);
    }, Qt::BlockingQueuedConnection);
    if (!status) {
        throw sol::error("failed to write to database key: " + key);
    }
}

std::vector<std::string> LuaDataProcess::datatableList() {
    QSet<QString> datatableSet{};
    QMetaObject::invokeMethod(g_datatable, [&datatableSet] {
        datatableSet = g_datatable->datatableList();
    }, Qt::BlockingQueuedConnection);
    return uni_cast<QSet<QString>, std::vector<std::string>>(datatableSet);
}

void LuaDataProcess::datatableWrite(const std::string &key, const sol::object &value) {
    bool status = false;
    const QString valueStr = uni_cast<sol::object, QString>(value);
    QMetaObject::invokeMethod(g_datatable, [&status, &key, &valueStr] {
        status = g_datatable->datatableWrite(QString::fromStdString(key), valueStr);
    }, Qt::BlockingQueuedConnection);
    if (!status) {
        throw sol::error("failed to write to datatable key: " + key);
    }
}

void LuaDataProcess::datatableExport(const std::string &fileName) {
    QMetaObject::invokeMethod(g_datatable, [&fileName] {
        g_datatable->datatableExport(QString::fromStdString(fileName));
    }, Qt::BlockingQueuedConnection);
}
