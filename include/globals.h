#ifndef UNICOMM_GLOBALS_H
#define UNICOMM_GLOBALS_H

#include <QJsonObject>
#include <QMainWindow>
#include <QUrl>
#include <QVariantMap>

class DatabaseModule;
class DatatableModule;
class DataplotModule;
class DebugModule;
class Log;
class PortModule;
class ScriptModule;
class StructureModule;
class ThreadpoolModule;
class UndoModule;

extern QMainWindow *g_mainWindow;
extern DatabaseModule *g_database;
extern DatatableModule *g_datatable;
extern DataplotModule *g_dataplot;
extern DebugModule *g_debug;
extern Log *g_log;
extern PortModule *g_port;
extern ScriptModule *g_script;
extern StructureModule *g_structure;
extern ThreadpoolModule *g_threadpool;
extern UndoModule *g_undo;

extern QJsonObject g_config;

extern QVariantMap g_cursorPosition;

extern QHash<QUrl, QHash<int, QVariantMap> > g_breakpoints;

enum {
    SERIALPORT = 1,
    TCPCLIENT,
    TCPSERVER,
    UDPSOCKET,
    SCREEN,
    CAMERA
};

enum {
    THREAD_RUN,
    THREAD_DEBUG,
    THREAD_STOP
};

enum {
    DEBUG_RUN,
    DEBUG_PAUSE,
    DEBUG_STEPOVER,
    DEBUG_STEPINTO,
    DEBUG_STEPOUT,
    DEBUG_RUNTOCURSOR
};

enum {
    MARKER_BREAKPOINT,
    MARKER_ARROW,
    MARKER_ERROR,
    MARKER_HINT,
};

enum {
    INDICATOR_ERROR = 1,
    INDICATOR_WARNING,
    INDICATOR_INFO,
    INDICATOR_HINT,
    INDICATOR_HIGHLIGHT,
};

enum {
    RAW,
    GAUSSIANBLUR,
    THRESHOLD,
};

#endif //UNICOMM_GLOBALS_H