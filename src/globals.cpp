#include "globals.h"

MainWindow *g_mainWindow = nullptr;
GlobalManager *g_global = nullptr;
DatabaseModule *g_database = nullptr;
DatatableModule *g_datatable = nullptr;
DataplotModule *g_dataplot = nullptr;
LogModule *g_log = nullptr;
NuspellModule *g_nuspell = nullptr;
PortModule *g_port = nullptr;
DocumentModule *g_document = nullptr;
UndoModule *g_undo = nullptr;

bool g_terminating = false;
int g_theme = {};
QUrl g_workspaceUrl = {};
bool g_gitEnabled = false;
QJsonObject g_workspaceConfig = {};

ViSession g_rm = VI_NULL;

QVariantMap g_cursorPosition = {
    {"url", QUrl()},
    {"line", -1},
    {"character", -1}
};

QHash<QUrl, QHash<int, QVariantHash> > g_breakpoints = {};
QStandardItemModel *g_portStandardItemModel = {};
QStandardItemModel *g_databaseStandardItemModel = {};
QStandardItemModel *g_datatableHeaderItemModel = {};
QStandardItemModel *g_datatableStandardItemModel = {};
QStandardItemModel *g_watchStandardItemModel = {};
