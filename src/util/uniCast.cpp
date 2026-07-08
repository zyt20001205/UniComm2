#include "util/uniCast.h"

#include <QByteArray>
#include <QDir>
#include <QVector>
#include <sol/state_view.hpp>
#include <sol/table_core.hpp>
#include <sol/variadic_args.hpp>
#include <sol/userdata.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>

#ifdef emit
#pragma push_macro("emit")
#undef emit
#define UNICOMM_UNICAST_CPP_RESTORE_QT_EMIT
#endif
#include <ghostty/vt/render.h>
#include <ghostty/vt/terminal.h>
#ifdef UNICOMM_UNICAST_CPP_RESTORE_QT_EMIT
#pragma pop_macro("emit")
#undef UNICOMM_UNICAST_CPP_RESTORE_QT_EMIT
#endif

#include "cmark-gfm.h"
#include "cmark-gfm-core-extensions.h"
#include "cmark-gfm-extension_api.h"
#include "globals.h"
#include "core/globalManager.h"
#include "terminal/module/terminalTypes.h"

// lua -> qt
template<>
QUrl uni_cast<QUrl, LUrl>(const LUrl &s, const int depth) {
    Q_UNUSED(depth);
    auto uri = QUrl::fromPercentEncoding(s.value.toUtf8());
    if (uri.size() > 8) {
        const QChar drive = uri[8];
        if (drive.isLetter() && drive.isLower()) {
            uri[8] = drive.toUpper();
        }
    }
    return {uri};
}

template<>
QUrl uni_cast<QUrl, LPath>(const LPath &s, int depth) {
    Q_UNUSED(depth);
    const QDir workspaceDir(g_workspaceUrl.toLocalFile());
    const auto documentPath = workspaceDir.absoluteFilePath(s.value);
    const auto documentUrl = QUrl::fromLocalFile(documentPath);
    return documentUrl;
}

// sol -> qt
template<>
QString uni_cast<QString, sol::object>(const sol::object &s, const int depth) {
    Q_UNUSED(depth);
    switch (s.get_type()) {
        case sol::type::nil:
            return "nil";
        case sol::type::boolean:
            return s.as<bool>() ? "true" : "false";
        case sol::type::lightuserdata:
            return "lightuserdata";
        case sol::type::number:
            if (s.is<int>()) {
                return QString::number(s.as<int>());
            }
            return QString::number(s.as<double>());
        case sol::type::string:
            return QString::fromStdString(s.as<std::string>());
        case sol::type::table:
            return "{...}";
        case sol::type::function:
            return "function";
        case sol::type::userdata:
            return "userdata";
        case sol::type::thread:
            return "thread";
        default:
            return "?";
    }
}

template<>
QVariant uni_cast<QVariant, sol::object>(const sol::object &s, const int depth) {
    constexpr int MAX_DEPTH = 100;
    if (depth > MAX_DEPTH) {
        throw sol::error("Maximum recursion depth exceeded");
    }
    switch (s.get_type()) {
        case sol::type::nil: {
            return "nil";
        }
        case sol::type::boolean: {
            return s.as<bool>();
        }
        case sol::type::number: {
            if (s.is<int>()) {
                return s.as<int>();
            }
            return s.as<double>();
        }
        case sol::type::string: {
            const std::string str = s.as<std::string>();
            bool raw = false;
            for (const char ch: str) {
                if (ch == '\r' || ch == '\n' || ch == '\t' || ch == ' ') {
                    continue;
                }
                if ((ch >= 0x00 && ch <= 0x1F) || ch == 0x7F) {
                    raw = true;
                    break;
                }
            }
            if (raw) {
                const QByteArray byteArray(str.data(), static_cast<qsizetype>(str.size()));
                return byteArray.toHex(' ').toUpper();
            }
            QString string{};
            // try utf-8
            string = QString::fromUtf8(str.data(), static_cast<qsizetype>(str.size()));
            if (!string.contains(QChar::ReplacementCharacter)) {
                return string;
            }
            // try ascii
            string = QString::fromLatin1(str.data(), static_cast<qsizetype>(str.size()));
            return string;
        }
        case sol::type::table: {
            const auto table = s.as<sol::table>();
            QVariantMap map{};
            sol::state_view lua(table.lua_state());
            const auto pairs = lua["pairs"](table);
            const auto next = pairs.get<sol::function>(0);
            const auto data = pairs.get<sol::object>(1);
            auto currentKey = pairs.get<sol::object>(2);
            while (true) {
                auto pair = next(data, currentKey);
                auto key = pair.get<sol::object>(0);
                if (key.get_type() == sol::type::nil) break;
                auto value = pair.get<sol::object>(1);
                currentKey = key;
                QString key_str{};
                if (key.is<std::string>()) key_str = QString::fromStdString(key.as<std::string>());
                else if (key.is<int>()) key_str = QString::number(key.as<int>());
                else if (key.is<double>()) key_str = QString::number(key.as<double>());
                else continue;
                map[key_str] = uni_cast<QVariant>(value, depth + 1);
            }
            return QVariant::fromValue(map);
        }
        case sol::type::userdata: {
            const auto mapPtr = s.as<sol::userdata>().as<QVariantMap *>();
            return QVariant::fromValue(*mapPtr);
        }
        default: {
            qDebug() << "Unsupported Lua Type" << static_cast<int>(s.get_type());;
            return "?";
        }
    }
}

template<>
QVariantList uni_cast<QVariantList, sol::variadic_args>(const sol::variadic_args &s, const int depth) {
    Q_UNUSED(depth);
    QVariantList d{};
    for (const sol::object &arg: s) {
        d.append(uni_cast<QVariant>(arg));
    }
    return d;
}

// qt -> sol
template<>
sol::object uni_cast<sol::object, QVariant>(const sol::this_state ts, const QVariant &s, const int depth) {
    sol::state_view lua(ts);
    constexpr int MAX_DEPTH = 100;
    if (depth > MAX_DEPTH) {
        throw sol::error("Maximum recursion depth exceeded");
    }
    if (!s.isValid()) return sol::nil;
    switch (s.typeId()) {
        case QMetaType::Bool:
            return sol::make_object(lua, s.toBool());
        case QMetaType::Int:
            return sol::make_object(lua, s.toInt());
        case QMetaType::UInt:
            return sol::make_object(lua, s.toUInt());
        case QMetaType::LongLong:
            return sol::make_object(lua, s.toLongLong());
        case QMetaType::Double:
            return sol::make_object(lua, s.toDouble());
        case QMetaType::QVariantMap: {
            const auto _s = s.toMap();
            sol::table _d = lua.create_table();
            for (auto it = _s.constBegin(); it != _s.constEnd(); ++it) {
                const QString &key = it.key();
                const QVariant &value = it.value();
                _d[key.toStdString()] = uni_cast<sol::object>(ts, value, depth + 1);
            }
            return sol::make_object(lua, _d);
        }
        case QMetaType::QVariantList: {
            const auto _s = s.toList();
            sol::table _d = lua.create_table();
            for (int i = 0; i < _s.size(); ++i) {
                _d[i + 1] = uni_cast<sol::object>(ts, _s[i], depth + 1);
            }
            return sol::make_object(lua, _d);
        }
        case QMetaType::QString:
            return sol::make_object(lua, s.toString().toStdString());
        case QMetaType::QByteArray: {
            const auto ba = s.toByteArray();
            return sol::make_object(lua, std::string(ba.constData(), ba.size()));
        }
        case QMetaType::QVariantHash: {
            const auto _s = s.toHash();
            sol::table _d = lua.create_table();
            for (auto it = _s.constBegin(); it != _s.constEnd(); ++it) {
                const QString &key = it.key();
                const QVariant &value = it.value();
                _d[key.toStdString()] = uni_cast<sol::object>(ts, value, depth + 1);
            }
            return sol::make_object(lua, _d);
        }
        case QMetaType::Float:
            return sol::make_object(lua, s.toFloat());
        default:
            return sol::nil;
    }
    // Bool = 1, Int = 2, UInt = 3, LongLong = 4, ULongLong = 5, Double = 6, Long = 32, Short = 33, Char = 34, Char16 = 56, Char32 = 57, ULong = 35, UShort = 36, UChar = 37,
    // Float = 38, SChar = 40, Nullptr = 51, QCborSimpleType = 52, Void = 43, VoidStar = 31, QChar = 7, QString = 10, QByteArray = 12, QBitArray = 13, QDate = 14, QTime = 15,
    // QDateTime = 16, QUrl = 17, QLocale = 18, QRect = 19, QRectF = 20, QSize = 21, QSizeF = 22, QLine = 23, QLineF = 24, QPoint = 25, QPointF = 26, QEasingCurve = 29, QUuid = 30,
    // QVariant = 41, QRegularExpression = 44, QJsonValue = 45, QJsonObject = 46, QJsonArray = 47, QJsonDocument = 48, QCborValue = 53, QCborArray = 54, QCborMap = 55, Float16 = 63,
    // QModelIndex = 42, QPersistentModelIndex = 50, QObjectStar = 39, QVariantMap = 8, QVariantList = 9, QVariantHash = 28, QVariantPair = 58, QByteArrayList = 49, QStringList = 11,
    // QFont = 0x1000, QPixmap = 0x1001, QBrush = 0x1002, QColor = 0x1003, QPalette = 0x1004, QIcon = 0x1005, QImage = 0x1006, QPolygon = 0x1007, QRegion = 0x1008, QBitmap = 0x1009,
    // QCursor = 0x100a, QKeySequence = 0x100b, QPen = 0x100c, QTextLength = 0x100d, QTextFormat = 0x100e, QTransform = 0x1010, QMatrix4x4 = 0x1011, QVector2D = 0x1012,
    // QVector3D = 0x1013, QVector4D = 0x1014, QQuaternion = 0x1015, QPolygonF = 0x1016, QColorSpace = 0x1017, QSizePolicy = 0x2000,
}

template<>
sol::object uni_cast<sol::object, QVariantMap>(const sol::this_state ts, const QVariantMap &s, const int depth) {
    return uni_cast<sol::object, QVariant>(ts, QVariant::fromValue(s), depth);
}

template<>
sol::object uni_cast<sol::object, QVariantList>(const sol::this_state ts, const QVariantList &s, const int depth) {
    if (s.size() == 1) return uni_cast<sol::object, QVariant>(ts, s[0], depth);
    return uni_cast<sol::object, QVariant>(ts, QVariant::fromValue(s), depth);
}

template<>
sol::object uni_cast<sol::object, QVariantHash>(const sol::this_state ts, const QVariantHash &s, const int depth) {
    return uni_cast<sol::object, QVariant>(ts, QVariant::fromValue(s), depth);
}

template<>
sol::table uni_cast<sol::table, QSet<QString> >(const sol::this_state ts, const QSet<QString> &s, const int depth) {
    Q_UNUSED(depth);
    sol::state_view lua(ts);
    int index = 1;
    sol::table d = lua.create_table();
    for (const auto &value: s) {
        d[index++] = sol::make_object(lua, value.toStdString());
    }
    return d;
}

// qt -> qt
template<>
QFileIcon uni_cast<QFileIcon, QUrl>(const QUrl &s, const int depth) {
    Q_UNUSED(depth);
    const auto fileInfo = QFileInfo(s.toLocalFile());
    const auto suffix = fileInfo.suffix().toLower();
    const QStringList imageType = {"bmp", "gif", "ico", "jpeg", "jpg", "png", "svg", "tif", "tiff", "webp"};
    const QStringList libType = {"dll", "so"};
    if (imageType.contains(suffix)) return QUrl("qrc:/icon/fileTypeImage.svg");
    if (suffix == "bat") return QUrl("qrc:/icon/fileTypeBatch.svg");
    if (suffix == "csv") return QUrl("qrc:/icon/fileTypeCsv.svg");
    if (suffix == "exe") return QUrl("qrc:/icon/fileTypeExe.svg");
    if (suffix == "gitignore") return QUrl("qrc:/icon/fileTypeGit.svg");
    if (suffix == "json") return QUrl("qrc:/icon/fileTypeJson.svg");
    if (libType.contains(suffix)) return QUrl("qrc:/icon/fileTypeLib.svg");
    if (suffix == "lua") return QUrl("qrc:/icon/fileTypeLua.svg");
    if (suffix == "md") return QUrl("qrc:/icon/fileTypeMarkdown.svg");
    if (suffix == "pdf") return QUrl("qrc:/icon/fileTypePdf.svg");
    if (suffix == "ps1") return QUrl("qrc:/icon/fileTypePowershell.svg");
    if (suffix == "txt") return QUrl("qrc:/icon/fileTypeTxt.svg");
    if (fileInfo.isDir() && fileInfo.fileName() == ".git") return QUrl("qrc:/icon/fileTypeFolderGit.svg");
    if (fileInfo.isDir() && fileInfo.fileName() == ".idea") return QUrl("qrc:/icon/fileTypeFolderIntellij.svg");
    if (fileInfo.isDir()) return QUrl("qrc:/icon/fileTypeFolder.svg");
    return QUrl("qrc:/icon/fileTypeDefault.svg");
}

template<>
QIcon uni_cast<QIcon, QUrl>(const QUrl &s, const int depth) {
    Q_UNUSED(depth);
    const auto &source = ':' + uni_cast<QFileIcon>(s).value.path();
    return QIcon(source);
}

template<>
QHtmlString uni_cast<QHtmlString, QString>(const QString &s, const int depth) {
    Q_UNUSED(depth);
    const auto md = s.toUtf8();

    cmark_gfm_core_extensions_ensure_registered();
    constexpr int options = CMARK_OPT_UNSAFE | CMARK_OPT_FOOTNOTES;
    cmark_parser *parser = cmark_parser_new(options);
    const char *extensions[] = {
        "footnotes",
        "table",
        "strikethrough",
        "autolink",
        "tasklist",
    };
    for (const auto *name: extensions) {
        auto *extension = cmark_find_syntax_extension(name);
        if (extension) cmark_parser_attach_syntax_extension(parser, extension);
    }
    cmark_parser_feed(parser, md.constData(), md.size());
    cmark_node *document = cmark_parser_finish(parser);
    char *htmlChar = cmark_render_html(document, options, cmark_parser_get_syntax_extensions(parser));

    QString d = htmlChar ? QString::fromUtf8(htmlChar) : QString();
    if (htmlChar) free(htmlChar);
    cmark_node_free(document);
    cmark_parser_free(parser);
    if (!g_globalManager) return d;

    d.replace("<pre>", R"(<pre style='font-family: Consolas, monospace; background-color: @hover; border: 1px solid @stroke; padding: 8px; white-space: pre-wrap;'>)");
    d.replace("<pre ", R"(<pre style='font-family: Consolas, monospace; background-color: @hover; border: 1px solid @stroke; padding: 8px; white-space: pre-wrap;' )");
    d.replace("<code>", R"(<code style='font-family: Consolas, monospace; background-color: @hover; border: 1px solid @stroke; padding: 2px 4px; white-space: pre-wrap;'>)");
    d.replace("<code ", R"(<code style='font-family: Consolas, monospace; background-color: @hover; border: 1px solid @stroke; padding: 2px 4px; white-space: pre-wrap;' )");
    d.replace("<table>", R"(<table style='border-collapse: collapse; margin: 8px 0;'>)");
    d.replace("<table ", R"(<table style='border-collapse: collapse; margin: 8px 0;' )");
    d.replace("<td>", R"(<td style='border: 1px solid @stroke; padding: 4px 8px;'>)");
    d.replace("<td ", R"(<td style='border: 1px solid @stroke; padding: 4px 8px;' )");
    d.replace("<th>", R"(<th style='border: 1px solid @stroke; padding: 4px 8px; background-color: @hover; font-weight: 600;'>)");
    d.replace("<th ", R"(<th style='border: 1px solid @stroke; padding: 4px 8px; background-color: @hover; font-weight: 600;' )");
    d.replace("@hover", g_globalManager->backHoverGet());
    d.replace("@stroke", g_globalManager->strokeGet());
    return d;
}

template<>
QFullHtmlString uni_cast<QFullHtmlString, QString>(const QString &s, const int depth) {
    Q_UNUSED(depth);
    const auto md = s.toUtf8();

    cmark_gfm_core_extensions_ensure_registered();
    constexpr int options = CMARK_OPT_UNSAFE | CMARK_OPT_FOOTNOTES;
    cmark_parser *parser = cmark_parser_new(options);
    const char *extensions[] = {
        "footnotes",
        "table",
        "strikethrough",
        "autolink",
        "tasklist",
    };
    for (const auto *name: extensions) {
        auto *extension = cmark_find_syntax_extension(name);
        if (extension) cmark_parser_attach_syntax_extension(parser, extension);
    }
    cmark_parser_feed(parser, md.constData(), md.size());
    cmark_node *document = cmark_parser_finish(parser);
    char *htmlChar = cmark_render_html(document, options, cmark_parser_get_syntax_extensions(parser));

    const QString body = htmlChar ? QString::fromUtf8(htmlChar) : QString();
    if (htmlChar) free(htmlChar);
    cmark_node_free(document);
    cmark_parser_free(parser);

    auto d = QString(R"(<!doctype html>
<html>
<head>
<meta charset="utf-8">
<style>
html, body {
    margin: 0;
    min-height: 100%;
    color: @fore;
    background: @back;
    font-family: "Segoe UI", sans-serif;
    font-size: 14px;
}
body {
    padding: 12px;
}
a {
    color: @link;
}
pre, code {
    font-family: Consolas, monospace;
}
code {
    padding: 2px 4px;
    border: 1px solid @stroke;
    border-radius: 4px;
    background: @hover;
}
pre {
    overflow: auto;
    padding: 8px;
    background: @hover;
}
pre code {
    padding: 0;
    border: 0;
    background: transparent;
}
table {
    border-collapse: collapse;
    margin: 8px 0;
}
th, td {
    border: 1px solid @stroke;
    padding: 4px 8px;
}
th {
    background: @hover;
    font-weight: 600;
}
tr:nth-child(even) td {
    background: @selected;
}
img {
    max-width: 100%;
}
.mermaid {
    overflow: auto;
}
.mermaid svg {
    max-width: 100%;
}
.hljs {
    background: transparent;
}
</style>
<link rel="stylesheet" href="http://unicomm/@highlightTheme">
<script src="http://unicomm/mermaid.min.js"></script>
<script src="http://unicomm/highlight.min.js"></script>
<script>
document.addEventListener("DOMContentLoaded", async () => {
    const blocks = document.querySelectorAll("pre > code.language-mermaid, pre > code.mermaid");

    if (window.hljs) {
        document.querySelectorAll("pre > code").forEach((code) => {
            if (code.classList.contains("language-mermaid") || code.classList.contains("mermaid")) return;
            hljs.highlightElement(code);
        });
    }

    if (window.mermaid && blocks.length > 0) {
        blocks.forEach((code) => {
            const diagram = document.createElement("div");
            diagram.className = "mermaid";
            diagram.textContent = code.textContent;
            code.parentElement.replaceWith(diagram);
        });

        mermaid.initialize({
            startOnLoad: false,
            securityLevel: "loose",
            theme: "@mermaidTheme"
        });
        await mermaid.run({ querySelector: ".mermaid" });
    }
});
</script>
</head>
<body>@body</body>
</html>)");
    d.replace("@fore", g_globalManager->foreGet());
    d.replace("@back", g_globalManager->backGet());
    d.replace("@link", g_globalManager->brandLinkGet());
    d.replace("@hover", g_globalManager->backHoverGet());
    d.replace("@stroke", g_globalManager->strokeGet());
    d.replace("@selected", g_globalManager->backSelectedGet());
    d.replace("@mermaidTheme", g_globalManager->themeGet() == Theme::Light ? "default" : "dark");
    d.replace("@highlightTheme", g_globalManager->themeGet() == Theme::Light ? "highlight-light.min.css" : "highlight-dark.min.css");
    d.replace("@body", body);
    return d;
}

namespace {
    [[nodiscard]] QColor faintColor(const QColor &foreground, const QColor &background) {
        return {
            (foreground.red() * 55 + background.red() * 45) / 100,
            (foreground.green() * 55 + background.green() * 45) / 100,
            (foreground.blue() * 55 + background.blue() * 45) / 100
        };
    }

    [[nodiscard]] QColor ghosttyStyleColor(
        const GhosttyStyleColor &color,
        const GhosttyColorRgb &fallback,
        const GhosttyColorRgb *palette
    ) {
        switch (color.tag) {
            case GHOSTTY_STYLE_COLOR_RGB:
                return uni_cast<QColor>(color.value.rgb);
            case GHOSTTY_STYLE_COLOR_PALETTE:
                return uni_cast<QColor>(palette[color.value.palette]);
            case GHOSTTY_STYLE_COLOR_NONE:
            default:
                return uni_cast<QColor>(fallback);
        }
    }

    [[nodiscard]] QString ghosttyGridText(const GhosttyGridRef &ref) {
        std::array<uint32_t, 16> graphemes{};
        size_t graphemeCount{};
        GhosttyResult result = ghostty_grid_ref_graphemes(&ref, graphemes.data(), graphemes.size(), &graphemeCount);
        if (result == GHOSTTY_OUT_OF_SPACE && graphemeCount > 0) {
            QVector<uint32_t> rawGraphemes;
            rawGraphemes.resize(static_cast<qsizetype>(graphemeCount));
            result = ghostty_grid_ref_graphemes(&ref, rawGraphemes.data(), static_cast<size_t>(rawGraphemes.size()), &graphemeCount);
            if (result != GHOSTTY_SUCCESS || graphemeCount == 0) return {};

            QVector<char32_t> text;
            text.reserve(static_cast<qsizetype>(graphemeCount));
            for (size_t index = 0; index < graphemeCount; ++index) {
                text.append(static_cast<char32_t>(rawGraphemes[static_cast<qsizetype>(index)]));
            }
            return QString::fromUcs4(text.constData(), static_cast<qsizetype>(text.size()));
        }

        if (result != GHOSTTY_SUCCESS || graphemeCount == 0) return {};

        QVector<char32_t> text;
        text.reserve(static_cast<qsizetype>(graphemeCount));
        for (size_t index = 0; index < graphemeCount; ++index) text.append(static_cast<char32_t>(graphemes[index]));
        return QString::fromUcs4(text.constData(), static_cast<qsizetype>(text.size()));
    }

    [[nodiscard]] QString ghosttyGridHyperlink(const GhosttyGridRef &ref) {
        std::array<uint8_t, 256> bytes{};
        size_t byteCount{};
        GhosttyResult result = ghostty_grid_ref_hyperlink_uri(&ref, bytes.data(), bytes.size(), &byteCount);
        if (result == GHOSTTY_OUT_OF_SPACE && byteCount > 0) {
            QByteArray dynamic;
            dynamic.resize(static_cast<qsizetype>(byteCount));
            result = ghostty_grid_ref_hyperlink_uri(&ref, reinterpret_cast<uint8_t *>(dynamic.data()), static_cast<size_t>(dynamic.size()), &byteCount);
            if (result != GHOSTTY_SUCCESS || byteCount == 0) return {};

            dynamic.resize(static_cast<qsizetype>(byteCount));
            return QString::fromUtf8(dynamic.constData(), dynamic.size());
        }

        if (result != GHOSTTY_SUCCESS || byteCount == 0) return {};
        return QString::fromUtf8(reinterpret_cast<const char *>(bytes.data()), static_cast<qsizetype>(byteCount));
    }

    [[nodiscard]] QString ghosttyRenderCellText(const GhosttyRenderStateRowCells cells) {
        std::array<uint8_t, 64> storage{};
        GhosttyBuffer buffer{storage.data(), storage.size(), 0};
        GhosttyResult result = ghostty_render_state_row_cells_get(cells, GHOSTTY_RENDER_STATE_ROW_CELLS_DATA_GRAPHEMES_UTF8, &buffer);
        if (result == GHOSTTY_OUT_OF_SPACE && buffer.len > 0) {
            QByteArray dynamic;
            dynamic.resize(static_cast<qsizetype>(buffer.len));
            GhosttyBuffer dynamicBuffer{reinterpret_cast<uint8_t *>(dynamic.data()), static_cast<size_t>(dynamic.size()), 0};
            result = ghostty_render_state_row_cells_get(cells, GHOSTTY_RENDER_STATE_ROW_CELLS_DATA_GRAPHEMES_UTF8, &dynamicBuffer);
            if (result != GHOSTTY_SUCCESS || dynamicBuffer.len == 0) return {};

            dynamic.resize(static_cast<qsizetype>(dynamicBuffer.len));
            return QString::fromUtf8(dynamic.constData(), dynamic.size());
        }

        if (result != GHOSTTY_SUCCESS || buffer.len == 0) return {};
        return QString::fromUtf8(reinterpret_cast<const char *>(storage.data()), static_cast<qsizetype>(buffer.len));
    }
}

// ghostty -> qt
template<>
QColor uni_cast<QColor, GhosttyColorRgb>(const GhosttyColorRgb &s, const int depth) {
    Q_UNUSED(depth);
    return {s.r, s.g, s.b};
}

template<>
int uni_cast<int, GhosttyRenderStateCursorVisualStyle>(const GhosttyRenderStateCursorVisualStyle &s, const int depth) {
    Q_UNUSED(depth);
    switch (s) {
        case GHOSTTY_RENDER_STATE_CURSOR_VISUAL_STYLE_BLOCK:
        case GHOSTTY_RENDER_STATE_CURSOR_VISUAL_STYLE_BLOCK_HOLLOW:
            return TerminalCursorShape::Block;
        case GHOSTTY_RENDER_STATE_CURSOR_VISUAL_STYLE_UNDERLINE:
            return TerminalCursorShape::Underline;
        case GHOSTTY_RENDER_STATE_CURSOR_VISUAL_STYLE_BAR:
        default:
            return TerminalCursorShape::BarLeft;
    }
}

template<>
int uni_cast<int, GhosttyTerminal>(const GhosttyTerminal &s, const int depth) {
    Q_UNUSED(depth);
    bool any = false;
    bool button = false;
    bool normal = false;
    bool x10 = false;
    ghostty_terminal_mode_get(s, GHOSTTY_MODE_ANY_MOUSE, &any);
    ghostty_terminal_mode_get(s, GHOSTTY_MODE_BUTTON_MOUSE, &button);
    ghostty_terminal_mode_get(s, GHOSTTY_MODE_NORMAL_MOUSE, &normal);
    ghostty_terminal_mode_get(s, GHOSTTY_MODE_X10_MOUSE, &x10);
    if (any) return TerminalMouseMode::Move;
    if (button) return TerminalMouseMode::Drag;
    if (normal || x10) return TerminalMouseMode::Click;
    return TerminalMouseMode::None;
}

template<>
TerminalCell uni_cast<TerminalCell, GhosttyCellRef>(const GhosttyCellRef &s, const int depth) {
    Q_UNUSED(depth);

    GhosttyColorRgb defaultPalette[256]{};
    const GhosttyColorRgb *palette = s.palette;
    if (!palette) {
        ghostty_color_palette_default(defaultPalette);
        palette = defaultPalette;
    }

    TerminalCell d{};
    d.width = 1;
    d.text = QStringLiteral(" ");
    d.foreground = uni_cast<QColor>(s.foreground);
    d.background = uni_cast<QColor>(s.background);

    if (!s.ref) return d;
    const GhosttyGridRef &ref = *s.ref;

    GhosttyCell raw{};
    if (ghostty_grid_ref_cell(&ref, &raw) == GHOSTTY_SUCCESS) {
        GhosttyCellWide wide = GHOSTTY_CELL_WIDE_NARROW;
        if (ghostty_cell_get(raw, GHOSTTY_CELL_DATA_WIDE, &wide) == GHOSTTY_SUCCESS) {
            switch (wide) {
                case GHOSTTY_CELL_WIDE_WIDE:
                    d.width = 2;
                    break;
                case GHOSTTY_CELL_WIDE_SPACER_TAIL:
                    d.width = 0;
                    d.text.clear();
                    break;
                default:
                    break;
            }
        }

        GhosttyCellContentTag contentTag = GHOSTTY_CELL_CONTENT_CODEPOINT;
        if (ghostty_cell_get(raw, GHOSTTY_CELL_DATA_CONTENT_TAG, &contentTag) == GHOSTTY_SUCCESS) {
            if (contentTag == GHOSTTY_CELL_CONTENT_BG_COLOR_RGB) {
                GhosttyColorRgb background{};
                if (ghostty_cell_get(raw, GHOSTTY_CELL_DATA_COLOR_RGB, &background) == GHOSTTY_SUCCESS) {
                    d.background = uni_cast<QColor>(background);
                }
            } else if (contentTag == GHOSTTY_CELL_CONTENT_BG_COLOR_PALETTE) {
                GhosttyColorPaletteIndex index{};
                if (ghostty_cell_get(raw, GHOSTTY_CELL_DATA_COLOR_PALETTE, &index) == GHOSTTY_SUCCESS) {
                    d.background = uni_cast<QColor>(palette[index]);
                }
            }
        }
    }

    const QString text = ghosttyGridText(ref);
    if (!text.isEmpty()) d.text = text;
    d.hyperlink = ghosttyGridHyperlink(ref);

    GhosttyStyle style{};
    style.size = sizeof(GhosttyStyle);
    if (ghostty_grid_ref_style(&ref, &style) == GHOSTTY_SUCCESS) {
        d.foreground = ghosttyStyleColor(style.fg_color, s.foreground, palette);
        if (style.bg_color.tag != GHOSTTY_STYLE_COLOR_NONE) d.background = ghosttyStyleColor(style.bg_color, s.background, palette);
        d.bold = style.bold;
        d.faint = style.faint;
        d.italic = style.italic;
        d.underline = style.underline != 0;
        d.strike = style.strikethrough;
        if (style.inverse) std::swap(d.foreground, d.background);
        if (style.invisible) d.foreground = d.background;
    }

    if (d.faint) d.foreground = faintColor(d.foreground, d.background);
    return d;
}

template<>
TerminalCell uni_cast<TerminalCell, GhosttyRenderCellRef>(const GhosttyRenderCellRef &s, const int depth) {
    Q_UNUSED(depth);

    GhosttyRenderStateColors defaultColors{};
    defaultColors.size = sizeof(GhosttyRenderStateColors);
    defaultColors.foreground = {240, 240, 240};
    defaultColors.background = {0, 0, 0};
    ghostty_color_palette_default(defaultColors.palette);

    const GhosttyRenderStateColors &colors = s.colors ? *s.colors : defaultColors;

    TerminalCell d{};
    d.width = 1;
    d.text = QStringLiteral(" ");
    d.foreground = uni_cast<QColor>(colors.foreground);
    d.background = uni_cast<QColor>(colors.background);

    if (!s.cells) return d;

    GhosttyCell raw{};
    if (ghostty_render_state_row_cells_get(s.cells, GHOSTTY_RENDER_STATE_ROW_CELLS_DATA_RAW, &raw) == GHOSTTY_SUCCESS) {
        GhosttyCellWide wide = GHOSTTY_CELL_WIDE_NARROW;
        if (ghostty_cell_get(raw, GHOSTTY_CELL_DATA_WIDE, &wide) == GHOSTTY_SUCCESS) {
            switch (wide) {
                case GHOSTTY_CELL_WIDE_WIDE:
                    d.width = 2;
                    break;
                case GHOSTTY_CELL_WIDE_SPACER_TAIL:
                    d.width = 0;
                    d.text.clear();
                    break;
                default:
                    break;
            }
        }
    }

    const QString text = ghosttyRenderCellText(s.cells);
    if (!text.isEmpty()) d.text = text;

    GhosttyColorRgb foreground{};
    if (ghostty_render_state_row_cells_get(s.cells, GHOSTTY_RENDER_STATE_ROW_CELLS_DATA_FG_COLOR, &foreground) == GHOSTTY_SUCCESS) {
        d.foreground = uni_cast<QColor>(foreground);
    }

    GhosttyColorRgb background{};
    if (ghostty_render_state_row_cells_get(s.cells, GHOSTTY_RENDER_STATE_ROW_CELLS_DATA_BG_COLOR, &background) == GHOSTTY_SUCCESS) {
        d.background = uni_cast<QColor>(background);
    }

    GhosttyStyle style{};
    style.size = sizeof(GhosttyStyle);
    if (ghostty_render_state_row_cells_get(s.cells, GHOSTTY_RENDER_STATE_ROW_CELLS_DATA_STYLE, &style) == GHOSTTY_SUCCESS) {
        d.bold = style.bold;
        d.faint = style.faint;
        d.italic = style.italic;
        d.underline = style.underline != 0;
        d.strike = style.strikethrough;
        if (style.inverse) std::swap(d.foreground, d.background);
        if (style.invisible) d.foreground = d.background;
    }

    if (d.faint) d.foreground = faintColor(d.foreground, d.background);
    return d;
}

template<>
QString uni_cast<QString, GhosttyString>(const GhosttyString &s, const int depth) {
    Q_UNUSED(depth);
    if (!s.ptr || s.len == 0) return {};
    return QString::fromUtf8(reinterpret_cast<const char *>(s.ptr), static_cast<qsizetype>(s.len));
}

// qt -> ghostty
template<>
GhosttyMods uni_cast<GhosttyMods, int>(const int &s, const int depth) {
    Q_UNUSED(depth);
    GhosttyMods d = 0;
    if (s & Qt::ShiftModifier) d |= GHOSTTY_MODS_SHIFT;
    if (s & Qt::ControlModifier) d |= GHOSTTY_MODS_CTRL;
    if (s & Qt::AltModifier) d |= GHOSTTY_MODS_ALT;
    if (s & Qt::MetaModifier) d |= GHOSTTY_MODS_SUPER;
    return d;
}

template<>
GhosttyKey uni_cast<GhosttyKey, int>(const int &s, const int depth) {
    Q_UNUSED(depth);
    if (s >= Qt::Key_A && s <= Qt::Key_Z) return static_cast<GhosttyKey>(GHOSTTY_KEY_A + s - Qt::Key_A);
    if (s >= Qt::Key_0 && s <= Qt::Key_9) return static_cast<GhosttyKey>(GHOSTTY_KEY_DIGIT_0 + s - Qt::Key_0);
    if (s >= Qt::Key_F1 && s <= Qt::Key_F25) return static_cast<GhosttyKey>(GHOSTTY_KEY_F1 + s - Qt::Key_F1);

    switch (s) {
        case Qt::Key_QuoteLeft: return GHOSTTY_KEY_BACKQUOTE;
        case Qt::Key_Backslash: return GHOSTTY_KEY_BACKSLASH;
        case Qt::Key_BracketLeft: return GHOSTTY_KEY_BRACKET_LEFT;
        case Qt::Key_BracketRight: return GHOSTTY_KEY_BRACKET_RIGHT;
        case Qt::Key_Comma: return GHOSTTY_KEY_COMMA;
        case Qt::Key_Equal: return GHOSTTY_KEY_EQUAL;
        case Qt::Key_Minus: return GHOSTTY_KEY_MINUS;
        case Qt::Key_Period: return GHOSTTY_KEY_PERIOD;
        case Qt::Key_Apostrophe: return GHOSTTY_KEY_QUOTE;
        case Qt::Key_Semicolon: return GHOSTTY_KEY_SEMICOLON;
        case Qt::Key_Slash: return GHOSTTY_KEY_SLASH;
        case Qt::Key_Return:
        case Qt::Key_Enter: return GHOSTTY_KEY_ENTER;
        case Qt::Key_Backspace: return GHOSTTY_KEY_BACKSPACE;
        case Qt::Key_Tab:
        case Qt::Key_Backtab: return GHOSTTY_KEY_TAB;
        case Qt::Key_Escape: return GHOSTTY_KEY_ESCAPE;
        case Qt::Key_Space: return GHOSTTY_KEY_SPACE;
        case Qt::Key_Insert: return GHOSTTY_KEY_INSERT;
        case Qt::Key_Delete: return GHOSTTY_KEY_DELETE;
        case Qt::Key_Home: return GHOSTTY_KEY_HOME;
        case Qt::Key_End: return GHOSTTY_KEY_END;
        case Qt::Key_PageUp: return GHOSTTY_KEY_PAGE_UP;
        case Qt::Key_PageDown: return GHOSTTY_KEY_PAGE_DOWN;
        case Qt::Key_Up: return GHOSTTY_KEY_ARROW_UP;
        case Qt::Key_Down: return GHOSTTY_KEY_ARROW_DOWN;
        case Qt::Key_Left: return GHOSTTY_KEY_ARROW_LEFT;
        case Qt::Key_Right: return GHOSTTY_KEY_ARROW_RIGHT;
        default: return GHOSTTY_KEY_UNIDENTIFIED;
    }
}

template<>
GhosttyMouseButton uni_cast<GhosttyMouseButton, int>(const int &s, const int depth) {
    Q_UNUSED(depth);
    switch (s) {
        case Qt::LeftButton: return GHOSTTY_MOUSE_BUTTON_LEFT;
        case Qt::RightButton: return GHOSTTY_MOUSE_BUTTON_RIGHT;
        case Qt::MiddleButton: return GHOSTTY_MOUSE_BUTTON_MIDDLE;
        default: return GHOSTTY_MOUSE_BUTTON_UNKNOWN;
    }
}

template<>
GhosttyString uni_cast<GhosttyString, GhosttyStaticString>(const GhosttyStaticString &s, const int depth) {
    Q_UNUSED(depth);
    if (!s.value) return {};
    return {
        reinterpret_cast<const uint8_t *>(s.value),
        std::strlen(s.value)
    };
}

