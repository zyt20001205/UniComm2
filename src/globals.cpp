#include "globals.h"

MainWindow *g_mainWindow = nullptr;
DatabaseModule *g_database = nullptr;
DatatableModule *g_datatable = nullptr;
DataplotModule *g_dataplot = nullptr;
NuspellModule *g_nuspell = nullptr;
PortModule *g_port = nullptr;
DocumentModule *g_document = nullptr;
UndoModule *g_undo = nullptr;

QUrl g_workspaceUrl = {};
QJsonObject g_workspaceConfig = {};

QHash<QString, QString> g_color = {
    {"back", "#ffffff"},
    {"hoverBack", "#f5f5f5"},
    {"pressedBack", "#e0e0e0"},
    {"selectedBack", "#ebebeb"},
    {"brandFore", "#115ea3"},
    {"successBack", "#107c10"},
    {"warningBack", "#c50f1f"},
    {"dangerBack", "#f7630c"},
};

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
