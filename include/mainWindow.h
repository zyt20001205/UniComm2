#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDateTime>
#include <QDockWidget>
#include <QKeySequence>
#include <QMainWindow>
#include <QMenuBar>
#include <QShortcut>
#include <QWidget>
#include "config.h"
#include "database.h"
#include "dataplot.h"
#include "datatable.h"
#include "log.h"
#include "luaLanguageServer.h"
#include "port.h"
#include "script.h"
#include "send.h"

inline Database *g_database = nullptr;
inline Datatable *g_datatable = nullptr;
inline Log *g_log = nullptr;
inline Port *g_port = nullptr;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override = default;

signals:
    void appendLog(const QString &message, const QString &level);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void configInit();

    void menuInit();

    void moduleInit();

    void shortcutInit();

    void layoutInit();

    void layoutSave();

    void saveConfig();

    QJsonObject m_mainConfig{};

    Config *m_configModule = nullptr;
    LuaLanguageServer *m_llsModule = nullptr;
    Script *m_scriptModule = nullptr;
    Port *m_portModule = nullptr;
    Send *m_sendModule = nullptr;
    Database *m_databaseModule = nullptr;
    Datatable *m_datatableModule = nullptr;
    Dataplot *m_dataplotModule = nullptr;
    Log *m_logModule = nullptr;

    QAction *m_viewPort = nullptr;
    QAction *m_viewSend = nullptr;
    QAction *m_viewDatabase = nullptr;
    QAction *m_viewDatatable = nullptr;
    QAction *m_viewDataplot = nullptr;
    QAction *m_viewLog = nullptr;
};

#endif //MAINWINDOW_H
