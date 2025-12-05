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

QString ThreadpoolModule::threadExec(const QString &scriptPath, const QString &mode) {
    QString relativePath = scriptPath;
    relativePath = relativePath.replace('.', '/') + ".lua";
    const QString fullPath = QDir::current().filePath(g_workspaceUrl.toLocalFile() + "/" + relativePath);
    QFile file(fullPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QString script = in.readAll();
    file.close();
    if (mode == "run") {
        const QUrl scriptUrl = QUrl::fromLocalFile(fullPath);
        return threadRun(scriptUrl, script);
    }
    const QUrl scriptUrl = QUrl::fromLocalFile(fullPath);
    return threadDebug(scriptUrl, script);
}

QString ThreadpoolModule::threadRun(const QUrl &scriptUrl, const QString &script) {
    // launch lua interpreter thread
    auto *worker = new QThread(); // NOLINT
    auto *interpreter = new LuaInterpreter(g_workspaceUrl, scriptUrl); // NOLINT
    interpreter->moveToThread(worker);
    connect(worker, &QThread::finished, interpreter, &LuaInterpreter::deleteLater);
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QThread::started, [interpreter, script] {
        interpreter->run(script);
        QThread::currentThread()->quit();
    });
    worker->start();
    const QString threadId = QString("0x%1").arg(reinterpret_cast<quintptr>(worker), 0, 16);
    threadAppend(THREAD_RUN, scriptUrl.fileName(), threadId, worker);
    return threadId;
}

QString ThreadpoolModule::threadDebug(const QUrl &scriptUrl, const QString &script) {
    // launch lua interpreter thread
    auto *worker = new QThread(); // NOLINT
    const QString threadId = QString("0x%1").arg(reinterpret_cast<quintptr>(worker), 0, 16);
    DebugData debugData{
        scriptUrl,
        threadId,
        0,
        0,
        DEBUG_RESUME,
        {}
    };
    auto *interpreter = new LuaInterpreter(g_workspaceUrl, scriptUrl); // NOLINT
    interpreter->moveToThread(worker);
    connect(worker, &QThread::finished, interpreter, &LuaInterpreter::deleteLater);
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QThread::started, [interpreter, script, debugData] {
        interpreter->debug(script, debugData);
        QThread::currentThread()->quit();
    });
    worker->start();
    threadAppend(THREAD_DEBUG, scriptUrl.fileName(), threadId, worker);
    emit startDebug(threadId, interpreter);
    return threadId;
}

bool ThreadpoolModule::threadStop(const QString &threadId) {
    if (m_threadHash.contains(threadId)) {
        m_threadHash[threadId]->requestInterruption();
        return true;
    }
    return false;
}

bool ThreadpoolModule::threadWait(const QString &threadId) {
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
    const auto baseTime = m_threadpoolModel->item(row, SPAWN_COL)->data(Qt::UserRole + 1).toDateTime();
    const qint64 elapsedMs = baseTime.msecsTo(QDateTime::currentDateTime());
    const QTime elapsedTime = QTime::fromMSecsSinceStartOfDay(elapsedMs);
    return "Lifetime: " + elapsedTime.toString("HH:mm:ss");
}

// ThreadpoolModule private
void ThreadpoolModule::threadAppend(const int status, const QString &name, const QString &threadId, QThread *worker) {
    m_threadHash.insert(threadId, worker);
    const auto currentTime = QDateTime::currentDateTime();
    auto *iconItem = new QStandardItem(); // NOLINT
    const QString text = status == THREAD_RUN ? tr(" (Run)") : tr(" (Debug)");
    auto *nameItem = new QStandardItem(name + text); // NOLINT
    auto *spawnItem = new QStandardItem(currentTime.toString("yyyy-MM-dd HH:mm:ss.zzz")); // NOLINT
    spawnItem->setData(QVariant::fromValue(currentTime), Qt::UserRole + 1);
    auto *threadIdItem = new QStandardItem(threadId); // NOLINT
    m_threadpoolModel->appendRow({iconItem, nameItem, spawnItem, threadIdItem});

    connect(worker, &QThread::finished, this, [this, worker] {
        const QString id = QString("0x%1").arg(reinterpret_cast<quintptr>(worker), 0, 16);
        for (int row = 0; row < m_threadpoolModel->rowCount(); ++row) {
            if (m_threadpoolModel->item(row, THREADID_COL)->text() == id) {
                m_threadpoolModel->removeRow(row);
                break;
            }
        }
        m_threadHash.remove(id);
        emit threadStopped(id);
        // qDebug() << m_threadHash;
    });
}
