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
#include "datatable.h"
#include "log.h"
#include "manual.h"
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

    void saveConfig() const;

    Config *m_configModule = nullptr;
    Manual *m_manualModule = nullptr;
    Script *m_scriptModule = nullptr;
    Port *m_portModule = nullptr;
    Send *m_sendModule = nullptr;
    Database *m_databaseModule = nullptr;
    Datatable *m_datatableModule = nullptr;
    Log *m_logModule = nullptr;
};

#endif //MAINWINDOW_H
