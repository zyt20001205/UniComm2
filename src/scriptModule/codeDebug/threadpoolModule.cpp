#include "scriptModule/codeDebug/threadpoolModule.h"

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QHeaderView>
#include <QMessageBox>
#include <QQmlContext>
#include <QQuickWidget>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QThread>
#include <QTimer>

#include "globals.h"
#include "luaModule/luaInterpreter.h"
#include "scriptModule/scriptModule.h"

// ThreadpoolModule public
ThreadpoolModule::ThreadpoolModule()
    : DockWidget("threadpool"),
      m_threadpoolWidget(new QQuickWidget()),
      m_threadpoolModel(new QStandardItemModel(this)) {
    setWidget(m_threadpoolWidget);
    const QVariantList horizontalHeader = {"", tr("Source"), tr("Spawn Time"), tr("Thread ID")};
    m_threadpoolWidget->rootContext()->setContextProperty("threadpoolModule", this);
    m_threadpoolWidget->rootContext()->setContextProperty("horizontalHeader", horizontalHeader);
    m_threadpoolWidget->rootContext()->setContextProperty("threadpoolModel", m_threadpoolModel);
    m_threadpoolWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_threadpoolWidget->setSource(QUrl("qrc:/qml/scriptModule/codeDebug/threadpoolModule.qml"));
}

void ThreadpoolModule::threadStart(const QUrl &scriptUrl, const int mode, QString &threadId) {
    const QString script = g_script->textGet(scriptUrl);
    QVariantMap luaSession{};
    luaSession.insert("mode", mode);
    luaSession.insert("workspaceUrl", g_workspaceUrl);
    luaSession.insert("scriptUrl", scriptUrl);
    if (mode == LUATHREAD_DEBUG) {
        luaSession.insert("state", DEBUG_RESUME);
        luaSession.insert("baseDepth", 0);
        luaSession.insert("currentDepth", 0);
    }
    auto *worker = new QThread(); // NOLINT
    auto *interpreter = new LuaInterpreter(luaSession); // NOLINT
    connect(interpreter, &LuaInterpreter::insertMarker, this, &ThreadpoolModule::insertMarker);
    connect(interpreter, &LuaInterpreter::removeMarker, this, &ThreadpoolModule::removeMarker);
    connect(interpreter, &LuaInterpreter::appendLog, this, &ThreadpoolModule::appendLog);
    connect(interpreter, &LuaInterpreter::startThread, this, qOverload<const QString &, const int, QString &>(&ThreadpoolModule::threadStart), Qt::BlockingQueuedConnection);
    connect(interpreter, &LuaInterpreter::stopThread, this, &ThreadpoolModule::threadStop);
    connect(interpreter, &LuaInterpreter::joinThread, this, &ThreadpoolModule::threadJoin, Qt::BlockingQueuedConnection);
    interpreter->moveToThread(worker);
    connect(worker, &QThread::finished, interpreter, &LuaInterpreter::deleteLater);
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QThread::started, [interpreter, script] {
        interpreter->start(script);
        QThread::currentThread()->quit();
    });
    worker->start();
    threadId = QString("0x%1").arg(reinterpret_cast<quintptr>(worker), 0, 16);
    threadAppend(mode, scriptUrl.fileName(), threadId, worker);
    if (mode == LUATHREAD_DEBUG) {
        emit startDebug(threadId, interpreter);
    }
}

void ThreadpoolModule::threadStart(const QString &scriptPath, const int mode, QString &threadId) {
    QString relativePath = scriptPath;
    relativePath = relativePath.replace('.', '/') + ".lua";
    const QString fullPath = QDir::current().filePath(g_workspaceUrl.toLocalFile() + "/" + relativePath);
    const auto scriptUrl = QUrl::fromLocalFile(fullPath);
    threadStart(scriptUrl, mode, threadId);
}

bool ThreadpoolModule::threadStop(const QString &threadId) {
    if (m_threadHash.contains(threadId)) {
        if (m_threadHash[threadId]->isInterruptionRequested()) {
            qDebug() << "terminate request has been sent";
            return false;
        }
        m_threadHash[threadId]->requestInterruption();
        for (int row = 0; row < m_threadpoolModel->rowCount(); ++row) {
            if (m_threadpoolModel->item(row, THREADID_COL)->data(Qt::UserRole + 1).toString() == threadId) {
                m_threadpoolModel->item(row, THREADID_COL)->setText(threadId + " (Terminating)");
                break;
            }
        }
        return true;
    }
    return false;
}

bool ThreadpoolModule::threadJoin(const QString &threadId) {
    if (m_threadHash.contains(threadId)) {
        QEventLoop loop;
        connect(this, &ThreadpoolModule::threadStopped, &loop, [&loop, threadId](const QString &id) {
            if (threadId == id) {
                loop.quit();
            }
        });
        loop.exec();
        return true;
    }
    return false;
}

QString ThreadpoolModule::lifetimeCalc(const int row) const {
    const auto item = m_threadpoolModel->item(row, SPAWN_COL);
    if (!item) return{};
    const auto baseTime = item->data(Qt::UserRole + 1).toDateTime();
    const qint64 elapsedMs = baseTime.msecsTo(QDateTime::currentDateTime());
    const QTime elapsedTime = QTime::fromMSecsSinceStartOfDay(elapsedMs);
    return "Lifetime: " + elapsedTime.toString("HH:mm:ss");
}

// ThreadpoolModule private
void ThreadpoolModule::threadAppend(const int status, const QString &name, const QString &threadId, QThread *worker) {
    m_threadHash.insert(threadId, worker);
    const auto currentTime = QDateTime::currentDateTime();
    auto *iconItem = new QStandardItem(); // NOLINT
    const QString text = status == LUATHREAD_RUN ? tr(" (Run)") : tr(" (Debug)");
    auto *nameItem = new QStandardItem(name + text); // NOLINT
    auto *spawnItem = new QStandardItem(currentTime.toString("yyyy-MM-dd HH:mm:ss.zzz")); // NOLINT
    spawnItem->setData(QVariant::fromValue(currentTime), Qt::UserRole + 1);
    auto *threadIdItem = new QStandardItem(threadId); // NOLINT
    threadIdItem->setData(threadId, Qt::UserRole + 1);
    m_threadpoolModel->appendRow({iconItem, nameItem, spawnItem, threadIdItem});

    connect(worker, &QThread::finished, this, [this, worker] {
        const QString id = QString("0x%1").arg(reinterpret_cast<quintptr>(worker), 0, 16);
        for (int row = 0; row < m_threadpoolModel->rowCount(); ++row) {
            if (m_threadpoolModel->item(row, THREADID_COL)->data(Qt::UserRole + 1).toString() == id) {
                m_threadpoolModel->removeRow(row);
                break;
            }
        }
        m_threadHash.remove(id);
        emit threadStopped(id);
        // qDebug() << m_threadHash;
    });
}
