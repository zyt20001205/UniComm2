#include "api/mqtt.h"

#include <sol/state_view.hpp>
#include <sol/table_core.hpp>

#include "util/uniCast.h"

Mqtt::Mqtt(QObject *parent)
    : QObject(parent) {
    m_options["protocol"] = "MQTT 3.1";
}

sol::object Mqtt::optionLog(const sol::this_state ts) const {
    return uni_cast<sol::object>(ts, m_options);
}

sol::object Mqtt::optionGet(const sol::this_state ts, const std::string &key) const {
    const auto _key = QString::fromStdString(key);
    if (!m_options.contains(_key)) throw sol::error("invalid mqtt option: " + key);
    return uni_cast<sol::object>(ts, m_options[_key]);
}

void Mqtt::optionSet(const std::string &key, const sol::object &value) {
    const auto _key = QString::fromStdString(key);
    if (!m_options.contains(_key)) throw sol::error("invalid mqtt option: " + key);
    m_options[_key] = uni_cast<QVariant>(value);
}

sol::table Mqtt::optionsProxy(const sol::this_state ts) const {
    sol::state_view lua(ts);
    sol::table table = lua.create_table();
    sol::table metatable = lua.create_table();
    metatable[sol::meta_function::pairs] = [this](const sol::this_state _ts) {
        return std::make_tuple(sol::state_view(_ts)["next"].get<sol::function>(), this->optionLog(_ts).as<sol::table>(), sol::nil);
    };
    metatable[sol::meta_function::index] = [this](const sol::table&, const std::string &key, const sol::this_state _ts) {
        return this->optionGet(_ts, key);
    };
    metatable[sol::meta_function::new_index] = [this](const sol::table&, const std::string &key, const sol::object &val) {
        const_cast<Mqtt *>(this)->optionSet(key, val);
    };
    table[sol::metatable_key] = metatable;
    return table;
}
