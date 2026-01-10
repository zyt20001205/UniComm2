#include "luaModule/luaIO.h"

#include <QEventLoop>
#include <QTextToSpeech>
#include <QThread>
#include <QVariant>
#include <sol/object.hpp>
#include <sol/variadic_args.hpp>

#include "utils/luaUtils.h"

LuaIO::LuaIO(QObject *parent)
    : QObject(parent) {
}

void LuaIO::log(const sol::variadic_args &args) {
    std::function<void(const QVariant &)> logging = [&](const QVariant &var) {
        if (var.typeId() == QMetaType::QVariantMap) {
            QVariantMap map = var.toMap();
            if (map.isEmpty()) {
                emit appendLog("{}", "info");
                return;
            }
            for (auto it = map.begin(); it != map.end(); ++it) {
                const QString &key = it.key();
                const QVariant &value = it.value();
                if (value.typeId() == QMetaType::QVariantMap) {
                    emit appendLog(QString("%1: {").arg(key), "info");
                    logging(value);
                    emit appendLog("}", "info");
                } else {
                    emit appendLog(QString("%1: %2").arg(key, value.toString()), "info");
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

void LuaIO::message(const std::string &text) const {
    auto *eventloop = new QEventLoop();
    emit newMessageDialog(eventloop, QString::fromStdString(text));
    eventloop->exec();
    delete eventloop;
}

void LuaIO::speak(const std::string &text) {
    QTextToSpeech tts;
    if (tts.engine().isEmpty()) {
        throw sol::error(tr("No TTS engine found").toStdString());
    }
    if (text.empty()) {
        return;
    }
    // tts.setLocale(QLocale::English);
    tts.setLocale(QLocale::Chinese);
    tts.setRate(0.0);
    tts.setVolume(1.0);
    QEventLoop loop;
    connect(&tts, &QTextToSpeech::stateChanged, [&](const QTextToSpeech::State state) { if (state == QTextToSpeech::Ready) loop.quit(); });
    tts.say(QString::fromStdString(text));
    loop.exec();
}
