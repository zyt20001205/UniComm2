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
      m_threadStringListModel(new QStringListModel()){
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

void DebugModule::stateSet(const QString &threadId, const int state) {
    emit setState(threadId, state);
}

void DebugModule::callStackInsert(const QString &threadId, const QStandardItemModel *callStackModel) {
    m_callStackModelHash[threadId] = callStackModel;
    // if (!m_debugPageHash.contains(threadId)) return;
    // m_debugPageHash[threadId]->callLoad(callTable);
}
