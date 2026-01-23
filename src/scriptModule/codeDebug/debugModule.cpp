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
    : DockWidget("Debug"),
      m_debugWidget(new QQuickWidget()),
      m_threadStringListModel(new QStringListModel()) {
    setWidget(m_debugWidget);
}

DebugModule::~DebugModule() {
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] debug module destructed").arg(timestamp);
}

void DebugModule::propertySet(const QVariantMap &objects) {
    m_errorDialog = qvariant_cast<QObject *>(objects["debugModuleErrorDialog"]);

    m_debugWidget->rootContext()->setContextProperty("debugModule", this);
    m_debugWidget->rootContext()->setContextProperty("stringListModel", m_threadStringListModel);
    m_debugWidget->rootContext()->setContextProperty("standardItemModel", new QStandardItemModel());
    m_debugWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_debugWidget->setSource(QUrl("qrc:/qml/scriptModule/codeDebug/debugModule.qml"));
}

void DebugModule::propertyGet(const QVariantMap &objects) {
    m_threadComboBox = qvariant_cast<QObject *>(objects["threadComboBox"]);
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

QString DebugModule::threadGet() const {
    return m_threadComboBox->property("currentText").toString();
}

void DebugModule::stateSet(const QString &threadId, const int state) {
    const auto &currenThread = m_threadComboBox->property("currentText").toString();
    if (currenThread.isEmpty()) {
        QMetaObject::invokeMethod(m_errorDialog, "open");
        return;
    }
    if (state == DEBUG_RUNTOCURSOR) {
        emit getCursorPosition();
        const QString &currentThreadId = currenThread;
        emit setState(currentThreadId, state);
    } else {
        emit setState(threadId, state);
    }
}

bool DebugModule::valueSet(const QString &threadId, const QString &scriptUrl, const QString &expression, const QString &value, const QString &type) {
    qDebug() << threadId << scriptUrl << expression << value << type;
    return true;
}

void DebugModule::callStackInsert(const QString &threadId, QStandardItemModel *callStackModel) {
    if (!m_callStackModelHash.contains(threadId)) {
        m_callStackModelHash.insert(threadId, callStackModel);
    } else {
        m_callStackModelHash[threadId] = callStackModel;
        m_debugWidget->rootContext()->setContextProperty("standardItemModel", callStackModel);
    }
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
