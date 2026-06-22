#include "terminal/module/conptyWidget.h"

#include <QDir>
#include <QMetaObject>
#include <windows.h>

#ifndef PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE
#define PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE ProcThreadAttributeValue(22, FALSE, TRUE, FALSE)
#endif

ConptyWidget::ConptyWidget(QObject *parent) : QObject(parent) {}

ConptyWidget::~ConptyWidget() {
    stop();
}

bool ConptyWidget::start(const QString &program, const QString &arguments, const QString &workingDirectory, const int rows, const int cols) {
    if (program.isEmpty() || rows < 1 || cols < 1) return false;

    HANDLE inputRead{};
    HANDLE inputWrite{};
    HANDLE outputRead{};
    HANDLE outputWrite{};

    if (!CreatePipe(&inputRead, &inputWrite, nullptr, 0)) return false;
    if (!CreatePipe(&outputRead, &outputWrite, nullptr, 0)) {
        CloseHandle(inputRead);
        CloseHandle(inputWrite);
        return false;
    }

    HPCON pseudoConsole{};
    const HRESULT hr = CreatePseudoConsole(COORD{static_cast<SHORT>(cols), static_cast<SHORT>(rows)}, inputRead, outputWrite, 0, &pseudoConsole);
    CloseHandle(inputRead);
    CloseHandle(outputWrite);
    if (FAILED(hr)) {
        CloseHandle(inputWrite);
        CloseHandle(outputRead);
        return false;
    }

    SIZE_T attributeListSize{};
    InitializeProcThreadAttributeList(nullptr, 1, 0, &attributeListSize);
    auto *attributeList = static_cast<PPROC_THREAD_ATTRIBUTE_LIST>(HeapAlloc(GetProcessHeap(), 0, attributeListSize));
    if (!attributeList || !InitializeProcThreadAttributeList(attributeList, 1, 0, &attributeListSize)) {
        if (attributeList) HeapFree(GetProcessHeap(), 0, attributeList);
        ClosePseudoConsole(pseudoConsole);
        CloseHandle(inputWrite);
        CloseHandle(outputRead);
        return false;
    }

    if (!UpdateProcThreadAttribute(
        attributeList,
        0,
        PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
        pseudoConsole,
        sizeof(pseudoConsole),
        nullptr,
        nullptr
    )) {
        DeleteProcThreadAttributeList(attributeList);
        HeapFree(GetProcessHeap(), 0, attributeList);
        ClosePseudoConsole(pseudoConsole);
        CloseHandle(inputWrite);
        CloseHandle(outputRead);
        return false;
    }

    STARTUPINFOEXW startupInfo{};
    startupInfo.StartupInfo.cb = sizeof(startupInfo);
    startupInfo.lpAttributeList = attributeList;

    const QString commandLine = QString("\"%1\" %2").arg(program, arguments);
    const std::wstring applicationName = program.toStdWString();
    std::wstring command = commandLine.toStdWString();
    const std::wstring directory = QDir::toNativeSeparators(workingDirectory).toStdWString();

    PROCESS_INFORMATION processInfo{};
    const BOOL created = CreateProcessW(
        applicationName.c_str(),
        command.data(),
        nullptr,
        nullptr,
        FALSE,
        EXTENDED_STARTUPINFO_PRESENT,
        nullptr,
        directory.empty() ? nullptr : directory.c_str(),
        &startupInfo.StartupInfo,
        &processInfo
    );

    DeleteProcThreadAttributeList(attributeList);
    HeapFree(GetProcessHeap(), 0, attributeList);

    if (!created) {
        ClosePseudoConsole(pseudoConsole);
        CloseHandle(inputWrite);
        CloseHandle(outputRead);
        return false;
    }

    m_pseudoConsole = pseudoConsole;
    m_conptyInputWrite = inputWrite;
    m_conptyOutputRead = outputRead;
    m_processHandle = processInfo.hProcess;
    m_threadHandle = processInfo.hThread;

    m_readerThread = QThread::create([this, outputRead] {
        char buffer[4096]{};
        DWORD read{};
        while (ReadFile(outputRead, buffer, sizeof(buffer), &read, nullptr) && read > 0) {
            const QByteArray bytes(buffer, static_cast<qsizetype>(read));
            emit outputReady(bytes);
        }
        emit closed();
    });
    m_readerThread->start();
    return true;
}

void ConptyWidget::write(const QByteArray &bytes) const {
    if (!m_conptyInputWrite || bytes.isEmpty()) return;

    DWORD written{};
    WriteFile(
        m_conptyInputWrite,
        bytes.constData(),
        bytes.size(),
        &written,
        nullptr
    );
}

void ConptyWidget::resize(const int rows, const int cols) const {
    if (!m_pseudoConsole || rows < 1 || cols < 1) return;
    ResizePseudoConsole(m_pseudoConsole, COORD{static_cast<SHORT>(cols), static_cast<SHORT>(rows)});
}

void ConptyWidget::stop() {
    if (m_conptyInputWrite) write("exit\r\n");

    if (m_processHandle) WaitForSingleObject(m_processHandle, 1000);

    closeHandle(m_conptyInputWrite);

    if (m_pseudoConsole) {
        ClosePseudoConsole(m_pseudoConsole);
        m_pseudoConsole = nullptr;
    }

    if (m_readerThread) {
        m_readerThread->quit();
        m_readerThread->wait(1000);
        delete m_readerThread;
        m_readerThread = nullptr;
    }

    closeHandle(m_conptyOutputRead);
    closeHandle(m_threadHandle);
    closeHandle(m_processHandle);
}

bool ConptyWidget::running() const {
    if (!m_processHandle) return false;
    return WaitForSingleObject(m_processHandle, 0) == WAIT_TIMEOUT;
}

void ConptyWidget::closeHandle(void *&handle) {
    if (handle) {
        CloseHandle(handle);
        handle = nullptr;
    }
}
