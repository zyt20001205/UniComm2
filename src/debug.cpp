#include "../include/debug.h"

#include <QLabel>

#include "../include/luaInterpreter.h"

// Debug public
Debug::Debug(QWidget *parent)
    : QDockWidget("debug", parent),
      m_debugThreadCombobox(new QComboBox()),
      m_debugBreakpointsTableModel(new QStandardItemModel()),
      m_debugBreakpointsProxyModel(new BreakpointsProxyModel()),
      m_debugBreakpointsTableView(new QTableView()) {
    auto *widget = new QWidget(); // NOLINT
    setWidget(widget);
    auto *layout = new QVBoxLayout(widget); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);

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
                if (m_debugThreadCombobox->count() == 0) {
                    QMessageBox::critical(this, tr("Error"), tr("No active debug threads available."));
                    return;
                }
                const QString threadId = m_debugThreadCombobox->currentText();
                m_interpreterHash[threadId]->debugStateSet(DEBUG_RUN);
                emit resume(threadId);
            });
            auto *debugPauseButton = new QPushButton(); // NOLINT
            debugCtrlLayout->addWidget(debugPauseButton);
            debugPauseButton->setFixedSize(24, 24);
            debugPauseButton->setIcon(QIcon(":/icon/pause.svg"));
            debugPauseButton->setToolTip(tr("pause"));
            connect(debugPauseButton, &QPushButton::clicked, this, [this] {
                if (m_debugThreadCombobox->count() == 0) {
                    QMessageBox::critical(this, tr("Error"), tr("No active debug threads available."));
                    return;
                }
                const QString threadId = m_debugThreadCombobox->currentText();
                m_interpreterHash[threadId]->debugStateSet(DEBUG_PAUSE);
                emit resume(threadId);
            });
            auto *debugStepOverButton = new QPushButton(); // NOLINT
            debugCtrlLayout->addWidget(debugStepOverButton);
            debugStepOverButton->setFixedSize(24, 24);
            debugStepOverButton->setIcon(QIcon(":/icon/debugStepOver.svg"));
            debugStepOverButton->setToolTip(tr("step over"));
            connect(debugStepOverButton, &QPushButton::clicked, this, [this] {
                if (m_debugThreadCombobox->count() == 0) {
                    QMessageBox::critical(this, tr("Error"), tr("No active debug threads available."));
                    return;
                }
                const QString threadId = m_debugThreadCombobox->currentText();
                m_interpreterHash[threadId]->debugStateSet(DEBUG_STEPOVER);
                emit resume(threadId);
            });
            auto *debugStepIntoButton = new QPushButton(); // NOLINT
            debugCtrlLayout->addWidget(debugStepIntoButton);
            debugStepIntoButton->setFixedSize(24, 24);
            debugStepIntoButton->setIcon(QIcon(":/icon/debugStepInto.svg"));
            debugStepIntoButton->setToolTip(tr("step into"));
            connect(debugStepIntoButton, &QPushButton::clicked, this, [this] {
                if (m_debugThreadCombobox->count() == 0) {
                    QMessageBox::critical(this, tr("Error"), tr("No active debug threads available."));
                    return;
                }
                const QString threadId = m_debugThreadCombobox->currentText();
                m_interpreterHash[threadId]->debugStateSet(DEBUG_STEPINTO);
                emit resume(threadId);
            });
            auto *debugStepOutButton = new QPushButton(); // NOLINT
            debugCtrlLayout->addWidget(debugStepOutButton);
            debugStepOutButton->setFixedSize(24, 24);
            debugStepOutButton->setIcon(QIcon(":/icon/debugStepOut.svg"));
            debugStepOutButton->setToolTip(tr("step out"));
            connect(debugStepOutButton, &QPushButton::clicked, this, [this] {
                if (m_debugThreadCombobox->count() == 0) {
                    QMessageBox::critical(this, tr("Error"), tr("No active debug threads available."));
                    return;
                }
                const QString threadId = m_debugThreadCombobox->currentText();
                m_interpreterHash[threadId]->debugStateSet(DEBUG_STEPOUT);
                emit resume(threadId);
            });
            auto *debugTerminateButton = new QPushButton(); // NOLINT
            debugCtrlLayout->addWidget(debugTerminateButton);
            debugTerminateButton->setFixedSize(24, 24);
            debugTerminateButton->setIcon(QIcon(":/icon/stop.svg"));
            debugTerminateButton->setToolTip(tr("terminate"));
            connect(debugTerminateButton, &QPushButton::clicked, this, [this] {
                if (m_debugThreadCombobox->count() == 0) {
                    QMessageBox::critical(this, tr("Error"), tr("No active debug threads available."));
                    return;
                }
                const QString threadId = m_debugThreadCombobox->currentText();
                const LuaInterpreter *interpreter = m_interpreterHash[threadId];
                interpreter->thread()->requestInterruption();
            });
        }
        // debug thread
        {
            auto *debugThreadLabel = new QLabel(tr("Thread Control")); // NOLINT
            debugMasterCtrlLayout->addWidget(debugThreadLabel);
            debugMasterCtrlLayout->addWidget(m_debugThreadCombobox);
        }
        // debug breakpoints
        {
            auto *debugBreakpointsLabel = new QLabel(tr("Breakpoints")); // NOLINT
            debugMasterCtrlLayout->addWidget(debugBreakpointsLabel);
            debugMasterCtrlLayout->addWidget(m_debugBreakpointsTableView);
            m_debugBreakpointsTableModel->setColumnCount(3);
            m_debugBreakpointsTableModel->setHeaderData(0, Qt::Horizontal, tr("Script"));
            m_debugBreakpointsTableModel->setHeaderData(1, Qt::Horizontal, tr("Line"));
            m_debugBreakpointsTableModel->setHeaderData(2, Qt::Horizontal, "");
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

    // debug variable treeview
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
    m_debugThreadCombobox->addItem(threadId);
    m_interpreterHash.insert(threadId, interpreter);
    connect(interpreter->thread(), &QThread::finished, this, [this, threadId] { debugEnd(threadId); });
}

void Debug::debugEnd(const QString &threadId) {
    const int index = m_debugThreadCombobox->findText(threadId);
    if (index != -1) m_debugThreadCombobox->removeItem(index);
    m_interpreterHash.remove(threadId);
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
