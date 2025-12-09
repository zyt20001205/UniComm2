#include "scriptModule/codeDebug/debugModule.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QTableView>
#include <QThread>
#include <QTreeView>

#include "globals.h"
#include "luaModule/luaInterpreter.h"
#include "scriptModule/scriptModule.h"

// DebugModule public
DebugModule::DebugModule()
    : DockWidget("debug"),
      m_debugWidget(new QQuickWidget()),
      m_threadStringListModel(new QStringListModel()),
      m_debugBreakpointsTableModel(new QStandardItemModel()),
      m_debugBreakpointsProxyModel(new BreakpointsProxyModel()),
      m_debugBreakpointsTableView(new QTableView()),
      m_debugTabWidget(new QTabWidget()),
      m_debugTabOverlay(new QWidget(m_debugTabWidget)) {
    setWidget(m_debugWidget);
    m_debugWidget->rootContext()->setContextProperty("debugModule", this);
    m_debugWidget->rootContext()->setContextProperty("stringListModel", m_threadStringListModel);
    m_debugWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_debugWidget->setSource(QUrl("qrc:/qml/scriptModule/codeDebug/debugModule.qml"));
}

void DebugModule::debugStart(const QString &threadId) const {
    const int row = m_threadStringListModel->rowCount();
    m_threadStringListModel->insertRow(row);
    m_threadStringListModel->setData(m_threadStringListModel->index(row), threadId);
}

void DebugModule::debugStop(const QString &threadId) const {
    QStringList threads = m_threadStringListModel->stringList();
    threads.removeOne(threadId);
    m_threadStringListModel->setStringList(threads);
}

void DebugModule::debugEnd(const QString &threadId, const DebugPage *debugPage) {
    // const int index = m_debugTabWidget->indexOf(debugPage);
    // if (index != -1) m_debugTabWidget->removeTab(index);
    // delete debugPage;
    // m_interpreterHash.remove(threadId);
    // m_debugPageHash.remove(threadId);
    // if (m_debugPageHash.isEmpty()) overlayShow();
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
            emit insertMarker(scriptUrl, MARKER_HINT, line - 1, 1000);
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
