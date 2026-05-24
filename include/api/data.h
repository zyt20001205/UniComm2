#ifndef UNICOMM_DATA_H
#define UNICOMM_DATA_H

#include <QObject>
#include <sol/object.hpp>

class Data final : public QObject {
    Q_OBJECT

public:
    explicit Data(QObject *parent = nullptr);

    ~Data() override = default;

    [[nodiscard]] static sol::table databaseList(sol::this_state ts);

    static void databaseClear();

    static void databaseWrite(const std::string &key, const sol::object &value);

    [[nodiscard]] static sol::table datatableList(sol::this_state ts);

    static void datatableClear();

    static void datatableWrite(const std::string &key, const sol::object &value);

    static void datatableExport(const std::string &path);
};

#endif //UNICOMM_DATA_H
