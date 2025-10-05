#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <QDockWidget>
#include <QEventLoop>
#include <QHeaderView>
#include <QMessageBox>
#include <QTableWidget>
#include <QThread>

class Threadpool final : public QDockWidget {
    Q_OBJECT

public:
    explicit Threadpool(QWidget *parent = nullptr);

    ~Threadpool() override = default;

    QHash<QString, QThread *> threadGet();

    void threadSpawn(int status, const QString &name, const QString &threadId, QThread *worker);

    bool threadStop(const QString &threadId);

    bool threadWait(const QString &threadId);

signals:
    void threadStopped(const QString &threadId);

private:
    QTableWidget *m_threadpoolTableWidget{};
    QHash<int, QColor> m_threadpoolColor{};
    QHash<QString, QThread *> m_threadHash{};

    enum {
        THREAD_RUN,
        THREAD_DEBUG,
        THREAD_STOP
    };
};

#endif //THREADPOOL_H
