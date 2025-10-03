#include "../include/threadpool.h"

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
            m_threadpoolTableWidget->item(row, 0)->setText("stop");
            m_threadpoolTableWidget->item(row, 0)->setBackground(m_threadpoolColor[THREAD_STOP]);
            m_threadpoolTableWidget->item(row, 1)->setBackground(m_threadpoolColor[THREAD_STOP]);
            m_threadpoolTableWidget->item(row, 2)->setBackground(m_threadpoolColor[THREAD_STOP]);
            m_threadpoolTableWidget->item(row, 3)->setBackground(m_threadpoolColor[THREAD_STOP]);

            const QString id = m_threadpoolTableWidget->item(row, 2)->text();
            m_threadHash[id]->requestInterruption();
            m_threadHash.remove(id);
            // qDebug() << m_threadHash;
        }
    });
}

void Threadpool::threadSpawn(const int status, const QString &name, const QString &threadId, QThread *worker) {
    m_threadHash.insert(threadId, worker);
    // qDebug() << m_threadHash;
    m_threadpoolTableWidget->insertRow(0);
    auto *statusItem = new QTableWidgetItem(); // NOLINT
    if (status == 0) statusItem->setText("run");
    else statusItem->setText("debug");
    auto *nameItem = new QTableWidgetItem(name); // NOLINT
    auto *threadIdItem = new QTableWidgetItem(threadId); // NOLINT
    auto *stopItem = new QTableWidgetItem(); // NOLINT
    stopItem->setIcon(QIcon(":/icon/stop.svg"));

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
        // qDebug() << m_threadHash;
    });
}
