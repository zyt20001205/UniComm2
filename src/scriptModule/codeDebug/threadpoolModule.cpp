#include "scriptModule/codeDebug/threadpoolModule.h"

#include <QDir>
#include <QFile>
#include <QHeaderView>
#include <QQmlContext>
#include <QQuickItem>
#include <QStandardItemModel>
#include <QThread>
#include <QTimer>

#include "globals.h"
#include "luaModule/luaInterpreter.h"
#include "scriptModule/scriptModule.h"

// ThreadpoolModule public
ThreadpoolModule::ThreadpoolModule()
    : DockWidget("Threadpool"),
      m_threadpoolWidget(new QQuickWidget()),
      m_threadpoolStandardItemModel(new QStandardItemModel(this)) {
    setWidget(m_threadpoolWidget);
}

ThreadpoolModule::~ThreadpoolModule() {
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] threadpool module destructed").arg(timestamp);
}

void ThreadpoolModule::propertySet(const QVariantMap &objects) {
    const auto mainObject = qvariant_cast<QObject *>(objects["mainItem"]);
    m_mainItem = qobject_cast<QQuickItem *>(mainObject);
    m_threadpoolWidget->rootContext()->setContextProperty("threadMenu", qvariant_cast<QObject *>(objects["threadpoolModuleThreadMenu"]));

    const QVariantList horizontalHeader = {"", tr("Source"), tr("Spawn Time"), tr("Thread ID")};
    m_threadpoolWidget->rootContext()->setContextProperty("threadpoolModule", this);
    m_threadpoolWidget->rootContext()->setContextProperty("horizontalHeader", horizontalHeader);
    m_threadpoolWidget->rootContext()->setContextProperty("standardItemModel", m_threadpoolStandardItemModel);
    m_threadpoolWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_threadpoolWidget->setSource(QUrl("qrc:/qml/scriptModule/codeDebug/threadpoolModule.qml"));
}

void ThreadpoolModule::quit() {
    if (const int total = static_cast<int>(m_threadHash.size())) {
        int current = 0;
        emit trackQuit(0, QString(tr("Waiting for 0/%1 threads to terminate...")).arg(total));

        QEventLoop eventLoop{};
        for (const auto &thread: m_threadHash) {
            connect(thread, &QThread::finished, this, [this, &current, &total, &eventLoop] {
                current++;
                emit trackQuit(static_cast<float>(current) / static_cast<float>(total),
                               QString(tr("Waiting for %1/%2 threads to terminate...")).arg(QString::number(current), QString::number(total)));
                if (current == total) {
                    emit trackQuit(0, "");
                    eventLoop.quit();
                }
            });
        }
        for (const auto &threadId: m_threadHash.keys()) {
            threadStop(threadId);
        }
        eventLoop.exec();
    }
}

void ThreadpoolModule::threadStart(const QUrl &scriptUrl, const int mode, QString &threadId) {
    auto *worker = new QThread(); // NOLINT
    threadId = QString("0x%1").arg(reinterpret_cast<quintptr>(worker), 0, 16);
    // preload thread with lua session
    QVariantMap luaSession{};
    luaSession.insert("mode", mode);
    luaSession.insert("workspaceUrl", g_workspaceUrl);
    luaSession.insert("scriptUrl", scriptUrl);
    luaSession.insert("threadId", threadId);
    if (mode == LUATHREAD_DEBUG) {
        luaSession.insert("currentUrl", scriptUrl);
        luaSession.insert("state", DEBUG_RESUME);
        luaSession.insert("baseDepth", 0);
        luaSession.insert("currentDepth", 0);
    }
    auto *interpreter = new LuaInterpreter(luaSession); // NOLINT
    connect(interpreter, &LuaInterpreter::openScript, this, &ThreadpoolModule::openScript);
    connect(interpreter, &LuaInterpreter::insertMarker, this, &ThreadpoolModule::insertMarker);
    connect(interpreter, &LuaInterpreter::removeMarker, this, &ThreadpoolModule::removeMarker);
    connect(interpreter, &LuaInterpreter::insertCallStack, this, &ThreadpoolModule::insertCallStack);
    connect(interpreter, &LuaInterpreter::startThread, this, qOverload<const QString &, const int, QString &>(&ThreadpoolModule::threadStart), Qt::BlockingQueuedConnection);
    connect(interpreter, &LuaInterpreter::stopThread, this, &ThreadpoolModule::threadStop);
    connect(interpreter, &LuaInterpreter::listDatabase, this, &ThreadpoolModule::listDatabase, Qt::BlockingQueuedConnection);
    connect(interpreter, &LuaInterpreter::writeDatabase, this, &ThreadpoolModule::writeDatabase, Qt::BlockingQueuedConnection);
    connect(interpreter, &LuaInterpreter::listDatatable, this, &ThreadpoolModule::listDatatable, Qt::BlockingQueuedConnection);
    connect(interpreter, &LuaInterpreter::writeDatatable, this, &ThreadpoolModule::writeDatatable, Qt::BlockingQueuedConnection);
    connect(interpreter, &LuaInterpreter::appendLog, this, &ThreadpoolModule::appendLog);
    connect(interpreter, &LuaInterpreter::newMessageDialog, this, &ThreadpoolModule::messageDialogNew);
    connect(interpreter, &LuaInterpreter::listPort, this, &ThreadpoolModule::listPort, Qt::BlockingQueuedConnection);
    interpreter->moveToThread(worker);
    connect(worker, &QThread::finished, interpreter, &LuaInterpreter::deleteLater);
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    // load thread with script
    const QString script = g_script->textGet(scriptUrl);
    connect(worker, &QThread::started, [interpreter, script] {
        interpreter->start(script);
        QThread::currentThread()->quit();
    });
    // start thread
    worker->start();
    m_threadHash.insert(threadId, worker);
    m_interpreterHash.insert(threadId, interpreter);
    connect(worker, &QThread::finished, this, [this, threadId] { m_threadHash.remove(threadId); });
    connect(worker, &QThread::finished, this, [this, threadId] { m_interpreterHash.remove(threadId); });
    threadAppend(mode, scriptUrl.fileName(), threadId);
    if (mode == LUATHREAD_DEBUG) {
        emit startDebug(threadId);
        connect(worker, &QThread::finished, this, [this, threadId] { emit stopDebug(threadId); });
    }
}

void ThreadpoolModule::threadStart(const QUrl &scriptUrl, const int mode) {
    QString threadId{};
    threadStart(scriptUrl, mode, threadId);
}

void ThreadpoolModule::threadStart(const QString &scriptPath, const int mode, QString &threadId) {
    const auto scriptUrl = QUrl::fromLocalFile(scriptPath);
    threadStart(scriptUrl, mode, threadId);
}

void ThreadpoolModule::threadStop(const QString &threadId) {
    if (m_threadHash.contains(threadId)) {
        if (m_threadHash[threadId]->isInterruptionRequested()) {
            qDebug() << "terminate request has been sent";
            return;
        }
        m_threadHash[threadId]->requestInterruption();
        for (int row = 0; row < m_threadpoolStandardItemModel->rowCount(); ++row) {
            if (m_threadpoolStandardItemModel->item(row, THREADID_COL)->data(Qt::UserRole + 1).toString() == threadId) {
                m_threadpoolStandardItemModel->item(row, THREADID_COL)->setText(threadId + " (Terminating)");
                break;
            }
        }
    }
}

bool ThreadpoolModule::debugging() const {
    return m_debug != 0;
}

QString ThreadpoolModule::lifetimeCalc(const int row) const {
    const auto item = m_threadpoolStandardItemModel->item(row, SPAWN_COL);
    if (!item) return {};
    const auto baseTime = item->data(Qt::UserRole + 1).toDateTime();
    const qint64 elapsedMs = baseTime.msecsTo(QDateTime::currentDateTime());
    const QTime elapsedTime = QTime::fromMSecsSinceStartOfDay(elapsedMs);
    return "Lifetime: " + elapsedTime.toString("HH:mm:ss");
}

void ThreadpoolModule::stateSet(const QString &threadId, const int state) {
    if (m_interpreterHash.contains(threadId)) {
        if (state == DEBUG_TERMINATE) {
            m_threadHash[threadId]->requestInterruption();
        }
        auto *interpreter = m_interpreterHash[threadId];
        QMetaObject::invokeMethod(interpreter, [interpreter, state] { interpreter->stateSet(state); }, Qt::BlockingQueuedConnection);
    }
}

void ThreadpoolModule::valueSet(const QString &threadId, const QString &scriptUrl, const QString &expression, const QString &value) {
    if (m_interpreterHash.contains(threadId)) {
        auto *interpreter = m_interpreterHash[threadId];
        QMetaObject::invokeMethod(interpreter, [interpreter, scriptUrl, expression, value] {
            interpreter->valueSet(scriptUrl, expression, value);
        }, Qt::BlockingQueuedConnection);
    }
}

// ThreadpoolModule private
void ThreadpoolModule::threadAppend(const int mode, const QString &name, const QString &threadId) {
    const auto currentTime = QDateTime::currentDateTime();
    auto *iconItem = new QStandardItem(); // NOLINT
    const QString text = mode == LUATHREAD_RUN ? tr(" (Run)") : tr(" (Debug)");
    auto *nameItem = new QStandardItem(name + text); // NOLINT
    auto *spawnItem = new QStandardItem(currentTime.toString("yyyy-MM-dd HH:mm:ss.zzz")); // NOLINT
    spawnItem->setData(QVariant::fromValue(currentTime), Qt::UserRole + 1);
    auto *threadIdItem = new QStandardItem(threadId); // NOLINT
    threadIdItem->setData(mode, Qt::UserRole + 1);
    m_threadpoolStandardItemModel->appendRow({iconItem, nameItem, spawnItem, threadIdItem});
    // refresh status bar
    if (mode == LUATHREAD_RUN) m_run++;
    else m_debug++;
    emit refreshThread(m_run, m_debug);

    const auto *worker = m_threadHash[threadId];
    connect(worker, &QThread::finished, this, [this, threadId] {
        for (int row = 0; row < m_threadpoolStandardItemModel->rowCount(); ++row) {
            if (m_threadpoolStandardItemModel->item(row, THREADID_COL)->text() == threadId) {
                const int mode = m_threadpoolStandardItemModel->item(row, THREADID_COL)->data(Qt::UserRole + 1).toInt();
                m_threadpoolStandardItemModel->removeRow(row);
                // refresh status bar
                if (mode == LUATHREAD_RUN) m_run--;
                else m_debug--;
                emit refreshThread(m_run, m_debug);
                break;
            }
        }
    });
}

void ThreadpoolModule::messageDialogNew(const QEventLoop *eventloop, const QString &threadId, const QString &text) const {
    QMetaObject::invokeMethod(m_mainItem, "messageDialogNew", Q_ARG(QVariant, QVariant::fromValue(eventloop)), Q_ARG(QVariant, threadId), Q_ARG(QVariant, text));
}
