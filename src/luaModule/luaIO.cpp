#include "luaModule/luaIO.h"

#include <QEventLoop>
#include <QTextToSpeech>
#include <QThread>
#include <QVariant>
#include <sol/object.hpp>

#include "globals.h"
#include "utils/uniCast.h"

LuaIO::LuaIO(QObject *parent)
    : QObject(parent) {
}

void LuaIO::log(const sol::variadic_args &args) {
    std::function<void(const QVariant &)> logging = [&](const QVariant &var) {
        if (var.typeId() == QMetaType::QVariantMap) {
            QVariantMap map = var.toMap();
            if (map.isEmpty()) {
                emit appendLog("{}", LOG_INFO);
                return;
            }
            for (auto it = map.begin(); it != map.end(); ++it) {
                const QString &key = it.key();
                const QVariant &value = it.value();
                if (value.typeId() == QMetaType::QVariantMap) {
                    emit appendLog(QString("%1: {").arg(key), LOG_INFO);
                    logging(value);
                    emit appendLog("}", LOG_INFO);
                } else {
                    emit appendLog(QString("%1: %2").arg(key, value.toString()), LOG_INFO);
                }
            }
        } else {
            emit appendLog(var.toString(), LOG_INFO);
        }
    };

    for (const auto &parsed: uni_cast<QVariantList>(args)) {
        logging(parsed);
    }
}

void LuaIO::message(const std::string &text) const {
    const auto eventloop = std::make_unique<QEventLoop>();
    emit newMessageDialog(eventloop.get(), QString::fromStdString(text));
    eventloop->exec();
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
