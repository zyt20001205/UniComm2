#ifndef UNICOMM_UNICAST_H
#define UNICOMM_UNICAST_H

#include <QVariant>
#include <sol/object.hpp>

template<typename S, typename D>
[[nodiscard]] D uni_cast(const S& s, int depth = 0);

template<>
[[nodiscard]] QString uni_cast<sol::object, QString>(const sol::object &s, int depth);

template<>
[[nodiscard]] QVariant uni_cast<sol::object, QVariant>(const sol::object &s, int depth);

template<>
[[nodiscard]] QVariantList uni_cast<sol::variadic_args, QVariantList>(const sol::variadic_args &s, int depth);

#endif //UNICOMM_UNICAST_H