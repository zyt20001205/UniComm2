#ifndef UNICOMM_UNICAST_H
#define UNICOMM_UNICAST_H

#include <QVariant>
#include <sol/object.hpp>

template<typename D, typename S>
[[nodiscard]] D uni_cast(const S& s, int depth = 0);

template<typename D, typename S>
[[nodiscard]] D uni_cast(sol::this_state ts, const S& s, int depth = 0);

// luals -> qt
template<>
[[nodiscard]] QUrl uni_cast<QUrl, QString>(const QString &s, int depth);

// sol -> qt
template<>
[[nodiscard]] QString uni_cast<QString, sol::object>(const sol::object &s, int depth);

template<>
[[nodiscard]] QVariant uni_cast<QVariant, sol::object>(const sol::object &s, int depth);

template<>
[[nodiscard]] QVariantList uni_cast<QVariantList, sol::variadic_args>(const sol::variadic_args &s, int depth);

// qt -> sol
template<>
[[nodiscard]] sol::object uni_cast<sol::object, QVariant>(sol::this_state ts, const QVariant &s, int depth);

template<>
[[nodiscard]] sol::object uni_cast<sol::object, QVariantHash>(sol::this_state ts, const QVariantHash &s, int depth);

template<>
[[nodiscard]] sol::object uni_cast<sol::object, QVariantMap>(sol::this_state ts, const QVariantMap &s, int depth);

template<>
[[nodiscard]] sol::table uni_cast<sol::table, QSet<QString>>(sol::this_state ts, const QSet<QString> &s, int depth);

#endif //UNICOMM_UNICAST_H