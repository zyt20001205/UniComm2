#include "api/io.h"

#include <fcntl.h>
#include <io.h>
#include <QTextToSpeech>
#include <QThread>
#include <QVariant>
#include <sol/object.hpp>
#include <windows.h>

#include "globals.h"
#include "util/uniCast.h"

IO::IO(const QString &threadId, QObject *parent)
    : QObject(parent),
      m_threadId(threadId) {
}

IO::~IO() {
    if (m_stdout) {
        fclose(static_cast<FILE *>(m_stdout));
        m_stdout = nullptr;
    }
    if (m_stderr) {
        fclose(static_cast<FILE *>(m_stderr));
        m_stderr = nullptr;
    }
    if (m_outputThread) {
        m_outputThread->wait();
        delete m_outputThread;
        m_outputThread = nullptr;
    }
    handleClose(m_outputRead);
}

void IO::redirect(lua_State *L) {
    HANDLE outputRead{};
    HANDLE stdoutWrite{};
    HANDLE stderrWrite{};
    if (!CreatePipe(&outputRead, &stdoutWrite, nullptr, 0)) throw sol::error("create stdout pipe failed");
    if (!DuplicateHandle(GetCurrentProcess(), stdoutWrite, GetCurrentProcess(), &stderrWrite, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
        CloseHandle(outputRead);
        CloseHandle(stdoutWrite);
        throw sol::error("create stderr pipe failed");
    }

    void *stdoutHandle = stdoutWrite;
    void *stderrHandle = stderrWrite;
    auto *stdoutFile = fileOpen(stdoutHandle);
    auto *stderrFile = fileOpen(stderrHandle);
    handleClose(stdoutHandle);
    handleClose(stderrHandle);
    if (!stdoutFile || !stderrFile) {
        if (stdoutFile) fclose(stdoutFile);
        if (stderrFile) fclose(stderrFile);
        CloseHandle(outputRead);
        throw sol::error("open standard output failed");
    }

    lua_getglobal(L, "io");
    lua_getfield(L, -1, "stdout");
    auto *stdoutStream = static_cast<luaL_Stream *>(luaL_testudata(L, -1, LUA_FILEHANDLE));
    lua_pop(L, 1);
    lua_getfield(L, -1, "stderr");
    auto *stderrStream = static_cast<luaL_Stream *>(luaL_testudata(L, -1, LUA_FILEHANDLE));
    lua_pop(L, 2);
    if (!stdoutStream || !stderrStream) {
        fclose(stdoutFile);
        fclose(stderrFile);
        CloseHandle(outputRead);
        throw sol::error("standard output is unavailable");
    }

    stdoutStream->f = stdoutFile;
    stderrStream->f = stderrFile;
    m_outputRead = outputRead;
    m_stdout = stdoutFile;
    m_stderr = stderrFile;
    m_outputThread = QThread::create([this] {
        char buffer[4096]{};
        DWORD read{};
        while (ReadFile(m_outputRead, buffer, sizeof(buffer), &read, nullptr) && read > 0) {
            emit writeTerminal(m_threadId, QByteArray(buffer, static_cast<qsizetype>(read)));
        }
    });
    m_outputThread->start();
}

void IO::print(const sol::variadic_args &args) const {
    QByteArray data{};
    std::function<void(const QVariant &, int)> printing = [&](const QVariant &value, const int depth) {
        if (value.typeId() == QMetaType::QVariantMap) {
            const auto map = value.toMap();
            data.append('{');
            for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
                data.append('\n');
                data.append(QByteArray(" ").repeated((depth + 1) * 4));
                data.append(it.key().toUtf8());
                data.append(": ");
                printing(it.value(), depth + 1);
            }
            if (!map.isEmpty()) {
                data.append('\n');
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
    data.append('\n');
    auto *file = static_cast<FILE *>(m_stdout);
    if (!file || fwrite(data.constData(), 1, static_cast<size_t>(data.size()), file) != static_cast<size_t>(data.size()))
        throw sol::error("stdout write failed");
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

// private
std::FILE *IO::fileOpen(void *&handle) {
    const auto descriptor = _open_osfhandle(reinterpret_cast<std::intptr_t>(handle), _O_WRONLY | _O_TEXT);
    if (descriptor == -1) return nullptr;
    handle = nullptr;
    auto *file = _fdopen(descriptor, "w");
    if (!file) {
        _close(descriptor);
        return nullptr;
    }
    setvbuf(file, nullptr, _IONBF, 0);
    return file;
}

void IO::handleClose(void *&handle) {
    if (!handle) return;
    CloseHandle(handle);
    handle = nullptr;
}
