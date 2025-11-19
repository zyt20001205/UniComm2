#include "mainWindow.h"

#include <QCameraDevice>
#include <QCloseEvent>
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMediaDevices>
#include <QMenuBar>
#include <QMessageBox>
#include <QShortcut>
#include <QStandardPaths>
#include <QThread>
#include <QToolBar>
#include <qtoolbutton.h>
#include <kddockwidgets/LayoutSaver.h>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <kddockwidgets/qtwidgets/views/MainWindow.h>

#include "configModule.h"
#include "globals.h"
#include "logModule.h"
#include "undoModule.h"
#include "dataModule/databaseModule.h"
#include "dataModule/dataplotModule.h"
#include "dataModule/datatableModule.h"
#include "luaModule/luaInterpreter.h"
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
#include "settingModule/settingModule.h"

// MainWindow public
MainWindow::MainWindow(QWidget *parent, const QString &uniqueName)
    : KDDockWidgets::QtWidgets::MainWindow(uniqueName, KDDockWidgets::MainWindowOption_None, parent),
      m_scriptComboBox(new QComboBox()) {
    // mainWindow ui init
    g_mainWindow = this;
    QWidget::setWindowTitle("UniComm");
    QWidget::setWindowIcon(QIcon(":/icon/icon.ico"));
    QWidget::resize(1600, 900);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "main window created");

    moduleInit();
    shortcutInit();
    menuInit();
    layoutInit();
    // preload multimedia to avoid lagging on port selection
    QThread *worker = QThread::create([] {
        QMediaDevices::videoInputs();
    });
    worker->start();
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
}

// MainWindow protected
void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_askForSaving) {
        const QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Exit"),
            tr("Save and exit?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes);
        if (reply == QMessageBox::Yes) {
            workspaceSave();
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

// MainWindow private
void MainWindow::moduleInit() {
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "initializing module");

    m_configModule = new ConfigModule(this);
    m_mainConfig = g_workspaceConfig["mainConfig"].toObject();
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "config module initialized");

    m_llsModule = new LuaLanguageServer(this);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "lls module initialized");

    m_undoModule = new UndoModule(this);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "undo module initialized");

    m_settingModule = new SettingModule(this);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "setting module initialized");

    m_scriptModule = new ScriptModule();
    m_scriptModule->setObjectName("scriptModule");
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "script module initialized");

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

    connect(this, &MainWindow::appendLog, m_logModule, &LogModule::logAppend);
    connect(m_scriptComboBox, &QComboBox::activated, m_scriptModule, [this] {
        const QUrl scriptUrl = m_scriptComboBox->currentData().toUrl();
        m_scriptModule->scriptOpen(scriptUrl);
    });
    connect(m_configModule, &ConfigModule::appendLog, m_logModule, &LogModule::logAppend);
    connect(m_llsModule, &LuaLanguageServer::notificationPublishDiagnostics, m_scriptModule, &ScriptModule::diagnosticsNotification);
    connect(m_llsModule, &LuaLanguageServer::notificationPublishDiagnostics, m_diagnosticsModule, &DiagnosticsModule::diagnosticsNotification);
    connect(m_llsModule, &LuaLanguageServer::responseCompletion, m_scriptModule, &ScriptModule::completionResponse);
    connect(m_llsModule, &LuaLanguageServer::responseDefinition, m_scriptModule, &ScriptModule::definitionResponse);
    connect(m_llsModule, &LuaLanguageServer::responseDocumentHighlight, m_scriptModule, &ScriptModule::documentHighlightResponse);
    connect(m_llsModule, &LuaLanguageServer::responseDocumentSymbol, m_structureModule, &StructureModule::documentSymbolResponse);
    connect(m_llsModule, &LuaLanguageServer::responseFoldingRange, m_scriptModule, &ScriptModule::foldingRangeResponse);
    connect(m_llsModule, &LuaLanguageServer::responseFormatting, m_scriptModule, &ScriptModule::formattingResponse);
    connect(m_llsModule, &LuaLanguageServer::responseHover, m_scriptModule, &ScriptModule::hoverResponse);
    connect(m_llsModule, &LuaLanguageServer::responseReferences, m_scriptModule, &ScriptModule::referencesResponse);
    connect(m_llsModule, &LuaLanguageServer::responseSemanticTokens, m_scriptModule, &ScriptModule::semanticTokensResponse);
    connect(m_llsModule, &LuaLanguageServer::responseSignatureHelp, m_scriptModule, &ScriptModule::signatureHelpResponse);
    connect(m_settingModule, &SettingModule::reloadLogFont, m_logModule, &LogModule::logFontReload);
    connect(m_settingModule, &SettingModule::saveLogFont, m_logModule, &LogModule::logFontSave);
    connect(m_settingModule, &SettingModule::reloadScriptFont, m_scriptModule, &ScriptModule::scriptFontReload);
    connect(m_settingModule, &SettingModule::saveScriptFont, m_scriptModule, &ScriptModule::scriptFontSave);
    connect(m_settingModule, &SettingModule::reloadScriptIndicator, m_scriptModule, &ScriptModule::scriptIndicatorReload);
    connect(m_settingModule, &SettingModule::saveScriptIndicator, m_scriptModule, &ScriptModule::scriptIndicatorSave);
    connect(m_settingModule, &SettingModule::reloadScriptMarker, m_scriptModule, &ScriptModule::scriptMarkerReload);
    connect(m_settingModule, &SettingModule::saveScriptMarker, m_scriptModule, &ScriptModule::scriptMarkerSave);
    connect(m_scriptModule, &ScriptModule::requestJson, m_llsModule, &LuaLanguageServer::jsonRequest);
    connect(m_scriptModule, &ScriptModule::notificationJson, m_llsModule, &LuaLanguageServer::jsonNotification);
    connect(m_scriptModule, &ScriptModule::appendLog, m_logModule, &LogModule::logAppend);
    connect(m_scriptModule, &ScriptModule::openWorkspace, this, &MainWindow::workspaceOpen);
    connect(m_scriptModule, &ScriptModule::openScript, this, [this](const QUrl &scriptUrl) {
        const QString scriptName = scriptUrl.fileName();
        m_scriptComboBox->addItem(scriptName, scriptUrl);
    });
    connect(m_scriptModule, &ScriptModule::closeScript, this, [this](const QUrl &scriptUrl) {
        if (const int index = m_scriptComboBox->findData(scriptUrl); index != -1) {
            m_scriptComboBox->removeItem(index);
        }
    });
    connect(m_scriptModule, &ScriptModule::focusScript, this, [this](const QUrl &scriptUrl) {
        if (const int index = m_scriptComboBox->findData(scriptUrl); index != -1) {
            m_scriptComboBox->setCurrentIndex(index);
        }
    });
    connect(m_scriptModule, &ScriptModule::focusScript, m_structureModule, &StructureModule::scriptFocus);
    connect(m_scriptModule, &ScriptModule::insertPort, m_portModule, [this] {
        m_portModule->portInsert(-1, QJsonObject());
        m_portModule->portAnnotate();
    });
    connect(m_scriptModule, &ScriptModule::insertDatabase, m_databaseModule, [this] {
        m_databaseModule->databaseInsert(-1, QString());
        m_databaseModule->databaseAnnotate();
    });
    connect(m_scriptModule, &ScriptModule::insertDatatable, m_datatableModule, [this] {
        m_datatableModule->datatableInsert(-1, QString());
        m_datatableModule->datatableAnnotate();
    });
    connect(m_scriptModule, &ScriptModule::insertBreakpoint, m_debugModule, &DebugModule::breakpointInsert);
    connect(m_scriptModule, &ScriptModule::removeBreakpoint, m_debugModule, &DebugModule::breakpointRemove);
    connect(m_portModule, &PortModule::appendLog, m_logModule, &LogModule::logAppend);
    connect(m_explorerModule, &ExplorerModule::appendLog, m_logModule, &LogModule::logAppend);
    connect(m_explorerModule, &ExplorerModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_explorerModule, &ExplorerModule::runScript, m_threadpoolModule, &ThreadpoolModule::threadRun);
    connect(m_explorerModule, &ExplorerModule::debugScript, m_threadpoolModule, &ThreadpoolModule::threadDebug);
    connect(m_structureModule, &StructureModule::insertMarker, m_scriptModule, &ScriptModule::markerInsert);
    connect(m_databaseModule, &DatabaseModule::appendLog, m_logModule, &LogModule::logAppend);
    connect(m_datatableModule, &DatatableModule::appendLog, m_logModule, &LogModule::logAppend);
    connect(m_datatableModule, &DatatableModule::addGraphDataPlot, m_dataplotModule, &DataplotModule::dataplotAddGraph);
    connect(m_datatableModule, &DatatableModule::addPointDataPlot, m_dataplotModule, &DataplotModule::dataplotAddPoint);
    connect(m_dataplotModule, &DataplotModule::addGraphDatatable, m_datatableModule, &DatatableModule::datatableAddGraph);
    connect(m_diagnosticsModule, &DiagnosticsModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_diagnosticsModule, &DiagnosticsModule::setCursorPosition, m_scriptModule, &ScriptModule::cursorPositionSet);
    connect(m_diagnosticsModule, &DiagnosticsModule::insertIndicator, m_scriptModule, &ScriptModule::indicatorInsert);
    connect(m_debugModule, &DebugModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_debugModule, &DebugModule::insertMarker, m_scriptModule, &ScriptModule::markerInsert);
    connect(m_threadpoolModule, &ThreadpoolModule::startDebug, m_debugModule, &DebugModule::debugStart);

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

void MainWindow::shortcutInit() {
    auto shortcutConfig = g_workspaceConfig["shortcutConfig"].toObject();
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
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "shortcut initialized");
}

void MainWindow::menuInit() {
    auto *toolBar = new QToolBar(); // NOLINT
    addToolBar(Qt::TopToolBarArea, toolBar);
    // file menu
    {
        auto *fileMenu = new QMenu(tr("File")); // NOLINT
        auto *fileButton = new QToolButton(); // NOLINT
        fileButton->setText(tr("File"));
        fileButton->setMenu(fileMenu);
        fileButton->setPopupMode(QToolButton::InstantPopup);
        toolBar->addWidget(fileButton);

        auto shortcutConfig = g_workspaceConfig["shortcutConfig"].toObject();
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
        auto *viewButton = new QToolButton(); // NOLINT
        viewButton->setText(tr("View"));
        viewButton->setMenu(viewMenu);
        viewButton->setPopupMode(QToolButton::InstantPopup);
        toolBar->addWidget(viewButton);

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
    // setting menu
    {
        auto *settingAction = new QAction(tr("Setting"), this); // NOLINT
        toolBar->addAction(settingAction);
        connect(settingAction, &QAction::triggered, this, [this] {
            const QJsonObject logConfig = g_workspaceConfig["logConfig"].toObject();
            const QJsonObject scriptConfig = g_workspaceConfig["scriptConfig"].toObject();
            const QJsonObject settingConfig = {
                {"fontFamilyLog", logConfig["fontFamily"].toString()},
                {"fontSizeLog", logConfig["fontSize"].toInt()},
                {"fontFamilyScript", scriptConfig["fontFamily"].toString()},
                {"fontSizeScript", scriptConfig["fontSize"].toInt()},
                {"indicatorErrorStyleScript", scriptConfig["indicatorErrorStyle"].toInt()},
                {"indicatorErrorColorScript", scriptConfig["indicatorErrorColor"].toString()},
                {"indicatorWarningStyleScript", scriptConfig["indicatorWarningStyle"].toInt()},
                {"indicatorWarningColorScript", scriptConfig["indicatorWarningColor"].toString()},
                {"indicatorInfoStyleScript", scriptConfig["indicatorInfoStyle"].toInt()},
                {"indicatorInfoColorScript", scriptConfig["indicatorInfoColor"].toString()},
                {"indicatorHintStyleScript", scriptConfig["indicatorHintStyle"].toInt()},
                {"indicatorHintColorScript", scriptConfig["indicatorHintColor"].toString()},
                {"indicatorHighlightStyleScript", scriptConfig["indicatorHighlightStyle"].toInt()},
                {"indicatorHighlightColorScript", scriptConfig["indicatorHighlightColor"].toString()},
                {"indicatorReadStyleScript", scriptConfig["indicatorReadStyle"].toInt()},
                {"indicatorReadColorScript", scriptConfig["indicatorReadColor"].toString()},
                {"indicatorWriteStyleScript", scriptConfig["indicatorWriteStyle"].toInt()},
                {"indicatorWriteColorScript", scriptConfig["indicatorWriteColor"].toString()},
                {"indicatorSearchStyleScript", scriptConfig["indicatorSearchStyle"].toInt()},
                {"indicatorSearchColorScript", scriptConfig["indicatorSearchColor"].toString()},
                {"indicatorSelectionStyleScript", scriptConfig["indicatorSelectionStyle"].toInt()},
                {"indicatorSelectionColorScript", scriptConfig["indicatorSelectionColor"].toString()},
                {"indicatorHyperlinkStyleScript", scriptConfig["indicatorHyperlinkStyle"].toInt()},
                {"indicatorHyperlinkColorScript", scriptConfig["indicatorHyperlinkColor"].toString()},
                {"markerBreakpointStyleScript", scriptConfig["markerBreakpointStyle"].toInt()},
                {"markerBreakpointBackgroundScript", scriptConfig["markerBreakpointBackground"].toString()},
                {"markerBreakpointForegroundScript", scriptConfig["markerBreakpointForeground"].toString()},
                {"markerDebugStyleScript", scriptConfig["markerDebugStyle"].toInt()},
                {"markerDebugBackgroundScript", scriptConfig["markerDebugBackground"].toString()},
                {"markerDebugForegroundScript", scriptConfig["markerDebugForeground"].toString()}
            };
            m_settingModule->settingImport(settingConfig);
            if (m_settingModule->exec() == QDialog::Accepted) {
                workspaceSave();
            }
        });
    }
    // separator
    auto *spacer = new QWidget(); // NOLINT
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolBar->addWidget(spacer);
    toolBar->addSeparator();
    // control menu
    {
        toolBar->addWidget(m_scriptComboBox);
        m_scriptComboBox->setFont(QFont("Consolas", 12, QFont::Bold));
        m_scriptComboBox->setStyleSheet("color: #333333;");
        // load script
        const QJsonObject scriptConfig = g_workspaceConfig["scriptConfig"].toObject();
        const QJsonArray scriptList = scriptConfig["scriptList"].toArray();
        for (const auto &value: scriptConfig["scriptList"].toArray()) {
            const auto scriptUrl = QUrl(value.toString());
            const QString scriptName = scriptUrl.fileName();
            m_scriptComboBox->addItem(scriptName, scriptUrl);
        }

        auto runScript = [this] {
            if (m_scriptModule->m_scriptPageHash.isEmpty()) {
                QMessageBox::critical(this, tr("Error"), tr("Please open a script first."));
            } else {
                const QUrl scriptUrl = m_scriptComboBox->currentData().toUrl();
                const QString script = m_scriptModule->m_scriptPageHash[scriptUrl]->m_scriptEditor->text();
                emit runThread(scriptUrl, script);
                m_logModule->raise();
            }
        };
        auto *runButton = new QToolButton(); // NOLINT
        toolBar->addWidget(runButton);
        runButton->setFixedSize(32, 32);
        runButton->setIcon(QIcon(":/icon/play.svg"));
        runButton->setToolTip(tr("Run Shift + F10"));
        connect(runButton, &QToolButton::clicked, this, runScript);
        const auto *runShortcut = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F10), this); // NOLINT
        connect(runShortcut, &QShortcut::activated, this, runScript);
        connect(this, &MainWindow::runThread, m_threadpoolModule, &ThreadpoolModule::threadRun);

        auto debugScript = [this] {
            if (m_scriptModule->m_scriptPageHash.isEmpty()) {
                QMessageBox::critical(this, tr("Error"), tr("Please open a script first."));
            } else {
                const QUrl scriptUrl = m_scriptComboBox->currentData().toUrl();
                const QString script = m_scriptModule->m_scriptPageHash[scriptUrl]->m_scriptEditor->text();
                qDebug() << scriptUrl << script;
                emit debugThread(scriptUrl, script);
                m_debugModule->raise();
            }
        };
        auto *debugButton = new QToolButton(); // NOLINT
        toolBar->addWidget(debugButton);
        debugButton->setFixedSize(32, 32);
        debugButton->setIcon(QIcon(":/icon/bug.svg"));
        debugButton->setToolTip(tr("Debug Shift + F9"));
        connect(debugButton, &QToolButton::clicked, this, debugScript);
        const auto *debugShortcut = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F9), this); // NOLINT
        connect(debugShortcut, &QShortcut::activated, this, debugScript);
        connect(this, &MainWindow::debugThread, m_threadpoolModule, &ThreadpoolModule::threadDebug);
    }
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "menu initialized");
}

void MainWindow::layoutInit() {
    if (!m_mainConfig["geometry"].toString().isEmpty()) {
        const QByteArray geometry = QByteArray::fromBase64(m_mainConfig["geometry"].toString().toLatin1());
        restoreGeometry(geometry);
    }
    if (m_mainConfig["state"].toString().isEmpty()) {
        addDockWidget(m_scriptModule->welcomePage(), KDDockWidgets::Location_OnRight);
        addDockWidget(m_portModule, KDDockWidgets::Location_OnLeft, m_scriptModule->welcomePage(), KDDockWidgets::InitialOption(KDDockWidgets::Size(100, 0)));
        addDockWidget(m_explorerModule, KDDockWidgets::Location_OnBottom, m_portModule);
        addDockWidget(m_structureModule, KDDockWidgets::Location_OnBottom, m_explorerModule);
        addDockWidget(m_sendModule, KDDockWidgets::Location_OnRight, m_scriptModule->welcomePage(), KDDockWidgets::InitialVisibilityOption::StartHidden);
        addDockWidget(m_databaseModule, KDDockWidgets::Location_OnBottom, m_sendModule, KDDockWidgets::InitialVisibilityOption::StartHidden);
        addDockWidget(m_datatableModule, KDDockWidgets::Location_OnBottom, m_databaseModule, KDDockWidgets::InitialVisibilityOption::StartHidden);
        addDockWidget(m_logModule, KDDockWidgets::Location_OnBottom);
        m_logModule->addDockWidgetAsTab(m_diagnosticsModule);
        m_logModule->addDockWidgetAsTab(m_debugModule);
        m_logModule->raise();
        addDockWidget(m_threadpoolModule, KDDockWidgets::Location_OnRight, m_logModule, KDDockWidgets::InitialOption(KDDockWidgets::Size(100, 0)));
    } else {
        const QByteArray layoutData = QByteArray::fromBase64(m_mainConfig["state"].toString().toLatin1());
        KDDockWidgets::LayoutSaver layoutSaver;
        layoutSaver.restoreLayout(layoutData);
    }
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "layout initialized");
}

void MainWindow::mainConfigSave() {
    m_mainConfig["geometry"] = QString(saveGeometry().toBase64());
    const KDDockWidgets::LayoutSaver layoutSaver;
    const QByteArray layoutData = layoutSaver.serializeLayout();
    m_mainConfig["state"] = QString(layoutData.toBase64());
    g_workspaceConfig["mainConfig"] = m_mainConfig;
}

void MainWindow::workspaceOpen() {
    const QString workspaceDir = QFileDialog::getExistingDirectory(
        g_mainWindow,
        tr("Open Workspace"),
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    if (workspaceDir.isEmpty()) return;
    if (g_workspaceUrl == QUrl::fromLocalFile(workspaceDir)) {
        qDebug() << "same as prev workspace";
        return;
    }
    workspaceSave();
    g_workspaceUrl = QUrl::fromLocalFile(workspaceDir);
    // write to main config
    const QJsonObject json{
        {"version", "1.0.0"},
        {"workspace", g_workspaceUrl.toString()},
    };
    const QJsonDocument doc(json);
    QFile mainConfig(QDir::current().filePath("config.json"));
    mainConfig.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    mainConfig.write(doc.toJson(QJsonDocument::Indented));
    mainConfig.close();
    // restart main process
    QProcess::startDetached(QCoreApplication::applicationFilePath());
    m_askForSaving = false;
    QApplication::quit();
}

void MainWindow::workspaceSave(QString filePath) {
    m_scriptModule->scriptConfigSave();
    m_portModule->portConfigSave();
    m_sendModule->sendConfigSave();
    m_databaseModule->databaseConfigSave();
    m_datatableModule->datatableConfigSave();
    m_logModule->logConfigSave();
    mainConfigSave();
    m_configModule->workspaceConfigSave(filePath);
}
