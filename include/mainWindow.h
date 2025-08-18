#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDateTime>
#include <QDockWidget>
#include <QKeySequence>
#include <QMainWindow>
#include <QShortcut>
#include <QWidget>
#include "config.h"
#include "mainWindow.h"
#include "explorer.h"
#include "log.h"
#include "port.h"
#include "script.h"
#include "send.h"

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override = default;

private:
    void init();

    void configInit();

    void moduleInit();

    void shortcutInit();

    Config *m_configModule = nullptr;
    Script *m_scriptModule = nullptr;
    Port *m_portModule = nullptr;
    Send *m_sendModule = nullptr;
    Explorer *m_explorerModule = nullptr;
    Log *m_logModule = nullptr;

signals:
    void appendLog(const QString &message, const QString &level);
};

#endif //MAINWINDOW_H
