#include "util/uniCast.h"

#include <QDir>
#include <sol/state_view.hpp>
#include <sol/table_core.hpp>
#include <sol/variadic_args.hpp>
#include <sol/userdata.hpp>

#include "cmark.h"
#include "globals.h"

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
    if (suffix == "pdf") return QUrl("qrc:/icon/fileTypePdf.svg");
    if (suffix == "ps1") return QUrl("qrc:/icon/fileTypePowershell.svg");
    if (suffix == "txt") return QUrl("qrc:/icon/fileTypeTxt.svg");
    if (fileInfo.isDir() && fileInfo.fileName() == ".git") return QUrl("qrc:/icon/fileTypeFolderGit.svg");
    if (fileInfo.isDir() && fileInfo.fileName() == ".idea") return QUrl("qrc:/icon/fileTypeFolderIntellij.svg");
    if (fileInfo.isDir()) return QUrl("qrc:/icon/fileTypeFolder.svg");
    return QUrl("qrc:/icon/fileTypeDefault.svg");
}

template<>
QHtmlString uni_cast<QHtmlString, QString>(const QString &s, const int depth) {
    Q_UNUSED(depth);
    const auto md = s.toUtf8();
    char *htmlChar = cmark_markdown_to_html(md.constData(), md.size(), CMARK_OPT_DEFAULT);
    if (!htmlChar) return QString();
    const QString d = QString::fromUtf8(htmlChar);
    free(htmlChar);
    return d;
}

// vterm -> qt
template<>
TerminalCell uni_cast<TerminalCell, VTermScreenCell>(const VTermScreen *vts, const VTermScreenCell &s, const int depth) {
    Q_UNUSED(depth);
    TerminalCell d{};
    d.width = static_cast<int>(s.width);

    if (s.chars[0] == UINT32_MAX) {
        d.text = QString{};
        d.width = 0;
    } else if (s.chars[0] == 0 || s.chars[0] == ' ') {
        d.text = ' ';
    } else {
        int length = 0;
        while (length < VTERM_MAX_CHARS_PER_CELL && s.chars[length] != 0 && s.chars[length] != UINT32_MAX) ++length;
        d.text = QString::fromUcs4(s.chars, length);
    }

    VTermColor foreground = s.fg;
    vterm_screen_convert_color_to_rgb(vts, &foreground);
    d.foreground = QColor(foreground.rgb.red, foreground.rgb.green, foreground.rgb.blue);

    VTermColor background = s.bg;
    vterm_screen_convert_color_to_rgb(vts, &background);
    d.background = QColor(background.rgb.red, background.rgb.green, background.rgb.blue);

    return d;
}

// qt-> vterm
template<>
VTermButton uni_cast<VTermButton, int>(const int &s, const int depth) {
    Q_UNUSED(depth);
    switch (s) {
        case Qt::LeftButton:
            return 1;
        case Qt::MiddleButton:
            return 2;
        case Qt::RightButton:
            return 3;
        default:
            return 0;
    }
}

template<>
VTermKey uni_cast<VTermKey, int>(const int &s, const int depth) {
    Q_UNUSED(depth);
    int d = VTERM_KEY_NONE;
    switch (s) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            d = VTERM_KEY_ENTER;
            break;
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            d = VTERM_KEY_TAB;
            break;
        case Qt::Key_Backspace:
            d = VTERM_KEY_BACKSPACE;
            break;
        case Qt::Key_Escape:
            d = VTERM_KEY_ESCAPE;
            break;
        case Qt::Key_Up:
            d = VTERM_KEY_UP;
            break;
        case Qt::Key_Down:
            d = VTERM_KEY_DOWN;
            break;
        case Qt::Key_Left:
            d = VTERM_KEY_LEFT;
            break;
        case Qt::Key_Right:
            d = VTERM_KEY_RIGHT;
            break;
        case Qt::Key_Insert:
            d = VTERM_KEY_INS;
            break;
        case Qt::Key_Delete:
            d = VTERM_KEY_DEL;
            break;
        case Qt::Key_Home:
            d = VTERM_KEY_HOME;
            break;
        case Qt::Key_End:
            d = VTERM_KEY_END;
            break;
        case Qt::Key_PageUp:
            d = VTERM_KEY_PAGEUP;
            break;
        case Qt::Key_PageDown:
            d = VTERM_KEY_PAGEDOWN;
            break;
        default:
            if (s >= Qt::Key_F1 && s <= Qt::Key_F35) {
                d = VTERM_KEY_FUNCTION(s - Qt::Key_F1 + 1);
            }
            break;
    }
    return static_cast<VTermKey>(d);
}

template<>
VTermModifier uni_cast<VTermModifier, int>(const int &s, const int depth) {
    Q_UNUSED(depth);
    int d = VTERM_MOD_NONE;
    if (s & Qt::ShiftModifier) d |= VTERM_MOD_SHIFT;
    if (s & Qt::AltModifier) d |= VTERM_MOD_ALT;
    if (s & Qt::ControlModifier) d |= VTERM_MOD_CTRL;
    return static_cast<VTermModifier>(d);
}
