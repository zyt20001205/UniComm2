#ifndef UNICOMM_UNICAST_H
#define UNICOMM_UNICAST_H

#include <QColor>
#include <QIcon>
#include <QUrl>
#include <QVariant>
#include <sol/object.hpp>

#ifdef emit
#pragma push_macro("emit")
#undef emit
#define UNICOMM_UNICAST_RESTORE_QT_EMIT
#endif
#include <ghostty/vt/color.h>
#include <ghostty/vt/grid_ref.h>
#include <ghostty/vt/key/event.h>
#include <ghostty/vt/mouse/event.h>
#include <ghostty/vt/render.h>
#include <ghostty/vt/types.h>
#ifdef UNICOMM_UNICAST_RESTORE_QT_EMIT
#pragma pop_macro("emit")
#undef UNICOMM_UNICAST_RESTORE_QT_EMIT
#endif

struct TerminalCell;

struct LUrl {
    QString value;
    LUrl(QString s) : value(std::move(s)) {}
    operator QString() const { return value; }
};

struct LPath {
    QString value;
    LPath(QString s) : value(std::move(s)) {}
    operator QString() const { return value; }
};

struct QFileIcon {
    QUrl value;
    QFileIcon(QUrl s) : value(std::move(s)) {}
    operator QUrl() const { return value; }
};

struct QHtmlString {
    QString value;
    QHtmlString(QString s) : value(std::move(s)) {}
    operator QString() const { return value; }
};

struct QFullHtmlString {
    QString value;
    QFullHtmlString(QString s) : value(std::move(s)) {}
    operator QString() const { return value; }
};

struct GhosttyCellRef {
    const GhosttyGridRef *ref{};
    GhosttyColorRgb foreground{};
    GhosttyColorRgb background{};
    const GhosttyColorRgb *palette{};
};

struct GhosttyStaticString {
    const char *value{};
    GhosttyStaticString(const char *s) : value(s) {}
    operator const char *() const { return value; }
};

template<typename D, typename S>
[[nodiscard]] D uni_cast(const S& s, int depth = 0);

template<typename D, typename S>
[[nodiscard]] D uni_cast(sol::this_state ts, const S& s, int depth = 0);

// lua -> qt
template<>
[[nodiscard]] QUrl uni_cast<QUrl, LUrl>(const LUrl &s, int depth);

template<>
[[nodiscard]] QUrl uni_cast<QUrl, LPath>(const LPath &s, int depth);

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
[[nodiscard]] sol::object uni_cast<sol::object, QVariantMap>(sol::this_state ts, const QVariantMap &s, int depth);

template<>
[[nodiscard]] sol::object uni_cast<sol::object, QVariantList>(sol::this_state ts, const QVariantList &s, int depth);

template<>
[[nodiscard]] sol::object uni_cast<sol::object, QVariantHash>(sol::this_state ts, const QVariantHash &s, int depth);

template<>
[[nodiscard]] sol::table uni_cast<sol::table, QSet<QString>>(sol::this_state ts, const QSet<QString> &s, int depth);

// qt -> qt
template<>
[[nodiscard]] QFileIcon uni_cast<QFileIcon, QUrl>(const QUrl &s, int depth);

template<>
[[nodiscard]] QIcon uni_cast<QIcon, QUrl>(const QUrl &s, int depth);

template<>
[[nodiscard]] QHtmlString uni_cast<QHtmlString, QString>(const QString &s, int depth);

template<>
[[nodiscard]] QFullHtmlString uni_cast<QFullHtmlString, QString>(const QString &s, int depth);

// ghostty -> qt
template<>
[[nodiscard]] QColor uni_cast<QColor, GhosttyColorRgb>(const GhosttyColorRgb &s, int depth);

template<>
[[nodiscard]] int uni_cast<int, GhosttyRenderStateCursorVisualStyle>(const GhosttyRenderStateCursorVisualStyle &s, int depth);

template<>
[[nodiscard]] int uni_cast<int, GhosttyTerminal>(const GhosttyTerminal &s, int depth);

template<>
[[nodiscard]] TerminalCell uni_cast<TerminalCell, GhosttyCellRef>(const GhosttyCellRef &s, int depth);

template<>
[[nodiscard]] QString uni_cast<QString, GhosttyString>(const GhosttyString &s, int depth);

// qt -> ghostty
template<>
[[nodiscard]] GhosttyMods uni_cast<GhosttyMods, int>(const int &s, int depth);

template<>
[[nodiscard]] GhosttyKey uni_cast<GhosttyKey, int>(const int &s, int depth);

template<>
[[nodiscard]] GhosttyMouseButton uni_cast<GhosttyMouseButton, int>(const int &s, int depth);

template<>
[[nodiscard]] GhosttyString uni_cast<GhosttyString, GhosttyStaticString>(const GhosttyStaticString &s, int depth);

#endif //UNICOMM_UNICAST_H
