#include "api/io.h"

#include <QTextToSpeech>
#include <QThread>
#include <QVariant>
#include <sol/object.hpp>

#include "globals.h"
#include "util/uniCast.h"

IO::IO(QObject *parent)
    : QObject(parent) {
}

void IO::log(const sol::variadic_args &args) {
    std::function<void(const QString &, const QVariant &, int)> logging = [&](const QString &key, const QVariant &value, const int depth) {
        QString indent{};
        if (depth > 0) indent = QString("&nbsp;").repeated(depth * 4);

        if (value.typeId() == QMetaType::QVariantMap) {
            if (key.isEmpty()) emit appendLog(QString("%1{").arg(indent), LOG_INFO);
            else emit appendLog(QString("%1%2: {").arg(indent, key), LOG_INFO);
            QVariantMap map = value.toMap();
            for (auto it = map.begin(); it != map.end(); ++it) {
                logging(it.key(), it.value(), depth + 1);
            }
            emit appendLog(QString("%1}").arg(indent), LOG_INFO);
        } else {
            if (key.isEmpty()) emit appendLog(QString("%1%2").arg(indent, value.toString()), LOG_INFO);
            else emit appendLog(QString("%1%2: %3").arg(indent, key, value.toString()), LOG_INFO);
        }
    };

    for (const auto &arg: uni_cast<QVariantList>(args)) {
        logging("", arg, 0);
    }
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
    // tts.setLocale(QLocale::English);
    tts.setLocale(QLocale::Chinese);
    tts.setRate(0.0);
    tts.setVolume(1.0);
    QEventLoop loop;
    connect(&tts, &QTextToSpeech::stateChanged, [&](const QTextToSpeech::State state) { if (state == QTextToSpeech::Ready) loop.quit(); });
    tts.say(QString::fromStdString(text));
    loop.exec();
}
