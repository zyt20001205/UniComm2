#include "mainWindow.h"

#include <QCameraDevice>
#include <QCloseEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMediaDevices>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QStandardPaths>
#include <QThread>
#include "kddockwidgets/qtwidgets/views/MainWindow.h"
#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include "kddockwidgets/LayoutSaver.h"

#include "configModule.h"
#include "globals.h"
#include "logModule.h"
#include "undoModule.h"
#include "utils.h"
#include "dataModule/databaseModule.h"
#include "dataModule/dataplotModule.h"
#include "dataModule/datatableModule.h"
#include "luaModule/luaLanguageServer.h"
#include "portModule/portModule.h"
#include "portModule/sendModule.h"
#include "scriptModule/debugModule.h"
#include "scriptModule/diagnosticsModule.h"
#include "scriptModule/explorerModule.h"
#include "scriptModule/scriptModule.h"
#include "scriptModule/scriptPage.h"
#include "scriptModule/structureModule.h"
#include "scriptModule/threadpoolModule.h"

// MainWindow public
MainWindow::MainWindow(QWidget *parent, const QString &uniqueName)
    : KDDockWidgets::QtWidgets::MainWindow(uniqueName, KDDockWidgets::MainWindowOption_None, parent) {
    // mainWindow ui init
    g_mainWindow = this;
    QWidget::setWindowTitle("UniComm");
    QWidget::setWindowIcon(QIcon(":/icon/icon.ico"));
    QWidget::resize(1600, 900);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "main window created");

    configInit();
    moduleInit();
    workspaceInit();
    shortcutInit();
    menuInit();
    layoutInit();

    // preload multimedia to avoid lagging on first click
    QThread *worker = QThread::create([] {
        QMediaDevices::videoInputs();
    });
    worker->start();
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
}

void MainWindow::workspaceOpen() {
    QUrl rootUrl{};
    // select new root directory
    const QString rootDir = QFileDialog::getExistingDirectory(
        this,
        tr("Open Workspace"),
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    if (rootDir.isEmpty()) return;
    rootUrl = QUrl::fromLocalFile(rootDir);
    m_mainConfig["workspace"] = rootUrl.toString();
    // check if lua config files exist
    const QString rootPath = rootUrl.toLocalFile();
    if (const QString luarcPath = QDir(rootPath).filePath(".luarc.json"); !QFile::exists(luarcPath)) {
        QFile::copy(":/config/.luarc.json", luarcPath);
        QFile::setPermissions(luarcPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                         | QFileDevice::ReadUser | QFileDevice::WriteUser
                                         | QFileDevice::ReadGroup | QFileDevice::ReadOther);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, ".luarc.json generated");
    } else if (fileHashCalc(":/config/.luarc.json") != fileHashCalc(luarcPath)) {
        QFile::remove(luarcPath);
        QFile::copy(":/config/.luarc.json", luarcPath);
        QFile::setPermissions(luarcPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                         | QFileDevice::ReadUser | QFileDevice::WriteUser
                                         | QFileDevice::ReadGroup | QFileDevice::ReadOther);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, ".luarc.json updated");
    }
    if (const QString libdPath = QDir(rootPath).filePath("lib.d.lua"); !QFile::exists(libdPath)) {
        QFile::copy(":/config/lib.d.lua", libdPath);
        QFile::setPermissions(libdPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                        | QFileDevice::ReadUser | QFileDevice::WriteUser
                                        | QFileDevice::ReadGroup | QFileDevice::ReadOther);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "lib.d.lua generated");
    } else if (fileHashCalc(":/config/lib.d.lua") != fileHashCalc(libdPath)) {
        QFile::remove(libdPath);
        QFile::copy(":/config/lib.d.lua", libdPath);
        QFile::setPermissions(libdPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                        | QFileDevice::ReadUser | QFileDevice::WriteUser
                                        | QFileDevice::ReadGroup | QFileDevice::ReadOther);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "lib.d.lua updated");
    }
    emit openWorkspace(rootUrl);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "workspace opened");
}

// MainWindow protected
void MainWindow::closeEvent(QCloseEvent *event) {
    const QMessageBox::StandardButton reply =
            QMessageBox::question(this, "Exit", "Save and exit?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (reply == QMessageBox::Yes) {
        workspaceSave();
        event->accept();
    } else {
        event->ignore();
    }
}

// MainWindow private
void MainWindow::configInit() {
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "initializing config");
    m_configModule = new ConfigModule;
    m_configModule->configInit();
    m_mainConfig = g_config["mainConfig"].toObject();
}

void MainWindow::moduleInit() {
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "initializing module");

    m_llsModule = new LuaLanguageServer(this);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "lls module initialized");

    m_undoModule = new UndoModule(this);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "undo module initialized");

    m_portModule = new PortModule();
    m_portModule->setObjectName("portModule");
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "port module initialized");

    m_explorerModule = new ExplorerModule();
    m_explorerModule->setObjectName("explorerModule");
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "explorer module initialized");

    m_structureModule = new StructureModule();
    m_structureModule->setObjectName("structureModule");
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "structure module initialized");

    m_sendModule = new SendModule();
    m_sendModule->setObjectName("sendModule");
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "send module initialized");

    m_databaseModule = new DatabaseModule();
    m_databaseModule->setObjectName("databaseModule");
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "database module initialized");

    m_datatableModule = new DatatableModule();
    m_datatableModule->setObjectName("datatableModule");
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "datatable module initialized");

    m_dataplotModule = new DataplotModule();
    m_dataplotModule->setObjectName("dataplotModule");
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "dataplot module initialized");

    m_logModule = new LogModule();
    m_logModule->setObjectName("logModule");
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "log module initialized");

    m_diagnosticsModule = new DiagnosticsModule();
    m_diagnosticsModule->setObjectName("diagnosticsModule");
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "diagnostics module initialized");

    m_debugModule = new DebugModule();
    m_debugModule->setObjectName("debugModule");
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "debug module initialized");

    m_threadpoolModule = new ThreadpoolModule();
    m_threadpoolModule->setObjectName("threadpoolModule");
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "threadpool module initialized");

    m_scriptModule = new ScriptModule();
    m_scriptModule->setObjectName("scriptModule");
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "script module initialized");

    connect(this, &MainWindow::appendLog, m_logModule, &LogModule::logAppend);
    connect(this, &MainWindow::openWorkspace, m_llsModule, &LuaLanguageServer::workspaceOpen);
    connect(this, &MainWindow::openWorkspace, m_scriptModule, &ScriptModule::workspaceOpen);
    connect(this, &MainWindow::openWorkspace, m_explorerModule, &ExplorerModule::workspaceOpen);
    connect(this, &MainWindow::openWorkspace, m_threadpoolModule, &ThreadpoolModule::workspaceOpen);
    connect(m_configModule, &ConfigModule::appendLog, m_logModule, &LogModule::logAppend);
    connect(m_llsModule, &LuaLanguageServer::returnPublishDiagnostics, m_scriptModule, &ScriptModule::diagnosticsReturn);
    connect(m_llsModule, &LuaLanguageServer::returnPublishDiagnostics, m_diagnosticsModule, &DiagnosticsModule::diagnosticsReturn);
    connect(m_llsModule, &LuaLanguageServer::returnCompletion, m_scriptModule, &ScriptModule::completionReturn);
    connect(m_llsModule, &LuaLanguageServer::returnDocumentSymbol, m_structureModule, &StructureModule::documentSymbolReturn);
    connect(m_llsModule, &LuaLanguageServer::returnFoldingRange, m_scriptModule, &ScriptModule::foldingRangeReturn);
    connect(m_llsModule, &LuaLanguageServer::returnFormatting, m_scriptModule, &ScriptModule::formattingReturn);
    connect(m_llsModule, &LuaLanguageServer::returnHover, m_scriptModule, &ScriptModule::hoverReturn);
    connect(m_llsModule, &LuaLanguageServer::returnSemanticTokens, m_scriptModule, &ScriptModule::semanticTokensReturn);
    connect(m_llsModule, &LuaLanguageServer::returnSignatureHelp, m_scriptModule, &ScriptModule::signatureHelpReturn);
    connect(m_portModule, &PortModule::appendLog, m_logModule, &LogModule::logAppend);
    connect(m_explorerModule, &ExplorerModule::appendLog, m_logModule, &LogModule::logAppend);
    connect(m_explorerModule, &ExplorerModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_explorerModule, &ExplorerModule::runScript, m_threadpoolModule, &ThreadpoolModule::threadRun);
    connect(m_explorerModule, &ExplorerModule::debugScript, m_threadpoolModule, &ThreadpoolModule::threadDebug);
    connect(m_structureModule, &StructureModule::showMarker, m_scriptModule, &ScriptModule::markerShow);
    connect(m_datatableModule, &DatatableModule::addGraphDataPlot, m_dataplotModule, &DataplotModule::dataplotAddGraph);
    connect(m_datatableModule, &DatatableModule::addPointDataPlot, m_dataplotModule, &DataplotModule::dataplotAddPoint);
    connect(m_dataplotModule, &DataplotModule::addGraphDatatable, m_datatableModule, &DatatableModule::datatableAddGraph);
    connect(m_diagnosticsModule, &DiagnosticsModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_diagnosticsModule, &DiagnosticsModule::setCursorPosition, m_scriptModule, &ScriptModule::cursorPositionSet);
    connect(m_diagnosticsModule, &DiagnosticsModule::showIndicator, m_scriptModule, &ScriptModule::indicatorShow);
    connect(m_debugModule, &DebugModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_debugModule, &DebugModule::showMarker, m_scriptModule, &ScriptModule::markerShow);
    connect(m_threadpoolModule, &ThreadpoolModule::startDebug, m_debugModule, &DebugModule::debugStart);
    connect(m_scriptModule, &ScriptModule::requestJson, m_llsModule, &LuaLanguageServer::jsonRequest);
    connect(m_scriptModule, &ScriptModule::notificationJson, m_llsModule, &LuaLanguageServer::jsonNotification);
    connect(m_scriptModule, &ScriptModule::appendLog, m_logModule, &LogModule::logAppend);
    connect(m_scriptModule, &ScriptModule::openWorkspace, this, &MainWindow::workspaceOpen);
    connect(m_scriptModule, &ScriptModule::switchScript, m_structureModule, &StructureModule::scriptSwitch);
    connect(m_scriptModule, &ScriptModule::insertBreakpoint, m_debugModule, &DebugModule::breakpointInsert);
    connect(m_scriptModule, &ScriptModule::removeBreakpoint, m_debugModule, &DebugModule::breakpointRemove);

    g_database = m_databaseModule;
    g_datatable = m_datatableModule;
    g_dataplot = m_dataplotModule;
    g_debug = m_debugModule;
    g_log = m_logModule;
    g_port = m_portModule;
    g_script = m_scriptModule;
    g_threadpool = m_threadpoolModule;
    g_undo = m_undoModule;
}

void MainWindow::workspaceInit() {
    QUrl rootUrl{};
    // check if workspace is valid
    if (rootUrl = QUrl(m_mainConfig["workspace"].toString()); QFileInfo::exists(rootUrl.toLocalFile())) {
        // check if lua config files exist
        const QString rootPath = rootUrl.toLocalFile();
        if (const QString luarcPath = QDir(rootPath).filePath(".luarc.json"); !QFile::exists(luarcPath)) {
            QFile::copy(":/config/.luarc.json", luarcPath);
            QFile::setPermissions(luarcPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                             | QFileDevice::ReadUser | QFileDevice::WriteUser
                                             | QFileDevice::ReadGroup | QFileDevice::ReadOther);
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] %2").arg(timestamp, ".luarc.json generated");
        } else if (fileHashCalc(":/config/.luarc.json") != fileHashCalc(luarcPath)) {
            QFile::remove(luarcPath);
            QFile::copy(":/config/.luarc.json", luarcPath);
            QFile::setPermissions(luarcPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                             | QFileDevice::ReadUser | QFileDevice::WriteUser
                                             | QFileDevice::ReadGroup | QFileDevice::ReadOther);
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] %2").arg(timestamp, ".luarc.json updated");
        }
        if (const QString libdPath = QDir(rootPath).filePath("lib.d.lua"); !QFile::exists(libdPath)) {
            QFile::copy(":/config/lib.d.lua", libdPath);
            QFile::setPermissions(libdPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                            | QFileDevice::ReadUser | QFileDevice::WriteUser
                                            | QFileDevice::ReadGroup | QFileDevice::ReadOther);
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] %2").arg(timestamp, "lib.d.lua generated");
        } else if (fileHashCalc(":/config/lib.d.lua") != fileHashCalc(libdPath)) {
            QFile::remove(libdPath);
            QFile::copy(":/config/lib.d.lua", libdPath);
            QFile::setPermissions(libdPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                            | QFileDevice::ReadUser | QFileDevice::WriteUser
                                            | QFileDevice::ReadGroup | QFileDevice::ReadOther);
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] %2").arg(timestamp, "lib.d.lua updated");
        }
        emit openWorkspace(rootUrl);
    }
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "workspace loaded");
}

void MainWindow::shortcutInit() {
    auto shortcutConfig = g_config["shortcutConfig"].toObject();
    m_openWorkspaceShortcut = new QShortcut(QKeySequence(shortcutConfig["openWorkspace"].toString()), this); // NOLINT
    connect(m_openWorkspaceShortcut, &QShortcut::activated, this, [this] { workspaceOpen(); });
    m_saveWorkspaceShortcut = new QShortcut(QKeySequence(shortcutConfig["saveWorkspace"].toString()), this); // NOLINT
    connect(m_saveWorkspaceShortcut, &QShortcut::activated, this, [this] { workspaceSave(); });
    m_saveWorkspaceAsShortcut = new QShortcut(QKeySequence(shortcutConfig["saveWorkspaceAs"].toString()), this); // NOLINT
    connect(m_saveWorkspaceAsShortcut, &QShortcut::activated, this, [this] {
        const QString filePath = QFileDialog::getSaveFileName(
            nullptr,
            tr("Save Workspace As"),
            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/config",
            "JSON File (*.json)"
        );
        if (filePath.endsWith(".json", Qt::CaseInsensitive)) {
            workspaceSave(filePath);
        }
    });
}

void MainWindow::menuInit() {
    auto *menuBar = new QMenuBar(); // NOLINT
    setMenuBar(menuBar);
    // file menu
    {
        auto *fileMenu = new QMenu(tr("File")); // NOLINT
        menuBar->addMenu(fileMenu);
        auto shortcutConfig = g_config["shortcutConfig"].toObject();
        auto *openWorkspaceAction = new QAction(tr("Open Workspace") + "\t" + shortcutConfig["openWorkspace"].toString()); // NOLINT
        fileMenu->addAction(openWorkspaceAction);
        connect(openWorkspaceAction, &QAction::triggered, this, [this] { workspaceOpen(); });
        auto *saveWorkspaceAction = new QAction(tr("Save Workspace") + "\t" + shortcutConfig["saveWorkspace"].toString()); // NOLINT
        fileMenu->addAction(saveWorkspaceAction);
        connect(saveWorkspaceAction, &QAction::triggered, this, [this] { workspaceSave(); });
        auto *saveWorkspaceAsAction = new QAction(tr("Save Workspace As") + "\t" + shortcutConfig["saveWorkspaceAs"].toString()); // NOLINT
        fileMenu->addAction(saveWorkspaceAsAction);
        connect(saveWorkspaceAsAction, &QAction::triggered, this, [this] {
            const QString filePath = QFileDialog::getSaveFileName(
                nullptr,
                tr("Save Workspace As"),
                QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/config",
                "JSON File (*.json)"
            );
            if (filePath.endsWith(".json", Qt::CaseInsensitive)) {
                workspaceSave(filePath);
            }
        });
    }
    // edit menu
    // {
    //     auto *fileMenu = new QMenu(tr("Edit")); // NOLINT
    //     menuBar->addMenu(fileMenu);
    //     auto *undoAction = new QAction(tr("Undo")); // NOLINT
    //     fileMenu->addAction(undoAction);
    //     auto *redoAction = new QAction(tr("Redo")); // NOLINT
    //     fileMenu->addAction(redoAction);
    // }
    // view menu
    {
        auto *viewMenu = new QMenu(tr("View")); // NOLINT
        menuBar->addMenu(viewMenu);
        viewMenu->addAction(m_portModule->toggleAction());
        m_portModule->toggleAction()->setText(tr("Port"));
        viewMenu->addAction(m_explorerModule->toggleAction());
        m_explorerModule->toggleAction()->setText(tr("Explorer"));
        viewMenu->addAction(m_structureModule->toggleAction());
        m_structureModule->toggleAction()->setText(tr("Structure"));
        viewMenu->addAction(m_sendModule->toggleAction());
        m_sendModule->toggleAction()->setText(tr("Send"));
        viewMenu->addAction(m_portModule->toggleAction());
        m_databaseModule->toggleAction()->setText(tr("Database"));
        viewMenu->addAction(m_databaseModule->toggleAction());
        m_datatableModule->toggleAction()->setText(tr("Data Table"));
        viewMenu->addAction(m_datatableModule->toggleAction());
        m_dataplotModule->toggleAction()->setText(tr("Data Plot"));
        viewMenu->addAction(m_dataplotModule->toggleAction());
        m_logModule->toggleAction()->setText(tr("Log"));
        viewMenu->addAction(m_logModule->toggleAction());
        viewMenu->addAction(m_diagnosticsModule->toggleAction());
        m_diagnosticsModule->toggleAction()->setText(tr("Diagnostics"));
        viewMenu->addAction(m_debugModule->toggleAction());
        m_debugModule->toggleAction()->setText(tr("Debug"));
        viewMenu->addAction(m_threadpoolModule->toggleAction());
        m_threadpoolModule->toggleAction()->setText(tr("Thread Pool"));
    }
    // control widget
    {
        auto *ctrlWidget = new QWidget(); // NOLINT
        menuBar->setCornerWidget(ctrlWidget);
        auto *ctrlLayout = new QHBoxLayout(ctrlWidget); // NOLINT
        ctrlLayout->setContentsMargins(0, 0, 0, 0);
        ctrlLayout->setAlignment(Qt::AlignRight);
        auto *runButton = new QPushButton(); // NOLINT
        ctrlLayout->addWidget(runButton);
        runButton->setFixedSize(24, 24);
        runButton->setIcon(QIcon(":/icon/play.svg"));
        connect(runButton, &QPushButton::clicked, this, [this] {
            if (m_scriptModule->m_focusedPage == nullptr) {
                QMessageBox::critical(this, tr("Error"), tr("Please open a script first."));
            }
            else {
                const QUrl scriptUrl = m_scriptModule->m_focusedPage->m_scriptUrl;
                const QString script = m_scriptModule->m_focusedPage->m_scriptEditor->text();
                emit runThread(scriptUrl, script);
            }
        });
        auto *debugButton = new QPushButton(); // NOLINT
        ctrlLayout->addWidget(debugButton);
        debugButton->setFixedSize(24, 24);
        debugButton->setIcon(QIcon(":/icon/bug.svg"));
        connect(debugButton, &QPushButton::clicked, this, [this] {
            if (m_scriptModule->m_focusedPage == nullptr) {
                QMessageBox::critical(this, tr("Error"), tr("Please open a script first."));
            }
            else {
                const QUrl scriptUrl = m_scriptModule->m_focusedPage->m_scriptUrl;
                const QString script = m_scriptModule->m_focusedPage->m_scriptEditor->text();
                emit debugThread(scriptUrl, script);
            }
        });
        connect(this, &MainWindow::runThread, m_threadpoolModule, &ThreadpoolModule::threadRun);
        connect(this, &MainWindow::debugThread, m_threadpoolModule, &ThreadpoolModule::threadDebug);
    }
}

void MainWindow::layoutInit() {
    if (!m_mainConfig["geometry"].toString().isEmpty()) {
        const QByteArray geometry = QByteArray::fromBase64(m_mainConfig["geometry"].toString().toLatin1());
        restoreGeometry(geometry);
    }
    if (m_mainConfig["state"].toString().isEmpty()) {
        // dock placement
        addDockWidget(m_portModule, KDDockWidgets::Location_OnLeft, nullptr);
        addDockWidget(m_explorerModule, KDDockWidgets::Location_OnBottom, m_portModule);
        addDockWidget(m_structureModule, KDDockWidgets::Location_OnBottom, m_explorerModule);
        addDockWidget(m_scriptModule->welcomePage(), KDDockWidgets::Location_OnRight);
        addDockWidget(m_sendModule, KDDockWidgets::Location_OnRight, nullptr, KDDockWidgets::InitialVisibilityOption::StartHidden);
        addDockWidget(m_databaseModule, KDDockWidgets::Location_OnBottom, m_sendModule, KDDockWidgets::InitialVisibilityOption::StartHidden);
        addDockWidget(m_datatableModule, KDDockWidgets::Location_OnBottom, m_databaseModule, KDDockWidgets::InitialVisibilityOption::StartHidden);
        addDockWidget(m_logModule, KDDockWidgets::Location_OnBottom);
        m_logModule->addDockWidgetAsTab(m_diagnosticsModule);
        m_logModule->addDockWidgetAsTab(m_debugModule);
        addDockWidget(m_threadpoolModule, KDDockWidgets::Location_OnRight, m_logModule);
        // dock resize
        // still figuring out how to do this
    } else {
        m_scriptModule->scriptLoad();
        const QByteArray layoutData = QByteArray::fromBase64(m_mainConfig["state"].toString().toLatin1());
        KDDockWidgets::LayoutSaver layoutSaver;
        layoutSaver.restoreLayout(layoutData);
    }
}

void MainWindow::mainConfigSave() {
    m_mainConfig["geometry"] = QString(saveGeometry().toBase64());
    const KDDockWidgets::LayoutSaver layoutSaver;
    const QByteArray layoutData = layoutSaver.serializeLayout();
    m_mainConfig["state"] = QString(layoutData.toBase64());
    g_config["mainConfig"] = m_mainConfig;
}

void MainWindow::workspaceSave(const QString &filePath) {
    m_scriptModule->scriptConfigSave();
    m_portModule->portConfigSave();
    m_sendModule->sendConfigSave();
    m_databaseModule->databaseConfigSave();
    m_datatableModule->datatableConfigSave();
    m_logModule->logConfigSave();
    mainConfigSave();
    m_configModule->configSave(filePath);
}
