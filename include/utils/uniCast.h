#ifndef UNICOMM_UNICAST_H
#define UNICOMM_UNICAST_H

#include <QVariant>
#include <sol/object.hpp>

// sol -> qt
template<typename S, typename D>
[[nodiscard]] D uni_cast(const S& s, int depth = 0);

template<>
[[nodiscard]] QString uni_cast<sol::object, QString>(const sol::object &s, int depth);

template<>
[[nodiscard]] QVariant uni_cast<sol::object, QVariant>(const sol::object &s, int depth);

template<>
[[nodiscard]] QVariantList uni_cast<sol::variadic_args, QVariantList>(const sol::variadic_args &s, int depth);

// qt -> sol
template<typename S, typename D>
[[nodiscard]] D uni_cast(sol::this_state ts, const S& s, int depth = 0);

template<>
[[nodiscard]] sol::object uni_cast<QVariant, sol::object>(sol::this_state ts, const QVariant &s, int depth);

template<>
[[nodiscard]] sol::table uni_cast<QSet<QString>, sol::table>(sol::this_state ts, const QSet<QString> &s, int depth);

#endif //UNICOMM_UNICAST_H