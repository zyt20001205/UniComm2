#include "luaModule/luaMiscellaneous.h"

#include "globals.h"
#include "log.h"
#include "scriptModule/threadpoolModule.h"

int lua_exec(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 1)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const char *param1 = luaL_checkstring(L, 1);
    // start operation
    const QString scriptPath = QString::fromUtf8(param1);
    QString threadId = "null";
    QMetaObject::invokeMethod(g_mainWindow, [scriptPath, &threadId] {
        threadId = g_threadpool->threadExec(scriptPath);
    }, Qt::BlockingQueuedConnection);
    lua_pushstring(L, threadId.toUtf8().constData());
    return 1;
}

int lua_stop(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 1)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const char *param1 = luaL_checkstring(L, 1);
    // start operation
    const QString threadId = QString::fromUtf8(param1);
    bool status = false;
    QMetaObject::invokeMethod(g_mainWindow, [threadId, &status] {
        status = g_threadpool->threadStop(threadId);
    }, Qt::BlockingQueuedConnection);
    lua_pushboolean(L, status);
    return 1;
}

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
        status = g_threadpool->threadWait(threadId);
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

int lua_print(lua_State *L) {
    const int n = lua_gettop(L);
    QString message;
    for (int i = 1; i <= n; i++) {
        size_t len = 0;
        const char *s = luaL_tolstring(L, i, &len);
        if (i > 1) message += " ";
        if (s) message += QString::fromUtf8(s, static_cast<int>(len));
        lua_pop(L, 1);
    }
    if (!message.isEmpty()) {
        QMetaObject::invokeMethod(g_mainWindow, [message] {
            g_log->logAppend(message, "info");
        }, Qt::QueuedConnection);
    }
    return 0;
}

int lua_sleep(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 1)
        luaL_error(L, "unexpected number of arguments");
    // extract arguments
    const int param = static_cast<int>(luaL_checkinteger(L, 1));
    // start operation
    const int millisecond = param;
    QThread::msleep(millisecond);
    return 0;
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