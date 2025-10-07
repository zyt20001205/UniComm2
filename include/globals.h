#ifndef GLOBALS_H
#define GLOBALS_H

#include <QJsonObject>
#include <QMainWindow>
#include <QUrl>
#include <QVariantMap>

class Database;
class Datatable;
class Dataplot;
class Debug;
class Log;
class Port;
class Script;
class Threadpool;

extern QMainWindow *g_mainWindow;
extern Database *g_database;
extern Datatable *g_datatable;
extern Dataplot *g_dataplot;
extern Debug *g_debug;
extern Log *g_log;
extern Port *g_port;
extern Script *g_script;
extern Threadpool *g_threadpool;

extern QJsonObject g_config;

extern QVariantMap g_cursorPosition;

extern QHash<QUrl, QHash<int, QVariantMap> > g_breakpoints;

enum {
    DEBUG_RUN,
    DEBUG_PAUSE,
    DEBUG_STEPOVER,
    DEBUG_STEPINTO,
    DEBUG_STEPOUT,
    DEBUG_RUNTOCURSOR
};

#endif //GLOBALS_H