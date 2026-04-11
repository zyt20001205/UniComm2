#include "api/lua/data.h"

#include <sol/table_core.hpp>

#include "globals.h"
#include "data/databaseModule.h"
#include "data/datatableModule.h"
#include "utils/uniCast.h"

Data::Data(QObject *parent)
    : QObject(parent) {
}

sol::table Data::databaseList(const sol::this_state ts) {
    QSet<QString> databaseSet{};
    QMetaObject::invokeMethod(g_database, [&databaseSet] {
        databaseSet = g_database->databaseList();
    }, Qt::BlockingQueuedConnection);
    return uni_cast<sol::table>(ts, databaseSet);
}

void Data::databaseWrite(const std::string &key, const sol::object &value) {
    bool status = false;
    const QString valueStr = uni_cast<QString>(value);
    QMetaObject::invokeMethod(g_database, [&status, &key, &valueStr] {
        status = g_database->databaseWrite(QString::fromStdString(key), valueStr);
    }, Qt::BlockingQueuedConnection);
    if (!status) {
        throw sol::error("failed to write to database key: " + key);
    }
}

sol::table Data::datatableList(const sol::this_state ts) {
    QSet<QString> datatableSet{};
    QMetaObject::invokeMethod(g_datatable, [&datatableSet] {
        datatableSet = g_datatable->datatableList();
    }, Qt::BlockingQueuedConnection);
    return uni_cast<sol::table>(ts, datatableSet);
}

void Data::datatableWrite(const std::string &key, const sol::object &value) {
    bool status = false;
    const QString valueStr = uni_cast<QString>(value);
    QMetaObject::invokeMethod(g_datatable, [&status, &key, &valueStr] {
        status = g_datatable->datatableWrite(QString::fromStdString(key), valueStr);
    }, Qt::BlockingQueuedConnection);
    if (!status) {
        throw sol::error("failed to write to datatable key: " + key);
    }
}

void Data::datatableExport(const std::string &fileName) {
    QMetaObject::invokeMethod(g_datatable, [&fileName] {
        g_datatable->datatableExport(QString::fromStdString(fileName));
    }, Qt::BlockingQueuedConnection);
}
