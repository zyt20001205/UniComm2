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
      m_threadStringListModel(new QStringListModel()) {
    setWidget(m_debugWidget);
    m_debugWidget->rootContext()->setContextProperty("debugModule", this);
    m_debugWidget->rootContext()->setContextProperty("stringListModel", m_threadStringListModel);
    m_debugWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_debugWidget->setSource(QUrl("qrc:/qml/scriptModule/codeDebug/debugModule.qml"));
}

void DebugModule::debugStart(const QString &threadId) const {
    QStringList threads = m_threadStringListModel->stringList();
    threads.append(threadId);
    m_threadStringListModel->setStringList(threads);
}

void DebugModule::debugStop(const QString &threadId) {
    // string list model
    QStringList threads = m_threadStringListModel->stringList();
    threads.removeOne(threadId);
    m_threadStringListModel->setStringList(threads);
    // standard item model
    m_callStackModelHash.remove(threadId);
}

void DebugModule::stateSet(const QString &threadId, const int state) {
    emit setState(threadId, state);
}

void DebugModule::callStackInsert(const QString &threadId, QStandardItemModel *callStackModel) {
    m_callStackModelHash[threadId] = callStackModel;
}

void DebugModule::callStackSwitch(const QString &threadId) const {
    if (threadId.isEmpty()) {
        m_debugWidget->rootContext()->setContextProperty("standardItemModel", nullptr);
        return;
    }
    auto *callStackModel = m_callStackModelHash.value(threadId, nullptr);
    if (!callStackModel) {
        m_debugWidget->rootContext()->setContextProperty("standardItemModel", nullptr);
        return;
    }
    m_debugWidget->rootContext()->setContextProperty("standardItemModel", callStackModel);
}

void DebugModule::markerInsert(const QVariantHash &position) {
    emit openScript(position["scriptUrl"].toUrl());
    emit insertMarker(
        position["scriptUrl"].toUrl(),
        MARKER_HINT,
        position["line"].toInt() - 1,
        1000);
}
