#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <QDockWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QTableWidget>
#include <QThread>

class Threadpool final : public QDockWidget {
    Q_OBJECT

public:
    explicit Threadpool(QWidget *parent = nullptr);

    ~Threadpool() override = default;

    void threadSpawn(int status, const QString &name, const QString &threadId, QThread *worker);

private:
    QTableWidget *m_threadpoolTableWidget = nullptr;
    QHash<int, QColor> m_threadpoolColor;
    QHash<QString, QThread *> m_threadHash;

    enum {
        THREAD_RUN,
        THREAD_DEBUG,
        THREAD_STOP
    };
};

#endif //THREADPOOL_H
