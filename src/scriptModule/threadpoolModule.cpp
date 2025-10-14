#include "scriptModule/threadpoolModule.h"

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QHeaderView>
#include <QMessageBox>
#include <QTableWidget>
#include <QThread>
#include <QTimer>

#include "globals.h"
#include "luaModule/luaInterpreter.h"

// ThreadpoolModule public
ThreadpoolModule::ThreadpoolModule()
    : DockWidget("threadpool"),
      m_runtimeTimer(new QTimer(this)),
      m_threadpoolTableWidget(new QTableWidget()),
      m_threadpoolColor{
          {THREAD_RUN, QColor(230, 255, 230)},
          {THREAD_DEBUG, QColor(255, 255, 230)},
          {THREAD_STOP, QColor(255, 230, 230)}
      } {
    m_runtimeTimer->setInterval(1000);
    m_runtimeTimer->start();
    connect(m_runtimeTimer, &QTimer::timeout, this, &ThreadpoolModule::timeRefresh);

    setWidget(m_threadpoolTableWidget);
    m_threadpoolTableWidget->setColumnCount(5);
    m_threadpoolTableWidget->setHorizontalHeaderLabels({tr("Status"), tr("Runtime"), tr("Source"), tr("Thread ID"), tr("Stop")});
    m_threadpoolTableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    m_threadpoolTableWidget->horizontalHeader()->setSectionResizeMode(STATUS_COL, QHeaderView::ResizeToContents);
    m_threadpoolTableWidget->horizontalHeader()->setSectionResizeMode(TIMER_COL, QHeaderView::ResizeToContents);
    m_threadpoolTableWidget->horizontalHeader()->setSectionResizeMode(SOURCE_COL, QHeaderView::Stretch);
    m_threadpoolTableWidget->horizontalHeader()->setSectionResizeMode(THREAD_COL, QHeaderView::Stretch);
    m_threadpoolTableWidget->horizontalHeader()->setSectionResizeMode(STOP_COL, QHeaderView::ResizeToContents);
    m_threadpoolTableWidget->verticalHeader()->setVisible(false);
    m_threadpoolTableWidget->verticalHeader()->setDefaultSectionSize(24);
    m_threadpoolTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(m_threadpoolTableWidget, &QTableWidget::cellClicked, this, [this](const int row, const int column) {
        if (column == STOP_COL) {
            if (m_threadpoolTableWidget->item(row, STATUS_COL)->text() == "stop") {
                QMessageBox::warning(this, tr("Please Wait"), tr("Stop request has already been sent."));
                return;
            }
            m_threadpoolTableWidget->item(row, STATUS_COL)->setText("stop");
            m_threadpoolTableWidget->item(row, STATUS_COL)->setBackground(m_threadpoolColor[THREAD_STOP]);
            m_threadpoolTableWidget->item(row, TIMER_COL)->setBackground(m_threadpoolColor[THREAD_STOP]);
            m_threadpoolTableWidget->item(row, SOURCE_COL)->setBackground(m_threadpoolColor[THREAD_STOP]);
            m_threadpoolTableWidget->item(row, THREAD_COL)->setBackground(m_threadpoolColor[THREAD_STOP]);
            m_threadpoolTableWidget->item(row, STOP_COL)->setBackground(m_threadpoolColor[THREAD_STOP]);

            const QString id = m_threadpoolTableWidget->item(row, THREAD_COL)->text();
            threadStop(id);
        }
    });
}

void ThreadpoolModule::workspaceOpen(const QUrl &rootUrl) {
    m_rootUrl = rootUrl;
}

QString ThreadpoolModule::threadExec(const QString &scriptPath) {
    const QString fullPath = QDir::current().filePath(m_rootUrl.toLocalFile() + "/" + scriptPath);
    QFile file(fullPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QString script = in.readAll();
    file.close();
    // call script run
    const QUrl scriptUrl = QUrl::fromLocalFile(fullPath);
    return threadRun(scriptUrl, script);
}

QString ThreadpoolModule::threadRun(const QUrl &scriptUrl, const QString &script) {
    // launch lua interpreter thread
    auto *worker = new QThread(); // NOLINT
    auto *interpreter = new LuaInterpreter(m_rootUrl, scriptUrl); // NOLINT
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

void ThreadpoolModule::threadDebug(const QUrl &scriptUrl, const QString &script) {
    // launch lua interpreter thread
    auto *worker = new QThread(); // NOLINT
    const QString threadId = QString("0x%1").arg(reinterpret_cast<quintptr>(worker), 0, 16);
    DebugData debugData{
        scriptUrl,
        threadId,
        0,
        0,
        DEBUG_RUN
    };
    auto *interpreter = new LuaInterpreter(m_rootUrl, scriptUrl); // NOLINT
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

// ThreadpoolModule private
void ThreadpoolModule::threadAppend(const int status, const QString &name, const QString &threadId, QThread *worker) {
    m_threadHash.insert(threadId, worker);
    // qDebug() << m_threadHash;
    m_threadpoolTableWidget->insertRow(0);
    auto *statusItem = new QTableWidgetItem(); // NOLINT
    if (status == 0) statusItem->setText("run");
    else statusItem->setText("debug");
    auto *runtimeItem = new QTableWidgetItem("00:00:00"); // NOLINT
    const auto time = QDateTime::currentDateTime();
    runtimeItem->setData(Qt::UserRole + 1, time);
    auto *nameItem = new QTableWidgetItem(name); // NOLINT
    auto *threadIdItem = new QTableWidgetItem(threadId); // NOLINT
    auto *stopItem = new QTableWidgetItem(QIcon(":/icon/stop.svg"), ""); // NOLINT

    statusItem->setBackground(m_threadpoolColor[status]);
    runtimeItem->setBackground(m_threadpoolColor[status]);
    nameItem->setBackground(m_threadpoolColor[status]);
    threadIdItem->setBackground(m_threadpoolColor[status]);
    stopItem->setBackground(m_threadpoolColor[status]);

    m_threadpoolTableWidget->setItem(0, STATUS_COL, statusItem);
    m_threadpoolTableWidget->setItem(0, TIMER_COL, runtimeItem);
    m_threadpoolTableWidget->setItem(0, SOURCE_COL, nameItem);
    m_threadpoolTableWidget->setItem(0, THREAD_COL, threadIdItem);
    m_threadpoolTableWidget->setItem(0, STOP_COL, stopItem);

    connect(worker, &QThread::finished, this, [this, worker] {
        const QString id = QString("0x%1").arg(reinterpret_cast<quintptr>(worker), 0, 16);
        for (int row = 0; row < m_threadpoolTableWidget->rowCount(); ++row) {
            if (m_threadpoolTableWidget->item(row, THREAD_COL)->text() == id) {
                m_threadpoolTableWidget->removeRow(row);
                break;
            }
        }
        m_threadHash.remove(id);
        emit threadStopped(id);
        // qDebug() << m_threadHash;
    });
}

void ThreadpoolModule::timeRefresh() const {
    for (int row = 0; row < m_threadpoolTableWidget->rowCount(); ++row) {
        const auto baseTime = m_threadpoolTableWidget->item(row, TIMER_COL)->data(Qt::UserRole + 1).toDateTime();
        const qint64 elapsedMs = baseTime.msecsTo(QDateTime::currentDateTime());
        QTime elapsedTime = QTime::fromMSecsSinceStartOfDay(elapsedMs);
        m_threadpoolTableWidget->item(row, TIMER_COL)->setText(elapsedTime.toString("HH:mm:ss"));
    }
}
