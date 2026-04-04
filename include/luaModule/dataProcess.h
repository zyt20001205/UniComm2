#ifndef UNICOMM_DATAPROCESS_H
#define UNICOMM_DATAPROCESS_H

#include <QObject>
#include <sol/object.hpp>

class DataProcess final : public QObject {
    Q_OBJECT

public:
    explicit DataProcess(QObject *parent = nullptr);

    ~DataProcess() override = default;

    [[nodiscard]] static sol::table databaseList(sol::this_state ts);

    static void databaseWrite(const std::string &key, const sol::object &value);

    [[nodiscard]] static sol::table datatableList(sol::this_state ts);

    static void datatableWrite(const std::string &key, const sol::object &value);

    static void datatableExport(const std::string &fileName);
};

#endif //UNICOMM_DATAPROCESS_H
