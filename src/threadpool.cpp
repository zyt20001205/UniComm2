#include "../include/threadpool.h"

#include "../include/globals.h"
#include "../include/luaInterpreter.h"

// Threadpool public
Threadpool::Threadpool(QWidget *parent)
    : QDockWidget("threadpool", parent),
      m_threadpoolTableWidget(new QTableWidget()),
      m_threadpoolColor{
          {THREAD_RUN, QColor(230, 255, 230)},
          {THREAD_DEBUG, QColor(255, 255, 230)},
          {THREAD_STOP, QColor(255, 230, 230)}
      } {
    setWidget(m_threadpoolTableWidget);
    m_threadpoolTableWidget->setColumnCount(4);
    m_threadpoolTableWidget->setHorizontalHeaderLabels({"status", "source", "thread id", "stop"});
    m_threadpoolTableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    m_threadpoolTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_threadpoolTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_threadpoolTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_threadpoolTableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_threadpoolTableWidget->verticalHeader()->setVisible(false);
    m_threadpoolTableWidget->verticalHeader()->setDefaultSectionSize(24);
    m_threadpoolTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(m_threadpoolTableWidget, &QTableWidget::cellClicked, this, [this](const int row, const int column) {
        if (column == 3) {
            if (m_threadpoolTableWidget->item(row, 0)->text() == "stop") {
                QMessageBox::warning(this, tr("Please Wait"), tr("Stop request has already been sent."));
                return;
            }
            m_threadpoolTableWidget->item(row, 0)->setText("stop");
            m_threadpoolTableWidget->item(row, 0)->setBackground(m_threadpoolColor[THREAD_STOP]);
            m_threadpoolTableWidget->item(row, 1)->setBackground(m_threadpoolColor[THREAD_STOP]);
            m_threadpoolTableWidget->item(row, 2)->setBackground(m_threadpoolColor[THREAD_STOP]);
            m_threadpoolTableWidget->item(row, 3)->setBackground(m_threadpoolColor[THREAD_STOP]);

            const QString id = m_threadpoolTableWidget->item(row, 2)->text();
            threadStop(id);
        }
    });

    // open workspace
    if (const QUrl rootUrl(g_config["mainConfig"].toObject()["workspace"].toString()); !rootUrl.isEmpty()) {
        workspaceOpen(rootUrl);
    }
}

void Threadpool::workspaceOpen(const QUrl &rootUrl) {
    m_rootUrl = rootUrl;
}

QString Threadpool::threadExec(const QString &scriptPath) {
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

QString Threadpool::threadRun(const QUrl &scriptUrl, const QString &script) {
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

void Threadpool::threadDebug(const QUrl &scriptUrl, const QString &script) {
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

bool Threadpool::threadStop(const QString &threadId) {
    if (m_threadHash.contains(threadId)) {
        m_threadHash[threadId]->requestInterruption();
        return true;
    }
    return false;
}

bool Threadpool::threadWait(const QString &threadId) {
    if (m_threadHash.contains(threadId)) {
        QEventLoop loop;
        connect(this, &Threadpool::threadStopped, &loop, [&loop, threadId](const QString &id) {
            if (threadId == id) {
                loop.quit();
            }
        });
        loop.exec();
        return true;
    }
    return false;
}

// Threadpool private
void Threadpool::threadAppend(const int status, const QString &name, const QString &threadId, QThread *worker) {
    m_threadHash.insert(threadId, worker);
    // qDebug() << m_threadHash;
    m_threadpoolTableWidget->insertRow(0);
    auto *statusItem = new QTableWidgetItem(); // NOLINT
    if (status == 0) statusItem->setText("run");
    else statusItem->setText("debug");
    auto *nameItem = new QTableWidgetItem(name); // NOLINT
    auto *threadIdItem = new QTableWidgetItem(threadId); // NOLINT
    auto *stopItem = new QTableWidgetItem(QIcon(":/icon/stop.svg"), ""); // NOLINT

    statusItem->setBackground(m_threadpoolColor[status]);
    nameItem->setBackground(m_threadpoolColor[status]);
    threadIdItem->setBackground(m_threadpoolColor[status]);
    stopItem->setBackground(m_threadpoolColor[status]);

    m_threadpoolTableWidget->setItem(0, 0, statusItem);
    m_threadpoolTableWidget->setItem(0, 1, nameItem);
    m_threadpoolTableWidget->setItem(0, 2, threadIdItem);
    m_threadpoolTableWidget->setItem(0, 3, stopItem);

    connect(worker, &QThread::finished, this, [this, worker] {
        const QString id = QString("0x%1").arg(reinterpret_cast<quintptr>(worker), 0, 16);
        for (int row = 0; row < m_threadpoolTableWidget->rowCount(); ++row) {
            if (m_threadpoolTableWidget->item(row, 2)->text() == id) {
                m_threadpoolTableWidget->removeRow(row);
                break;
            }
        }
        m_threadHash.remove(id);
        emit threadStopped(id);
        // qDebug() << m_threadHash;
    });
}