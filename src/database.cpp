#include "../include/database.h"

// Database public
Database::Database(QObject *parent)
    : QDockWidget("database", qobject_cast<QWidget *>(parent)) {
    m_tableWidget = new QTableWidget(); // NOLINT
    setWidget(m_tableWidget);
    m_tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_tableWidget->setColumnCount(1);
    m_tableWidget->horizontalHeader()->setVisible(false);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tableWidget->verticalHeader()->setMinimumWidth(30);
    m_tableWidget->verticalHeader()->setSectionsMovable(true);
    connect(m_tableWidget->verticalHeader(), &QHeaderView::sectionMoved, this, [this](int logicalIndex, const int oldVisualIndex, const int newVisualIndex) {
        const QJsonValue tmp = m_databaseConfig.takeAt(oldVisualIndex);
        m_databaseConfig.insert(newVisualIndex, tmp);
        qDebug() << m_databaseConfig;
    });
    connect(m_tableWidget->verticalHeader(), &QHeaderView::sectionDoubleClicked, this, [this](const int logicalIndex) {
        bool ok = false;
        const QString input = QInputDialog::getText(this, "Rename", "", QLineEdit::Normal, m_tableWidget->verticalHeaderItem(logicalIndex)->text(), &ok);
        if (ok) {
            m_tableWidget->verticalHeaderItem(logicalIndex)->setText(input);
            const int visualRow = m_tableWidget->verticalHeader()->visualIndex(logicalIndex);
            databaseRename(visualRow);
        }
    });

    for (const QJsonValue &key: m_databaseConfig) {
        const int logicalIndex = m_tableWidget->rowCount();
        m_tableWidget->insertRow(logicalIndex);
        m_tableWidget->setVerticalHeaderItem(logicalIndex, new QTableWidgetItem(key.toString()));
        m_tableWidget->setItem(logicalIndex, 0, new QTableWidgetItem(""));
    }

    m_tableWidget->installEventFilter(this);
}

void Database::databaseConfigSave() const {
    g_config["databaseConfig"] = m_databaseConfig;
}

void Database::databaseWrite(const QString &key, const QString &value) const {
    for (int index = 0; index < m_tableWidget->rowCount(); index++) {
        if (m_tableWidget->verticalHeaderItem(index)->text() == key) {
            m_tableWidget->item(index, 0)->setText(value);
            return;
        }
    }
    qDebug() << "key not found in database";
}

void Database::databaseClear() const {
    for (int index = 0; index < m_tableWidget->rowCount(); index++) {
        m_tableWidget->item(index, 0)->setText("");
    }
}

// Database protected
void Database::contextMenuEvent(QContextMenuEvent *event) {
    const auto *vp = m_tableWidget->viewport();
    const QPoint vpPos = vp->mapFromGlobal(event->globalPos());
    if (!vp->rect().contains(vpPos)) return; // only show menu inside table(not header)
    const int logicalRow = m_tableWidget->indexAt(vpPos).row();
    const int visualRow = m_tableWidget->verticalHeader()->visualIndex(logicalRow);
    QMenu menu(this);
    if (logicalRow == -1) {
        menu.addAction(tr("new"), [this] {
            if (m_databaseConfig.isEmpty()) {
                databaseInsert(0);
            } else {
                databaseInsert(m_databaseConfig.size());
            }
        });
    } else {
        menu.addAction(tr("insert above (Ins)"), [this, visualRow] {
            databaseInsert(visualRow);
        });
        menu.addAction(tr("insert below (Ctrl+Ins)"), [this, visualRow] {
            databaseInsert(visualRow + 1);
        });
        menu.addAction(tr("remove (Del)"), [this, visualRow] {
            databaseRemove(visualRow);
        });
    }
    menu.exec(event->globalPos());
}

bool Database::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_tableWidget && event->type() == QEvent::KeyPress) {
        switch (static_cast<QKeyEvent *>(event)->key()) {
            case Qt::Key_Insert: {
                const int logicalRow = m_tableWidget->currentRow();
                const int visualRow = m_tableWidget->verticalHeader()->visualIndex(logicalRow);
                if (const auto keyEvent = static_cast<QKeyEvent *>(event); keyEvent->modifiers() & Qt::ControlModifier) {
                    databaseInsert(visualRow + 1);
                } else {
                    databaseInsert(visualRow);
                }
                return true;
            }
            case Qt::Key_Delete: {
                const int logicalRow = m_tableWidget->currentRow();
                const int visualRow = m_tableWidget->verticalHeader()->visualIndex(logicalRow);
                databaseRemove(visualRow);
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
void Database::databaseRename(const int visualRow) {
    const int logicalRow = m_tableWidget->verticalHeader()->logicalIndex(visualRow);
    m_databaseConfig[visualRow] = m_tableWidget->verticalHeaderItem(logicalRow)->text();
    qDebug() << m_databaseConfig;
}

void Database::databaseInsert(const int visualRow) {
    m_databaseConfig.insert(visualRow, "");
    m_tableWidget->insertRow(visualRow);
    m_tableWidget->setVerticalHeaderItem(visualRow, new QTableWidgetItem(""));
    m_tableWidget->setItem(visualRow, 0, new QTableWidgetItem(""));
    qDebug() << m_databaseConfig;
}

void Database::databaseRemove(const int visualRow) {
    const int logicalRow = m_tableWidget->verticalHeader()->logicalIndex(visualRow);
    m_tableWidget->removeRow(logicalRow);
    m_databaseConfig.removeAt(visualRow);
    qDebug() << m_databaseConfig;
}
