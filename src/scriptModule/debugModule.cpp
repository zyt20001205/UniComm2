#include "scriptModule/debugModule.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTableView>
#include <QThread>
#include <QTreeView>

#include "globals.h"
#include "luaModule/luaInterpreter.h"
#include "scriptModule/scriptModule.h"

// DebugModule public
DebugModule::DebugModule()
    : DockWidget("debug"),
      m_debugBreakpointsTableModel(new QStandardItemModel()),
      m_debugBreakpointsProxyModel(new BreakpointsProxyModel()),
      m_debugBreakpointsTableView(new QTableView()),
      m_debugTabWidget(new QTabWidget()),
      m_debugTabOverlay(new QWidget(m_debugTabWidget)) {
    auto *widget = new QWidget(); // NOLINT
    setWidget(widget);
    auto *layout = new QHBoxLayout(widget); // NOLINT
    // debug master control
    {
        auto *debugMasterCtrlWidget = new QWidget(); // NOLINT
        layout->addWidget(debugMasterCtrlWidget);
        auto *debugMasterCtrlLayout = new QVBoxLayout(debugMasterCtrlWidget); // NOLINT
        debugMasterCtrlLayout->setAlignment(Qt::AlignTop);
        debugMasterCtrlLayout->setContentsMargins(0, 0, 0, 0);
        debugMasterCtrlLayout->setSpacing(0);
        // debug ctrl
        {
            auto *debugCtrlLabel = new QLabel(tr("Command Bar")); // NOLINT
            debugMasterCtrlLayout->addWidget(debugCtrlLabel);
            auto *debugCtrlWidget = new QWidget(); // NOLINT
            debugMasterCtrlLayout->addWidget(debugCtrlWidget);
            auto *debugCtrlLayout = new QHBoxLayout(debugCtrlWidget); // NOLINT
            debugCtrlLayout->setContentsMargins(0, 0, 0, 0);
            debugCtrlLayout->setAlignment(Qt::AlignLeft);
            auto *debugStopButton = new QPushButton(); // NOLINT
            debugCtrlLayout->addWidget(debugStopButton);
            debugStopButton->setFixedSize(24, 24);
            debugStopButton->setIcon(QIcon(":/icon/stop.svg"));
            debugStopButton->setToolTip(tr("stop"));
            connect(debugStopButton, &QPushButton::clicked, this, [this] {
                if (m_debugPageHash.isEmpty()) {
                    QMessageBox::critical(this, tr("Error"), tr("No active debug session."));
                    return;
                }
                const int index = m_debugTabWidget->currentIndex();
                const QString threadId = m_debugTabWidget->tabText(index);
                const LuaInterpreter *interpreter = m_interpreterHash[threadId];
                interpreter->thread()->requestInterruption();
            });
            auto *debugResumeButton = new QPushButton(); // NOLINT
            debugCtrlLayout->addWidget(debugResumeButton);
            debugResumeButton->setFixedSize(24, 24);
            debugResumeButton->setIcon(QIcon(":/icon/play.svg"));
            debugResumeButton->setToolTip(tr("resume"));
            connect(debugResumeButton, &QPushButton::clicked, this, [this] {
                if (m_debugPageHash.isEmpty()) {
                    QMessageBox::critical(this, tr("Error"), tr("No active debug session."));
                    return;
                }
                const int index = m_debugTabWidget->currentIndex();
                const QString threadId = m_debugTabWidget->tabText(index);
                LuaInterpreter *interpreter = m_interpreterHash[threadId];
                QMetaObject::invokeMethod(interpreter->thread(), [interpreter] {
                    interpreter->debugStateSet(DEBUG_RESUME);
                }, Qt::QueuedConnection);
                emit resume(threadId);
            });
            auto *debugPauseButton = new QPushButton(); // NOLINT
            debugCtrlLayout->addWidget(debugPauseButton);
            debugPauseButton->setFixedSize(24, 24);
            debugPauseButton->setIcon(QIcon(":/icon/pause.svg"));
            debugPauseButton->setToolTip(tr("pause"));
            connect(debugPauseButton, &QPushButton::clicked, this, [this] {
                if (m_debugPageHash.isEmpty()) {
                    QMessageBox::critical(this, tr("Error"), tr("No active debug session."));
                    return;
                }
                const int index = m_debugTabWidget->currentIndex();
                const QString threadId = m_debugTabWidget->tabText(index);
                LuaInterpreter *interpreter = m_interpreterHash[threadId];
                QMetaObject::invokeMethod(interpreter->thread(), [interpreter] {
                    interpreter->debugStateSet(DEBUG_PAUSE);
                }, Qt::QueuedConnection);
                emit resume(threadId);
            });
            auto *debugStepOverButton = new QPushButton(); // NOLINT
            debugCtrlLayout->addWidget(debugStepOverButton);
            debugStepOverButton->setFixedSize(24, 24);
            debugStepOverButton->setIcon(QIcon(":/icon/debugStepOver.svg"));
            debugStepOverButton->setToolTip(tr("step over"));
            connect(debugStepOverButton, &QPushButton::clicked, this, [this] {
                if (m_debugPageHash.isEmpty()) {
                    QMessageBox::critical(this, tr("Error"), tr("No active debug session."));
                    return;
                }
                const int index = m_debugTabWidget->currentIndex();
                const QString threadId = m_debugTabWidget->tabText(index);
                LuaInterpreter *interpreter = m_interpreterHash[threadId];
                QMetaObject::invokeMethod(interpreter->thread(), [interpreter] {
                    interpreter->debugStateSet(DEBUG_STEPOVER);
                }, Qt::QueuedConnection);
                emit resume(threadId);
            });
            auto *debugStepIntoButton = new QPushButton(); // NOLINT
            debugCtrlLayout->addWidget(debugStepIntoButton);
            debugStepIntoButton->setFixedSize(24, 24);
            debugStepIntoButton->setIcon(QIcon(":/icon/debugStepInto.svg"));
            debugStepIntoButton->setToolTip(tr("step into"));
            connect(debugStepIntoButton, &QPushButton::clicked, this, [this] {
                if (m_debugPageHash.isEmpty()) {
                    QMessageBox::critical(this, tr("Error"), tr("No active debug session."));
                    return;
                }
                const int index = m_debugTabWidget->currentIndex();
                const QString threadId = m_debugTabWidget->tabText(index);
                LuaInterpreter *interpreter = m_interpreterHash[threadId];
                QMetaObject::invokeMethod(interpreter->thread(), [interpreter] {
                    interpreter->debugStateSet(DEBUG_STEPINTO);
                }, Qt::QueuedConnection);
                emit resume(threadId);
            });
            auto *debugStepOutButton = new QPushButton(); // NOLINT
            debugCtrlLayout->addWidget(debugStepOutButton);
            debugStepOutButton->setFixedSize(24, 24);
            debugStepOutButton->setIcon(QIcon(":/icon/debugStepOut.svg"));
            debugStepOutButton->setToolTip(tr("step out"));
            connect(debugStepOutButton, &QPushButton::clicked, this, [this] {
                if (m_debugPageHash.isEmpty()) {
                    QMessageBox::critical(this, tr("Error"), tr("No active debug session."));
                    return;
                }
                const int index = m_debugTabWidget->currentIndex();
                const QString threadId = m_debugTabWidget->tabText(index);
                LuaInterpreter *interpreter = m_interpreterHash[threadId];
                QMetaObject::invokeMethod(interpreter->thread(), [interpreter] {
                    interpreter->debugStateSet(DEBUG_STEPOUT);
                }, Qt::QueuedConnection);
                emit resume(threadId);
            });
            auto *debugRunToCursorButton = new QPushButton(); // NOLINT
            debugCtrlLayout->addWidget(debugRunToCursorButton);
            debugRunToCursorButton->setFixedSize(24, 24);
            debugRunToCursorButton->setIcon(QIcon(":/icon/debugContinue.svg"));
            debugRunToCursorButton->setToolTip(tr("run to cursor"));
            connect(debugRunToCursorButton, &QPushButton::clicked, this, [this] {
                if (m_debugPageHash.isEmpty()) {
                    QMessageBox::critical(this, tr("Error"), tr("No active debug session."));
                    return;
                }
                g_script->cursorPositionGet();
                const int index = m_debugTabWidget->currentIndex();
                const QString threadId = m_debugTabWidget->tabText(index);
                LuaInterpreter *interpreter = m_interpreterHash[threadId];
                QMetaObject::invokeMethod(interpreter->thread(), [interpreter] {
                    interpreter->debugStateSet(DEBUG_RUNTOCURSOR);
                }, Qt::QueuedConnection);
                emit resume(threadId);
            });
            auto *heatmapButton = new QPushButton(); // NOLINT
            debugCtrlLayout->addWidget(heatmapButton);
            heatmapButton->setCheckable(true);
            heatmapButton->setFixedSize(24, 24);
            heatmapButton->setIcon(QIcon(":/icon/dataBarVertical.svg"));
            heatmapButton->setToolTip(tr("line heatmap"));
            connect(heatmapButton, &QPushButton::clicked, this, [this, heatmapButton](const bool status) {
                if (m_debugPageHash.isEmpty()) {
                    heatmapButton->setChecked(false);
                    QMessageBox::critical(this, tr("Error"), tr("No active debug session."));
                    return;
                }
                const int index = m_debugTabWidget->currentIndex();
                const QString threadId = m_debugTabWidget->tabText(index);
                LuaInterpreter *interpreter = m_interpreterHash[threadId];
                QMetaObject::invokeMethod(interpreter->thread(), [interpreter, status] {
                    if (status) interpreter->showHeatmap();
                    else interpreter->hideHeatmap();
                }, Qt::QueuedConnection);
            });
        }
        // debug breakpoints
        {
            auto *debugBreakpointsLabel = new QLabel(tr("Breakpoints")); // NOLINT
            debugMasterCtrlLayout->addWidget(debugBreakpointsLabel);
            debugMasterCtrlLayout->addWidget(m_debugBreakpointsTableView);
            m_debugBreakpointsTableModel->setColumnCount(4);
            m_debugBreakpointsTableModel->setHorizontalHeaderLabels({tr("Script"), tr("Line"), tr("Expr"), tr("View")});
            m_debugBreakpointsProxyModel->setSourceModel(m_debugBreakpointsTableModel);
            m_debugBreakpointsTableView->setModel(m_debugBreakpointsProxyModel);
            m_debugBreakpointsTableView->sortByColumn(0, Qt::AscendingOrder);
            m_debugBreakpointsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
            m_debugBreakpointsTableView->setAlternatingRowColors(true);
            m_debugBreakpointsTableView->setShowGrid(false);
            m_debugBreakpointsTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
            m_debugBreakpointsTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
            m_debugBreakpointsTableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
            m_debugBreakpointsTableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
            m_debugBreakpointsTableView->verticalHeader()->setVisible(false);
            m_debugBreakpointsTableView->verticalHeader()->setDefaultSectionSize(24);
            connect(m_debugBreakpointsTableView, &QTableView::clicked, this, [this](const QModelIndex &index) {
                if (index.column() == 2) {
                    const QUrl scriptUrl = index.data(Qt::UserRole + 1).toUrl();
                    const int line = index.data(Qt::UserRole + 2).toInt();
                    const QString currentCondition = g_breakpoints[scriptUrl][line]["expr"].toString();
                    bool ok;
                    const QString newCondition = QInputDialog::getText(
                        this,
                        tr("Edit Breakpoint Condition"),
                        tr("Enter condition expression:"),
                        QLineEdit::Normal,
                        currentCondition,
                        &ok
                    );
                    if (ok) g_breakpoints[scriptUrl][line]["expr"] = newCondition;
                } else if (index.column() == 3) {
                    const QUrl scriptUrl = index.data(Qt::UserRole + 1).toUrl();
                    emit openScript(scriptUrl);
                    const int line = index.data(Qt::UserRole + 2).toInt();
                    emit insertMarker(scriptUrl, MARKER_HINT, line, 1000);
                }
            });
            // load breakpoint
            for (const auto &url: g_breakpoints.keys()) {
                const auto breakpointLineHash = g_breakpoints[url];
                for (auto it = breakpointLineHash.begin(); it != breakpointLineHash.end(); ++it) {
                    const int line = it.key();
                    const QVariantHash breakpointInfo = it.value();
                    breakpointInsert(url, line);
                }
            }
        }
    }
    // debug tabview
    {
        layout->addWidget(m_debugTabWidget);

        m_debugTabOverlay->setStyleSheet("background-color: rgba(0, 0, 0, 96);");
        auto *overlayLayout = new QVBoxLayout(m_debugTabOverlay); // NOLINT
        overlayLayout->setAlignment(Qt::AlignCenter);
        overlayLayout->setContentsMargins(0, 0, 0, 0);
        auto *overlayLabel = new QLabel(tr("No Active Debug Session")); // NOLINT
        overlayLayout->addWidget(overlayLabel);
        overlayLabel->setFont(QFont("Consolas", 12, QFont::Bold));
        overlayLabel->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: white;");
        overlayShow();
    }
    layout->setStretch(0, 0);
    layout->setStretch(1, 1);
}

void DebugModule::breakpointInsert(const QUrl &scriptUrl, const int line) const {
    const int row = m_debugBreakpointsTableModel->rowCount();
    m_debugBreakpointsTableModel->insertRow(row);
    m_debugBreakpointsTableModel->setItem(row, 0, new QStandardItem(scriptUrl.fileName()));
    m_debugBreakpointsTableModel->setItem(row, 1, new QStandardItem(QString::number(line)));
    auto *exprItem = new QStandardItem(QIcon(":/icon/function.svg"), ""); // NOLINT
    exprItem->setData(QVariant(scriptUrl), Qt::UserRole + 1);
    exprItem->setData(QVariant(line), Qt::UserRole + 2);
    m_debugBreakpointsTableModel->setItem(row, 2, exprItem);
    auto *viewItem = new QStandardItem(QIcon(":/icon/arrowRight.svg"), ""); // NOLINT
    viewItem->setData(QVariant(scriptUrl), Qt::UserRole + 1);
    viewItem->setData(QVariant(line), Qt::UserRole + 2);
    m_debugBreakpointsTableModel->setItem(row, 3, viewItem);
}

void DebugModule::breakpointRemove(const QUrl &scriptUrl, const int line) const {
    for (int row = 0; row < m_debugBreakpointsTableModel->rowCount(); ++row) {
        const QStandardItem *fileItem = m_debugBreakpointsTableModel->item(row, 0);
        const QStandardItem *lineItem = m_debugBreakpointsTableModel->item(row, 1);
        if (fileItem->text() == scriptUrl.fileName() && lineItem->text() == QString::number(line)) {
            m_debugBreakpointsTableModel->removeRow(row);
            break;
        }
    }
}

void DebugModule::debugStart(const QString &threadId, LuaInterpreter *interpreter) {
    m_interpreterHash.insert(threadId, interpreter);
    auto *debugPage = new DebugPage(interpreter); // NOLINT
    m_debugPageHash[threadId] = debugPage;
    m_debugTabWidget->addTab(debugPage, threadId);
    connect(interpreter->thread(), &QThread::finished, this, [this, threadId, debugPage] { debugEnd(threadId, debugPage); });
    connect(debugPage, &DebugPage::openScript, this, &DebugModule::openScript);
    connect(debugPage, &DebugPage::insertMarker, this, &DebugModule::insertMarker);
    overlayHide();
}

void DebugModule::debugEnd(const QString &threadId, const DebugPage *debugPage) {
    const int index = m_debugTabWidget->indexOf(debugPage);
    if (index != -1) m_debugTabWidget->removeTab(index);
    delete debugPage;
    m_interpreterHash.remove(threadId);
    m_debugPageHash.remove(threadId);
    if (m_debugPageHash.isEmpty()) overlayShow();
}

void DebugModule::varReturn(const QString &threadId, QStandardItemModel *varTree) {
    if (!m_debugPageHash.contains(threadId)) return;
    m_debugPageHash[threadId]->varLoad(varTree);
}

void DebugModule::callReturn(const QString &threadId, QStandardItemModel *callTable) {
    if (!m_debugPageHash.contains(threadId)) return;
    m_debugPageHash[threadId]->callLoad(callTable);
}

// DebugModule protected
void DebugModule::resizeEvent(QResizeEvent *event) {
    DockWidget::resizeEvent(event);
    if (!m_debugTabOverlay->isHidden()) overlayResize();
}

// DebugModule private
void DebugModule::overlayShow() const {
    overlayResize();
    m_debugTabOverlay->raise();
    m_debugTabOverlay->show();
}

void DebugModule::overlayHide() const {
    m_debugTabOverlay->hide();
}

void DebugModule::overlayResize() const {
    m_debugTabOverlay->resize(m_debugTabWidget->size());
    m_debugTabOverlay->move(0, 0);
}

BreakpointsProxyModel::BreakpointsProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent) {
}

bool BreakpointsProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const {
    // script column sort
    const QModelIndex leftScriptIndex = sourceModel()->index(source_left.row(), 0, QModelIndex());
    const QModelIndex rightScriptIndex = sourceModel()->index(source_right.row(), 0, QModelIndex());
    const QString leftScriptData = sourceModel()->data(leftScriptIndex, sortRole()).toString();
    const QString rightScriptData = sourceModel()->data(rightScriptIndex, sortRole()).toString();
    if (leftScriptData != rightScriptData) return leftScriptData < rightScriptData;
    // line column sort
    const QModelIndex leftLineIndex = sourceModel()->index(source_left.row(), 1, QModelIndex());
    const QModelIndex rightLineIndex = sourceModel()->index(source_right.row(), 1, QModelIndex());
    const int leftLineData = sourceModel()->data(leftLineIndex, sortRole()).toInt();
    const int rightLineData = sourceModel()->data(rightLineIndex, sortRole()).toInt();
    return leftLineData < rightLineData;
}

// DebugPage public
DebugPage::DebugPage(LuaInterpreter *interpreter, QWidget *parent)
    : QWidget(parent),
      m_interpreter(interpreter),
      m_varTreeView(new QTreeView()),
      m_callTableView(new QTableView()) {
    auto *layout = new QHBoxLayout(this); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *varWidget = new QWidget(); // NOLINT
    layout->addWidget(varWidget);
    auto *varLayout = new QVBoxLayout(varWidget); // NOLINT
    varLayout->setAlignment(Qt::AlignTop);
    varLayout->setContentsMargins(0, 0, 0, 0);
    varLayout->setSpacing(0);
    auto *varLabel = new QLabel(tr("Variable Monitor")); // NOLINT
    varLayout->addWidget(varLabel);
    varLayout->addWidget(m_varTreeView);

    auto *callWidget = new QWidget(); // NOLINT
    layout->addWidget(callWidget);
    auto *callLayout = new QVBoxLayout(callWidget); // NOLINT
    callLayout->setAlignment(Qt::AlignTop);
    callLayout->setContentsMargins(0, 0, 0, 0);
    callLayout->setSpacing(0);
    auto *callLabel = new QLabel(tr("Call Stack")); // NOLINT
    callLayout->addWidget(callLabel);
    callLayout->addWidget(m_callTableView);
    m_callTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_callTableView->setAlternatingRowColors(true);
    m_callTableView->setShowGrid(false);
    m_callTableView->verticalHeader()->setVisible(false);
    m_callTableView->verticalHeader()->setDefaultSectionSize(24);
    connect(m_callTableView, &QTableView::clicked, this, [this](const QModelIndex &index) {
        if (index.column() == 3) {
            const QUrl scriptUrl = index.data(Qt::UserRole + 1).toUrl();
            emit openScript(scriptUrl);
            const int line = index.data(Qt::UserRole + 2).toInt();
            emit insertMarker(scriptUrl, MARKER_HINT, line, 1000);
        }
    });
}

void DebugPage::varLoad(QStandardItemModel *varTree) const {
    m_varTreeView->setModel(varTree);
    connect(varTree, &QStandardItemModel::itemChanged, this, [this](const QStandardItem *item) {
        if (item->column() == 2) {
            const QString varScope = item->data(Qt::UserRole + 1).toString();
            const QString varName = item->data(Qt::UserRole + 2).toString();
            const QString varValue = item->text();
            QMetaObject::invokeMethod(m_interpreter, [this, varScope, varName, varValue] {
                m_interpreter->hotUpdate(varScope, varName, varValue);
            }, Qt::QueuedConnection);
        }
    });
    m_varTreeView->expandAll();
    m_varTreeView->resizeColumnToContents(0);
    m_varTreeView->resizeColumnToContents(1);
}

void DebugPage::callLoad(QStandardItemModel *callTable) const {
    m_callTableView->setModel(callTable);
    m_callTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_callTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_callTableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_callTableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
}
