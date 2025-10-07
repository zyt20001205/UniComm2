#include "../include/globals.h"

QMainWindow *g_mainWindow = nullptr;
Database *g_database = nullptr;
Datatable *g_datatable = nullptr;
Dataplot *g_dataplot = nullptr;
Debug *g_debug = nullptr;
Log *g_log = nullptr;
Port *g_port = nullptr;
Script *g_script = nullptr;
Threadpool *g_threadpool = nullptr;

QJsonObject g_config = {};

QHash<QUrl, QHash<int, QVariantMap> > g_breakpoints = {};
