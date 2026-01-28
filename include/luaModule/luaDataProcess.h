#ifndef UNICOMM_LUADATAPROCESS_H
#define UNICOMM_LUADATAPROCESS_H

#include <QObject>
#include "sol/object.hpp"

class QEventLoop;

class LuaDataProcess final : public QObject {
    Q_OBJECT

public:
    explicit LuaDataProcess(QObject *parent = nullptr);

    ~LuaDataProcess() override = default;

    std::vector<std::string> databaseList();

    void databaseWrite(const std::string &key, const sol::object &value);

    std::vector<std::string> datatableList();

    void datatableWrite(const std::string &key, const sol::object &value);

    void datatableExport(const std::string &fileName);

signals:
    void listDatabase(QSet<QString> &databaseSet);

    void writeDatabase(QEventLoop *eventloop, bool *status, const QString &key, const QString &value);

    void listDatatable(QSet<QString> &datatableSet);

    void writeDatatable(QEventLoop *eventloop, bool *status, const QString &key, const QString &value);

    void exportDatatable(const QString &fileName);
};

#endif //UNICOMM_LUADATAPROCESS_H
