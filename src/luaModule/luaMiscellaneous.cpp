#include "luaModule/luaMiscellaneous.h"

#include <QInputDialog>

#include "globals.h"
#include "logModule.h"
#include "scriptModule/codeDebug/threadpoolModule.h"

int lua_wait(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 1)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const char *param1 = luaL_checkstring(L, 1);
    // start operation
    const QString threadId = QString::fromUtf8(param1);
    bool status = false;
    QMetaObject::invokeMethod(g_mainWindow, [threadId, &status] {
        status = g_threadpool->threadJoin(threadId);
    }, Qt::BlockingQueuedConnection);
    lua_pushboolean(L, status);
    return 1;
}

int lua_input(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 0)
        luaL_error(L, "unexpected number of arguments");
    // start operation
    bool ok = false;
    QString input;
    QMetaObject::invokeMethod(qApp, [&ok, &input] {
        QWidget *parent = QApplication::activeWindow();
        input = QInputDialog::getText(parent, "Input Dialog", "input:", QLineEdit::Normal, QString(), &ok);
    }, Qt::BlockingQueuedConnection);
    if (!ok)
        return 0;
    lua_pushstring(L, input.toUtf8().constData());
    return 1;
}

int lua_speak(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 1)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const char *param1 = luaL_checkstring(L, 1);
    // start operation
    const QString text = QString::fromUtf8(param1);
    QTextToSpeech tts;
    if (text == "help") {
        if (tts.engine().isEmpty()) {
            QMetaObject::invokeMethod(g_mainWindow, [] {
                g_log->logAppend("no tts engine found", "info");
            }, Qt::QueuedConnection);
            return 0;
        }
        for (const QVoice &voice: tts.availableVoices()) {
            QString voiceInfo = "[" + voice.locale().name() + "] " + voice.name();
            QMetaObject::invokeMethod(g_mainWindow, [voiceInfo] {
                g_log->logAppend(voiceInfo, "info");
            }, Qt::QueuedConnection);
        }
        return 0;
    }
    if (tts.engine().isEmpty()) {
        luaL_error(L, "no tts engine found");
    }
    // tts.setLocale(QLocale::Chinese);
    tts.setLocale(QLocale::English);
    tts.setRate(0.0);
    tts.setVolume(1.0);
    QEventLoop loop;
    QObject::connect(&tts, &QTextToSpeech::stateChanged, [&](const QTextToSpeech::State state) {
        if (state == QTextToSpeech::Ready) loop.quit();
    });
    tts.say(text);
    loop.exec();
    return 0;
}
