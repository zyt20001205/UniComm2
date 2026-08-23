#include "api/io.h"

#include <QTextToSpeech>
#include <QThread>
#include <QVariant>
#include <sol/object.hpp>

#include "globals.h"
#include "sol/variadic_args.hpp"
#include "util/uniCast.h"

IO::IO(QObject *parent)
    : QObject(parent) {
}

QByteArray IO::print(const sol::variadic_args &args) {
    QByteArray data{};
    std::function<void(const QVariant &, int)> printing = [&](const QVariant &value, const int depth) {
        if (value.typeId() == QMetaType::QVariantMap) {
            const auto map = value.toMap();
            data.append('{');
            for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
                data.append("\r\n");
                data.append(QByteArray(" ").repeated((depth + 1) * 4));
                data.append(it.key().toUtf8());
                data.append(": ");
                printing(it.value(), depth + 1);
            }
            if (!map.isEmpty()) {
                data.append("\r\n");
                data.append(QByteArray(" ").repeated(depth * 4));
            }
            data.append('}');
        } else data.append(value.toString().toUtf8());
    };

    bool first = true;
    for (const auto &arg: uni_cast<QVariantList>(args)) {
        if (!first) data.append('\t');
        printing(arg, 0);
        first = false;
    }
    data.append("\r\n");
    return data;
}

QByteArray IO::write(const sol::variadic_args &args) {
    QByteArray data{};
    lua_State *L = args.lua_state();
    for (const auto &arg: args) {
        if (arg.get_type() != sol::type::string && arg.get_type() != sol::type::number) throw sol::error("io.write expects string or number");

        size_t length{};
        const auto *string = luaL_tolstring(L, arg.stack_index(), &length);
        for (size_t i = 0; i < length; ++i) {
            if (string[i] == '\n' && (data.isEmpty() || data.back() != '\r')) data.append('\r');
            data.append(string[i]);
        }
        lua_pop(L, 1);
    }
    return data;
}

void IO::message(const std::string &text) const {
    const auto eventloop = std::make_unique<QEventLoop>();
    emit newMessageDialog(eventloop.get(), QString::fromStdString(text));
    eventloop->exec();
}

void IO::speak(const std::string &text) {
    QTextToSpeech tts;
    if (tts.engine().isEmpty()) {
        throw sol::error(tr("No TTS engine found").toStdString());
    }
    if (text.empty()) {
        return;
    }
    tts.setLocale(QLocale::English);
    // tts.setLocale(QLocale::Chinese);
    tts.setRate(0.0);
    tts.setVolume(1.0);
    QEventLoop loop;
    connect(&tts, &QTextToSpeech::stateChanged, [&](const QTextToSpeech::State state) { if (state == QTextToSpeech::Ready) loop.quit(); });
    tts.say(QString::fromStdString(text));
    loop.exec();
}
