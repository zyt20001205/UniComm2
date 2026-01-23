#ifndef UNICOMM_LUADATAPROCESS_H
#define UNICOMM_LUADATAPROCESS_H

#include <QObject>
#include "sol/object.hpp"

class LuaDataProcess final : public QObject {
    Q_OBJECT

public:
    explicit LuaDataProcess(QObject *parent = nullptr);

    ~LuaDataProcess() override = default;

    std::vector<std::string> databaseList();

    void databaseWrite(const std::string &key, const sol::object &value);

    std::vector<std::string> datatableList();

    void datatableWrite(const std::string &key, const sol::object &value);

signals:
    void listDatabase(QSet<QString> &databaseSet);

    void writeDatabase(const QString &key, const QString &value, bool &status);

    void listDatatable(QSet<QString> &datatableSet);

    void writeDatatable(const QString &key, const QString &value, bool &status);
};

// int lua_databaseList(lua_State *L);
//
// int lua_databaseWrite(lua_State *L);
//
// int lua_databaseClear(lua_State *L);
//
// int lua_datatableList(lua_State *L);
//
// int lua_datatableWrite(lua_State *L);
//
// int lua_datatableClear(lua_State *L);
//
// int lua_datatableExport(lua_State *L);
//
// int lua_dataplotAppend(lua_State *L);
//
// int lua_dataplotRemove(lua_State *L);

#endif //UNICOMM_LUADATAPROCESS_H
