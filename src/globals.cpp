#include "globals.h"

MainWindow *g_mainWindow = nullptr;
QQuickItem *g_rootObject = nullptr;
DatabaseModule *g_database = nullptr;
DatatableModule *g_datatable = nullptr;
DataplotModule *g_dataplot = nullptr;
NuspellModule *g_nuspell = nullptr;
PortModule *g_port = nullptr;
ScriptModule *g_script = nullptr;
StructureModule *g_structure = nullptr;
UndoModule *g_undo = nullptr;

QUrl g_workspaceUrl = {};
QJsonObject g_workspaceConfig = {};

ViSession g_rm = VI_NULL;

QVariantMap g_cursorPosition = {
    {"url", QUrl()},
    {"line", -1},
    {"character", -1}
};

QHash<QUrl, QHash<int, QVariantHash> > g_breakpoints = {};