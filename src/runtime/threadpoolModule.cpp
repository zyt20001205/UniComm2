#include "runtime/threadpoolModule.h"

#include <QDir>
#include <QHeaderView>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QThread>
#include <QTimer>

#include "globals.h"
#include "runtime/luaInterpreter.h"
#include "document/documentModule.h"

// public
ThreadpoolModule::ThreadpoolModule()
    : DockWidget("Threadpool"),
      m_widget(new QQuickWidget()),
      m_standardItemModel(new ThreadpoolModel(this)) {
    setWidget(m_widget);
}

ThreadpoolModule::~ThreadpoolModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void ThreadpoolModule::propertySet(const QVariantHash &objects) {
    m_mainItem = qvariant_cast<QObject *>(objects["mainItem"]);
    m_errorDialog = qvariant_cast<QObject *>(objects["threadpoolModuleErrorDialog"]);
    const QVariantList horizontalHeader = {"", tr("Source"), tr("Spawn Time"), tr("Thread ID")};

    m_widget->rootContext()->setContextProperty("threadpoolModule", this);
    m_widget->rootContext()->setContextProperty("global", objects["global"]);
    m_widget->rootContext()->setContextProperty("threadMenu", qvariant_cast<QObject *>(objects["threadpoolModuleThreadMenu"]));
    m_widget->rootContext()->setContextProperty("horizontalHeader", horizontalHeader);
    m_widget->rootContext()->setContextProperty("standardItemModel", m_standardItemModel);

    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/runtime/threadpoolModule.qml"));
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

QJsonArray ThreadpoolModule::threadStart(const QUrl &documentUrl, const int mode, QString &threadId, const int startLine, const int startCharacter, const int endLine,
                                         const int endCharacter) {
    auto *worker = new QThread(); // NOLINT
    threadId = QString("0x%1").arg(reinterpret_cast<quintptr>(worker), 0, 16);
    QJsonArray buffer{};
    QEventLoop eventloop{};
    // preload thread with lua session
    QVariantMap luaSession{};
    luaSession.insert("mode", mode);
    luaSession.insert("workspaceUrl", g_workspaceUrl);
    luaSession.insert("documentUrl", documentUrl);
    luaSession.insert("threadId", threadId);
    if (mode == InterpreterMode::Debug) {
        luaSession.insert("currentUrl", documentUrl);
        luaSession.insert("state", Debug::Resume);
        luaSession.insert("baseDepth", 0);
        luaSession.insert("currentDepth", 0);
    }
    auto *interpreter = new LuaInterpreter(luaSession); // NOLINT
    connect(interpreter, &LuaInterpreter::openDocument, this, &ThreadpoolModule::openDocument);
    connect(interpreter, &LuaInterpreter::addMarker, this, &ThreadpoolModule::addMarker);
    connect(interpreter, &LuaInterpreter::deleteMarker, this, &ThreadpoolModule::deleteMarker);
    connect(interpreter, &LuaInterpreter::insertCallStack, this, &ThreadpoolModule::insertCallStack);
    connect(interpreter, &LuaInterpreter::startThread, this,
            qOverload<const QUrl &, const int, QString &, const int, const int, const int, const int>(&ThreadpoolModule::threadStart), Qt::BlockingQueuedConnection);
    connect(interpreter, &LuaInterpreter::stopThread, this, &ThreadpoolModule::threadStop);
    connect(interpreter, &LuaInterpreter::appendLog, this, &ThreadpoolModule::appendLog);
    connect(interpreter, &LuaInterpreter::newMessageDialog, this, &ThreadpoolModule::messageDialogNew);
    interpreter->moveToThread(worker);
    connect(worker, &QThread::finished, interpreter, &LuaInterpreter::deleteLater);
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    // load thread with script
    const auto script = g_document->textGet(documentUrl, startLine, startCharacter, endLine, endCharacter);
    connect(worker, &QThread::started, [interpreter, script] {
        interpreter->start(script);
        QThread::currentThread()->quit();
    });
    m_threadHash.insert(threadId, worker);
    m_interpreterHash.insert(threadId, interpreter);
    connect(worker, &QThread::finished, this, [this, threadId] { m_threadHash.remove(threadId); });
    connect(worker, &QThread::finished, this, [this, threadId] { m_interpreterHash.remove(threadId); });

    threadAppend(mode, documentUrl.fileName(), threadId);
    if (mode == InterpreterMode::Debug) {
        emit startDebug(threadId);
        connect(worker, &QThread::finished, this, [this, threadId] { emit stopDebug(threadId); });
    } else if (mode == InterpreterMode::Agent) {
        connect(interpreter, &LuaInterpreter::appendLog, this, [&buffer](const int type, const QString &prefix, const QString &message) {
            QString _type{};
            switch (type) {
                case LogLevel::Error: _type = "Error";
                    break;
                case LogLevel::Warning: _type = "Warning";
                    break;
                case LogLevel::Info: _type = "Info";
                    break;
                case LogLevel::Transmit: _type = "Transmit";
                    break;
                case LogLevel::Receive: _type = "Receive";
                    break;
            }
            const auto session = QJsonObject{
                {"type", _type},
                {"prefix", prefix},
                {"message", message}
            };
            buffer.append(session);
        });
        connect(worker, &QThread::finished, &eventloop, &QEventLoop::quit);
    }
    worker->start();
    if (mode == InterpreterMode::Agent) {
        eventloop.exec();
    }
    return buffer;
}

void ThreadpoolModule::threadStart(const QUrl &documentUrl, const int mode, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
    QString threadId{};
    threadStart(documentUrl, mode, threadId, startLine, startCharacter, endLine, endCharacter);
}

void ThreadpoolModule::threadStop(const QString &threadId) {
    if (m_threadHash.contains(threadId)) {
        if (m_threadHash[threadId]->isInterruptionRequested()) {
            QMetaObject::invokeMethod(m_errorDialog, "open");
            return;
        }
        for (int row = 0; row < m_standardItemModel->rowCount(); ++row) {
            if (m_standardItemModel->item(row, THREADID_COL)->text() == threadId) {
                const auto iconItem = m_standardItemModel->item(row, ICON_COL);
                iconItem->setData(InterpreterMode::Terminate, Qt::UserRole + 2);
                emit m_standardItemModel->dataChanged(
                    m_standardItemModel->index(row, 0),
                    m_standardItemModel->index(row, m_standardItemModel->columnCount() - 1),
                    {Qt::UserRole + 2}
                );
                if (iconItem->data(Qt::UserRole + 1).toInt() == InterpreterMode::Debug) {
                    stateSet(threadId, Debug::Terminate);
                }
                break;
            }
        }
        m_threadHash[threadId]->requestInterruption();
    }
}

bool ThreadpoolModule::debugging() const {
    return m_debug != 0;
}

void ThreadpoolModule::stateSet(const QString &threadId, const int state) {
    if (m_interpreterHash.contains(threadId)) {
        if (state == Debug::Terminate) {
            m_threadHash[threadId]->requestInterruption();
        }
        auto *interpreter = m_interpreterHash[threadId];
        QMetaObject::invokeMethod(interpreter, [interpreter, state] { interpreter->stateSet(state); }, Qt::BlockingQueuedConnection);
    }
}

void ThreadpoolModule::valueSet(const QString &threadId, const QString &documentUrl, const QString &expression, const QString &value, const QString &type) {
    if (m_interpreterHash.contains(threadId)) {
        auto *interpreter = m_interpreterHash[threadId];
        QMetaObject::invokeMethod(interpreter, [interpreter, documentUrl, expression, value, type] {
            interpreter->valueSet(documentUrl, expression, value, type);
        }, Qt::BlockingQueuedConnection);
    }
}

// private
void ThreadpoolModule::threadAppend(const int mode, const QString &name, const QString &threadId) {
    const auto currentTime = QDateTime::currentDateTime();
    auto *iconItem = new QStandardItem(); // NOLINT
    iconItem->setData(mode, Qt::UserRole + 1);
    iconItem->setData(mode, Qt::UserRole + 2);
    iconItem->setData(threadId, Qt::UserRole + 3);
    auto *nameItem = new QStandardItem(name); // NOLINT
    auto *spawnItem = new QStandardItem(currentTime.toString("yyyy-MM-dd HH:mm:ss.zzz")); // NOLINT
    auto *threadIdItem = new QStandardItem(threadId); // NOLINT
    m_standardItemModel->appendRow({iconItem, nameItem, spawnItem, threadIdItem});
    // refresh status bar
    if (mode == InterpreterMode::Run) m_run++;
    else m_debug++;
    emit refreshThread(m_run, m_debug);

    const auto *worker = m_threadHash[threadId];
    connect(worker, &QThread::finished, this, [this, threadId] {
        for (int row = 0; row < m_standardItemModel->rowCount(); ++row) {
            if (m_standardItemModel->item(row, THREADID_COL)->text() == threadId) {
                const int mode = m_standardItemModel->item(row, ICON_COL)->data(Qt::UserRole + 1).toInt();
                m_standardItemModel->removeRow(row);
                // refresh status bar
                if (mode == InterpreterMode::Run) m_run--;
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

// public
QHash<int, QByteArray> ThreadpoolModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[Qt::UserRole + 2] = "status";
    roles[Qt::UserRole + 3] = "threadId";
    return roles;
}

QVariant ThreadpoolModel::data(const QModelIndex &index, const int role) const {
    if (role == Qt::UserRole + 2 || role == Qt::UserRole + 3) {
        return QStandardItemModel::data(this->index(index.row(), 0), role);
    }
    return QStandardItemModel::data(index, role);
}
