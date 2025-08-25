#include "../include/database.h"

// Database public
Database::Database(QObject *parent)
    : QDockWidget("database", qobject_cast<QWidget *>(parent)) {
    m_tableWidget = new QTableWidget(); // NOLINT
    setWidget(m_tableWidget);
    m_tableWidget->setColumnCount(2);
    m_tableWidget->setHorizontalHeaderLabels({"key", "value"});
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableWidget->setDragEnabled(true);
    m_tableWidget->setAcceptDrops(true);
    m_tableWidget->setDragDropMode(QAbstractItemView::InternalMove);
    m_tableWidget->setDefaultDropAction(Qt::MoveAction);

    for (const QJsonValue &key: m_databaseConfig) {
        const int index = m_tableWidget->rowCount();
        m_tableWidget->insertRow(index);
        m_tableWidget->setItem(index, 0, new QTableWidgetItem(key.toString()));
        m_tableWidget->setItem(index, 1, new QTableWidgetItem(""));
    }

    m_tableWidget->installEventFilter(this);
    m_tableWidget->viewport()->installEventFilter(this);
    connect(m_tableWidget, &QTableWidget::cellChanged, this, [this](const int row, const int column) {
        if (column != 0) return;
        databaseRename(row);
    });
}

void Database::databaseConfigSave() const {
    g_config["databaseConfig"] = m_databaseConfig;
}

void Database::databaseWrite(const QString &key, const QString &value) {
    for (int index = 0; index < m_tableWidget->rowCount(); ++index) {
        if (m_tableWidget->item(index, 0)->text() == key) {
            m_tableWidget->item(index, 1)->text() = value;
            return;
        }
    }
    emit appendLog("key not found", "error");
}

// Database protected
void Database::contextMenuEvent(QContextMenuEvent *event) {
    const QPoint vpPos = m_tableWidget->viewport()->mapFromGlobal(event->globalPos());
    const QModelIndex index = m_tableWidget->indexAt(vpPos);
    QMenu menu(this);
    if (m_databaseConfig.isEmpty()) {
        menu.addAction(tr("new"), [this] {
            databaseInsert(0);
        });
        menu.exec(event->globalPos());
    } else if (!index.isValid()) {
        return;
    } else {
        menu.addAction(tr("insert above (Ins)"), [this, index] {
            databaseInsert(index.row());
        });
        menu.addAction(tr("insert below (Ctrl+Ins)"), [this, index] {
            databaseInsert(index.row() + 1);
        });
        menu.addAction(tr("delete (Del)"), [this, index] {
            databaseRemove(index.row());
        });
        menu.exec(event->globalPos());
    }
}

bool Database::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_tableWidget->viewport()) {
        switch (event->type()) {
            case QEvent::DragMove: {
                m_tableWidget->clearSelection();
                m_currentIndex = m_tableWidget->indexAt(static_cast<QDragMoveEvent *>(event)->position().toPoint()).row();
                if (m_currentIndex == -1) break;
                if (m_previousIndex == -1) {
                    m_sourceKey = m_tableWidget->item(m_currentIndex, 0)->text();
                    m_sourceIndex = m_currentIndex;
                    m_previousIndex = m_currentIndex;
                }
                if (m_previousIndex != m_currentIndex) {
                    m_tableWidget->removeRow(m_previousIndex);
                    m_tableWidget->insertRow(m_currentIndex);
                    m_tableWidget->setItem(m_currentIndex, 0, new QTableWidgetItem(m_sourceKey));
                    m_tableWidget->setItem(m_currentIndex, 1, new QTableWidgetItem(""));
                    m_previousIndex = m_currentIndex;
                }
                break;
            }
            case QEvent::Drop: {
                m_tableWidget->clearSelection();
                m_tableWidget->setCurrentItem(nullptr);
                m_tableWidget->clearFocus();
                m_previousIndex = -1;
                databaseRename(m_sourceIndex);
                return true;
            }
            default:
                break;
        }
    } else if (obj == m_tableWidget && event->type() == QEvent::KeyPress) {
        switch (static_cast<QKeyEvent *>(event)->key()) {
            case Qt::Key_Insert: {
                if (const auto keyEvent = static_cast<QKeyEvent *>(event); keyEvent->modifiers() & Qt::ControlModifier) {
                    databaseInsert(m_tableWidget->currentRow() + 1);
                } else {
                    databaseInsert(m_tableWidget->currentRow());
                }
                return true;
            }
            case Qt::Key_Delete: {
                databaseRemove(m_tableWidget->currentRow());
                return true;
            }
            case Qt::Key_Escape: {
                m_tableWidget->clearSelection();
                m_tableWidget->setCurrentItem(nullptr);
                m_tableWidget->clearFocus();
                return true;
            }
            default:
                break;
        }
    }
    return QDockWidget::eventFilter(obj, event);
}

// Database private
void Database::databaseRename(const int index) {
    m_databaseConfig[index] = m_tableWidget->item(index, 0)->text();
}

void Database::databaseInsert(const int index) {
    m_databaseConfig.insert(index, "");
    m_tableWidget->insertRow(index);
    m_tableWidget->setItem(index, 0, new QTableWidgetItem(""));
    m_tableWidget->setItem(index, 1, new QTableWidgetItem(""));
}

void Database::databaseRemove(const int index) {
    m_tableWidget->removeRow(index);
    m_databaseConfig.removeAt(index);
}
