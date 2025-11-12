#ifndef UNICOMM_GLOBALS_H
#define UNICOMM_GLOBALS_H

#include <QJsonObject>
#include <QUrl>
#include <QVariantMap>
#include <visa.h>

#include "mainWindow.h"

class DatabaseModule;
class DatatableModule;
class DataplotModule;
class DebugModule;
class LogModule;
class PortModule;
class ScriptModule;
class StructureModule;
class ThreadpoolModule;
class UndoModule;

extern MainWindow *g_mainWindow;
extern DatabaseModule *g_database;
extern DatatableModule *g_datatable;
extern DataplotModule *g_dataplot;
extern DebugModule *g_debug;
extern LogModule *g_log;
extern PortModule *g_port;
extern ScriptModule *g_script;
extern StructureModule *g_structure;
extern ThreadpoolModule *g_threadpool;
extern UndoModule *g_undo;

extern QUrl g_workspaceUrl;
extern QJsonObject g_workspaceConfig;

extern ViSession g_rm;

extern QVariantMap g_cursorPosition;

extern QHash<QUrl, QHash<int, QVariantHash> > g_breakpoints;

enum {
    SERIALPORT = 1,
    VISA,
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
    DEBUG_RESUME,
    DEBUG_PAUSE,
    DEBUG_STEPOVER,
    DEBUG_STEPINTO,
    DEBUG_STEPOUT,
    DEBUG_RUNTOCURSOR
};

enum {
    INDICATOR_ERROR = 1,
    INDICATOR_WARNING,
    INDICATOR_INFO,
    INDICATOR_HINT,
    INDICATOR_HIGHLIGHT,
    INDICATOR_SEARCH_RESULT,
    INDICATOR_SEARCH_CURRENT,
    INDICATOR_HYPERLINK_FONT,
    INDICATOR_HYPERLINK_UNDERLINE
};

enum {
    MARKER_BREAKPOINT,
    MARKER_ARROW,
    MARKER_ERROR,
    MARKER_HINT,
    MARKER_HEATMAP0,
    MARKER_HEATMAP25,
    MARKER_HEATMAP50,
    MARKER_HEATMAP75,
    MARKER_HEATMAP100
};

enum {
    LUATOKEN_CLASS = 64,
    LUATOKEN_TYPE,
    LUATOKEN_PARAMETER,
    LUATOKEN_VARIABLE,
    LUATOKEN_PROPERTY,
    LUATOKEN_FUNCTION_DECLARATION,
    LUATOKEN_FUNCTION_CALL,
    LUATOKEN_METHOD,
    LUATOKEN_MACRO,
    LUATOKEN_KEYWORD,
    LUATOKEN_COMMENT,
    LUATOKEN_STRING,
    LUATOKEN_NUMBER,
    LUATOKEN_OPERATOR,
};

enum {
    RAW,
    GAUSSIANBLUR,
    THRESHOLD,
};

#endif //UNICOMM_GLOBALS_H