#include "globals.h"

MainWindow *g_mainWindow = nullptr;
QNetworkAccessManager *g_networkAccessManager = nullptr;
GlobalManager *g_globalManager = nullptr;
Ripgrep *g_ripgrep = nullptr;
DatabaseModule *g_database = nullptr;
DatatableModule *g_datatable = nullptr;
DataplotModule *g_dataplot = nullptr;
DocumentModule *g_document = nullptr;
GitModule *g_git = nullptr;
LogModule *g_log = nullptr;
NuspellModule *g_nuspell = nullptr;
PortModule *g_port = nullptr;
ThreadpoolModule *g_thread = nullptr;
UndoModule *g_undo = nullptr;

bool g_terminating = false;
int g_theme = {};
QUrl g_workspaceUrl = {};
QString g_gitPath = {};
QJsonObject g_mainConfig = {};
QJsonObject g_workspaceConfig = {};

ViSession g_rm = VI_NULL;

QVariantMap g_cursorPosition = {
    {"url", QUrl()},
    {"line", -1},
    {"character", -1}
};

QHash<QChar, int> g_gitStatusCode = {
    {'?', GitStatusCode::Untracked},
    {'!', GitStatusCode::Ignored},
    {' ', GitStatusCode::Unmodified},
    {'M', GitStatusCode::Modified},
    {'T', GitStatusCode::FileTypeChanged},
    {'A', GitStatusCode::Added},
    {'D', GitStatusCode::Deleted},
    {'R', GitStatusCode::Renamed},
    {'C', GitStatusCode::Copied},
    {'U', GitStatusCode::UpdatedButUnmerged}
};

QHash<QUrl, QHash<int, QVariantHash> > g_breakpoints = {};
QStandardItemModel *g_portStandardItemModel = {};
QStandardItemModel *g_databaseStandardItemModel = {};
QStandardItemModel *g_datatableHeaderItemModel = {};
QStandardItemModel *g_datatableStandardItemModel = {};
QStandardItemModel *g_watchStandardItemModel = {};
