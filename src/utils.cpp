#include "../include/utils.h"

QString lua_toqstring(lua_State *L, const int idx) {
    switch (lua_type(L, idx)) {
        case LUA_TNIL:
            return QString("\\");
        case LUA_TBOOLEAN:
            return lua_toboolean(L, idx) ? "true" : "false";
        case LUA_TLIGHTUSERDATA:
            return QString("WIP");
        case LUA_TNUMBER:
            return lua_tostring(L, idx);
        case LUA_TSTRING:
            return lua_tostring(L, idx);
        case LUA_TTABLE:
            return QString("{...}");
        case LUA_TFUNCTION:
            return QString("\\");
        case LUA_TUSERDATA:
            return QString("WIP");
        case LUA_TTHREAD:
            return QString("\\");
        default:
            return QString("?");
    }
}

void lua_pushqstring(lua_State *L, const int idx, const QString &value) {
    switch (lua_type(L, idx)) {
        case LUA_TBOOLEAN: {
            if (value == "true" || value == "1") {
                lua_pushboolean(L, 1);
            } else {
                lua_pushboolean(L, 0);
            }
        }
        break;
        case LUA_TNUMBER: {
            if (value.contains(".") || value.contains("e")) {
                lua_pushnumber(L, value.toDouble());
            } else {
                lua_pushinteger(L, value.toInt());
            }
        }
        break;
        case LUA_TSTRING: {
            lua_pushstring(L, value.toUtf8().constData());
        }
        break;
        default: break;
    }
}

QString ocr(const QPixmap &pixmap, const QString &charset) {
    QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
    const char *tessCharset;
    if (charset.isEmpty()) {
        tessCharset = "eng";
    } else {
        tessCharset = charset.toUtf8().data();
    }
    auto *ocr = new tesseract::TessBaseAPI();
    ocr->Init(nullptr, tessCharset);
    ocr->SetImage(image.bits(), image.width(), image.height(), 3, image.bytesPerLine());
    char *result = ocr->GetUTF8Text();
    QString text = QString::fromUtf8(result);
    delete result;
    ocr->End();
    delete ocr;
    text = text.trimmed();
    return text.isEmpty() ? "null" : text;
}
