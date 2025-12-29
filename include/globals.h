#ifndef UNICOMM_GLOBALS_H
#define UNICOMM_GLOBALS_H

#include <visa.h>

#include "mainWindow.h"

class QStandardItemModel;
class QStringListModel;

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
extern NuspellModule *g_nuspell;
extern PortModule *g_port;
extern ScriptModule *g_script;
extern StructureModule *g_structure;
extern UndoModule *g_undo;

extern QUrl g_workspaceUrl;
extern QJsonObject g_workspaceConfig;

extern ViSession g_rm;

extern QVariantMap g_cursorPosition;

extern QHash<QUrl, QHash<int, QVariantHash> > g_breakpoints;
extern QStandardItemModel* g_portStandardItemModel;
extern QStandardItemModel* g_databaseStandardItemModel;
extern QStringListModel* g_datatableStringListModel;

enum {
    SERIALPORT,
    VISA,
    TCPCLIENT,
    TCPSERVER,
    UDPSOCKET,
    VIDEOSTREAM
};

enum {
    LUATHREAD_RUN,
    LUATHREAD_DEBUG
};

enum {
    DEBUG_TERMINATE,
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
    INDICATOR_TYPO,
    INDICATOR_HIGHLIGHT,
    INDICATOR_READ,
    INDICATOR_WRITE,
    INDICATOR_SEARCH,
    INDICATOR_SELECTION,
    INDICATOR_HYPERLINK
};

enum {
    MARKER_BREAKPOINT,
    MARKER_DEBUG,
    MARKER_ERROR,
    MARKER_HINT,
    MARKER_HEATMAP0,
    MARKER_HEATMAP25,
    MARKER_HEATMAP50,
    MARKER_HEATMAP75,
    MARKER_HEATMAP100
};

enum {
    LUA_TOKEN_NAMESPACE = 64,
    LUA_TOKEN_CLASS,
    LUA_TOKEN_TYPE,
    LUA_TOKEN_PARAMETER,
    LUA_TOKEN_VARIABLE,
    LUA_TOKEN_PROPERTY,
    LUA_TOKEN_ENUMMEMBAER,
    LUA_TOKEN_FUNCTION_DECLARATION,
    LUA_TOKEN_FUNCTION_CALL,
    LUA_TOKEN_METHOD,
    LUA_TOKEN_MACRO,
    LUA_TOKEN_KEYWORD,
    LUA_TOKEN_COMMENT,
    LUA_TOKEN_STRING,
    LUA_TOKEN_NUMBER,
    LUA_TOKEN_OPERATOR,
};

enum {
    COMPLETION_KIND_TEXT = 1,
    COMPLETION_KIND_METHOD,
    COMPLETION_KIND_FUNCTION,
    COMPLETION_KIND_CONSTRUCTOR,
    COMPLETION_KIND_FIELD,
    COMPLETION_KIND_VARIABLE,
    COMPLETION_KIND_CLASS,
    COMPLETION_KIND_INTERFACE,
    COMPLETION_KIND_MODULE,
    COMPLETION_KIND_PROPERTY,
    COMPLETION_KIND_UNIT,
    COMPLETION_KIND_VALUE,
    COMPLETION_KIND_ENUM,
    COMPLETION_KIND_KEYWORD,
    COMPLETION_KIND_SNIPPET,
    COMPLETION_KIND_COLOR,
    COMPLETION_KIND_FILE,
    COMPLETION_KIND_REFERENCE,
    COMPLETION_KIND_FOLDER,
    COMPLETION_KIND_ENUMMEMBER,
    COMPLETION_KIND_CONSTANT,
    COMPLETION_KIND_STRUCT,
    COMPLETION_KIND_EVENT,
    COMPLETION_KIND_OPERATOR,
    COMPLETION_KIND_TYPEPARAMETER,
};

enum {
    COMPLETION_MODE_FUNCTION,
    COMPLETION_MODE_ARGUMENT,
};

enum {
    SCALE,
    GAUSSIANBLUR,
    THRESHOLD,
};

#endif //UNICOMM_GLOBALS_H