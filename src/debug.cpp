#include "../include/debug.h"

#include <QLabel>

#include "../include/luaInterpreter.h"

// Debug public
Debug::Debug(QWidget *parent)
    : QDockWidget("debug", parent),
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
            auto *debugContinueButton = new QPushButton(); // NOLINT
            debugCtrlLayout->addWidget(debugContinueButton);
            debugContinueButton->setFixedSize(24, 24);
            debugContinueButton->setIcon(QIcon(":/icon/debugContinue.svg"));
            debugContinueButton->setToolTip(tr("resume"));
            connect(debugContinueButton, &QPushButton::clicked, this, [this] {
                if (m_debugPageHash.isEmpty()) {
                    QMessageBox::critical(this, tr("Error"), tr("No active debug session."));
                    return;
                }
                const int index = m_debugTabWidget->currentIndex();
                const QString threadId = m_debugTabWidget->tabText(index);
                m_interpreterHash[threadId]->debugStateSet(DEBUG_RUN);
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
                m_interpreterHash[threadId]->debugStateSet(DEBUG_PAUSE);
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
                m_interpreterHash[threadId]->debugStateSet(DEBUG_STEPOVER);
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
                m_interpreterHash[threadId]->debugStateSet(DEBUG_STEPINTO);
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
                m_interpreterHash[threadId]->debugStateSet(DEBUG_STEPOUT);
                emit resume(threadId);
            });
            auto *debugTerminateButton = new QPushButton(); // NOLINT
            debugCtrlLayout->addWidget(debugTerminateButton);
            debugTerminateButton->setFixedSize(24, 24);
            debugTerminateButton->setIcon(QIcon(":/icon/stop.svg"));
            debugTerminateButton->setToolTip(tr("terminate"));
            connect(debugTerminateButton, &QPushButton::clicked, this, [this] {
                if (m_debugPageHash.isEmpty()) {
                    QMessageBox::critical(this, tr("Error"), tr("No active debug session."));
                    return;
                }
                const int index = m_debugTabWidget->currentIndex();
                const QString threadId = m_debugTabWidget->tabText(index);
                const LuaInterpreter *interpreter = m_interpreterHash[threadId];
                interpreter->thread()->requestInterruption();
            });
        }
        // debug breakpoints
        {
            auto *debugBreakpointsLabel = new QLabel(tr("Breakpoints")); // NOLINT
            debugMasterCtrlLayout->addWidget(debugBreakpointsLabel);
            debugMasterCtrlLayout->addWidget(m_debugBreakpointsTableView);
            m_debugBreakpointsTableModel->setColumnCount(3);
            m_debugBreakpointsTableModel->setHorizontalHeaderLabels({tr("Script"), tr("Line"), tr("View")});
            m_debugBreakpointsProxyModel->setSourceModel(m_debugBreakpointsTableModel);
            m_debugBreakpointsTableView->setModel(m_debugBreakpointsProxyModel);
            m_debugBreakpointsTableView->sortByColumn(0, Qt::AscendingOrder);
            m_debugBreakpointsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
            m_debugBreakpointsTableView->setAlternatingRowColors(true);
            m_debugBreakpointsTableView->setShowGrid(false);
            m_debugBreakpointsTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
            m_debugBreakpointsTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
            m_debugBreakpointsTableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
            m_debugBreakpointsTableView->verticalHeader()->setVisible(false);
            m_debugBreakpointsTableView->verticalHeader()->setDefaultSectionSize(24);
            connect(m_debugBreakpointsTableView, &QTableView::clicked, this, [this](const QModelIndex &index) {
                if (index.column() == 2) {
                    const QUrl scriptUrl = index.data(Qt::UserRole + 1).toUrl();
                    emit openScript(scriptUrl);
                    const int line = index.data(Qt::UserRole + 2).toInt();
                    emit highlightMarker(scriptUrl, line, 1000);
                }
            });
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
    QTimer::singleShot(0, this, [this] { overlayResize(); });
}

void Debug::breakpointInsert(const QUrl &scriptUrl, const int line) const {
    const int row = m_debugBreakpointsTableModel->rowCount();
    m_debugBreakpointsTableModel->insertRow(row);
    m_debugBreakpointsTableModel->setItem(row, 0, new QStandardItem(scriptUrl.fileName()));
    m_debugBreakpointsTableModel->setItem(row, 1, new QStandardItem(QString::number(line)));
    auto *viewItem = new QStandardItem(QIcon(":/icon/arrowRight.svg"), ""); // NOLINT
    viewItem->setData(QVariant(scriptUrl), Qt::UserRole + 1);
    viewItem->setData(QVariant(line), Qt::UserRole + 2);
    m_debugBreakpointsTableModel->setItem(row, 2, viewItem);
}

void Debug::breakpointRemove(const QUrl &scriptUrl, const int line) const {
    for (int row = 0; row < m_debugBreakpointsTableModel->rowCount(); ++row) {
        const QStandardItem *fileItem = m_debugBreakpointsTableModel->item(row, 0);
        const QStandardItem *lineItem = m_debugBreakpointsTableModel->item(row, 1);
        if (fileItem->text() == scriptUrl.fileName() && lineItem->text() == QString::number(line)) {
            m_debugBreakpointsTableModel->removeRow(row);
            break;
        }
    }
}

void Debug::debugStart(const QString &threadId, LuaInterpreter *interpreter) {
    m_interpreterHash.insert(threadId, interpreter);
    auto *debugPage = new DebugPage(interpreter); // NOLINT
    m_debugPageHash[threadId] = debugPage;
    m_debugTabWidget->addTab(debugPage, threadId);
    connect(interpreter->thread(), &QThread::finished, this, [this, threadId, debugPage] { debugEnd(threadId, debugPage); });
    connect(debugPage, &DebugPage::openScript, this, &Debug::openScript);
    connect(debugPage, &DebugPage::highlightMarker, this, &Debug::highlightMarker);
    overlayHide();
}

void Debug::debugEnd(const QString &threadId, const DebugPage *debugPage) {
    const int index = m_debugTabWidget->indexOf(debugPage);
    if (index != -1) m_debugTabWidget->removeTab(index);
    delete debugPage;
    m_interpreterHash.remove(threadId);
    m_debugPageHash.remove(threadId);
    if (m_debugPageHash.isEmpty()) overlayShow();
}

void Debug::varReturn(const QString &threadId, QStandardItemModel *varTree) {
    if (!m_debugPageHash.contains(threadId)) return;
    m_debugPageHash[threadId]->varLoad(varTree);
}

void Debug::callReturn(const QString &threadId, QStandardItemModel *callTable) {
    if (!m_debugPageHash.contains(threadId)) return;
    m_debugPageHash[threadId]->callLoad(callTable);
}

// Debug protected
void Debug::resizeEvent(QResizeEvent *event) {
    QDockWidget::resizeEvent(event);
    if (m_debugTabOverlay->isVisible()) overlayResize();
}

// Debug private
void Debug::overlayShow() const {
    m_debugTabOverlay->raise();
    m_debugTabOverlay->show();
}

void Debug::overlayHide() const {
    m_debugTabOverlay->hide();
}

void Debug::overlayResize() const {
    m_debugTabOverlay->setGeometry(m_debugTabWidget->rect());
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
            emit highlightMarker(scriptUrl, line, 1000);
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
