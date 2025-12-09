#include "luaModule/luaIO.h"

#include <QVariant>
#include <sol/object.hpp>
#include <sol/variadic_args.hpp>

#include "utils/luaUtils.h"

LuaIO::LuaIO(QObject *parent)
    : QObject(parent) {
}

void LuaIO::log(const sol::variadic_args &args) {
    std::function<void(const QVariant&)> logging = [&](const QVariant& var) {
        if (var.typeId() == QMetaType::QVariantMap) {
            QVariantMap map = var.toMap();
            if (map.isEmpty()) {
                emit appendLog("{}", "info");
                return;
            }
            for (auto it = map.begin(); it != map.end(); ++it) {
                QString key = it.key();
                const QVariant& value = it.value();
                if (value.typeId() == QMetaType::QVariantMap) {
                    emit appendLog(QString("%1: {").arg(key), "info");
                    logging(value);
                    emit appendLog("}", "info");
                } else {
                    emit appendLog(QString("%1: %2").arg(key).arg(value.toString()), "info");
                }
            }
        } else {
            emit appendLog(var.toString(), "info");
        }
    };

    QVariantList parsedList = lua2qvarlist(args);
    for (const auto &parsed: parsedList) {
        logging(parsed);
    }
}
