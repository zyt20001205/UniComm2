#include "globals.h"

MainWindow *g_mainWindow = nullptr;
DatabaseModule *g_database = nullptr;
DatatableModule *g_datatable = nullptr;
DataplotModule *g_dataplot = nullptr;
DebugModule *g_debug = nullptr;
LogModule *g_log = nullptr;
PortModule *g_port = nullptr;
ScriptModule *g_script = nullptr;
StructureModule *g_structure = nullptr;
ThreadpoolModule *g_threadpool = nullptr;
UndoModule *g_undo = nullptr;

QJsonObject g_config = {};

QVariantMap g_cursorPosition = {
    {"url", QUrl()},
    {"line", -1},
    {"character", -1}
};

QHash<QUrl, QHash<int, QVariantMap> > g_breakpoints = {};