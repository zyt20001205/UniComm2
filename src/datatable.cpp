#include "../include/datatable.h"

// Datatable public
Datatable::Datatable(QObject *parent)
    : QDockWidget("data table", qobject_cast<QWidget *>(parent)) {
    m_tableWidget = new QTableWidget(); // NOLINT
    setWidget(m_tableWidget);
    m_tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->setRowCount(1);
    m_tableWidget->horizontalHeader()->setMinimumHeight(30);
    m_tableWidget->horizontalHeader()->setSectionsMovable(true);
    connect(m_tableWidget->horizontalHeader(), &QHeaderView::sectionMoved, this, [this](int logicalIndex, const int oldVisualIndex, const int newVisualIndex) {
        const QJsonValue tmp = m_datatableConfig.takeAt(oldVisualIndex);
        m_datatableConfig.insert(newVisualIndex, tmp);
        qDebug() << m_datatableConfig;
    });
    connect(m_tableWidget->horizontalHeader(), &QHeaderView::sectionDoubleClicked, this, [this](const int logicalIndex) {
        bool ok = false;
        const QString input = QInputDialog::getText(this, "Rename", "", QLineEdit::Normal, m_tableWidget->horizontalHeaderItem(logicalIndex)->text(), &ok);
        if (ok) {
            m_tableWidget->horizontalHeaderItem(logicalIndex)->setText(input);
            const int visualColumn = m_tableWidget->horizontalHeader()->visualIndex(logicalIndex);
            datatableRename(visualColumn);
        }
    });

    for (const QJsonValue &key: m_datatableConfig) {
        const int logicalIndex = m_tableWidget->columnCount();
        m_tableWidget->insertColumn(logicalIndex);
        m_tableWidget->setHorizontalHeaderItem(logicalIndex, new QTableWidgetItem(key.toString()));
        m_data[key.toString()] = DataMap{
            /*enable*/ true,
            /*basetime*/ {},
            /*x*/ {},
            /*y*/ {}
        };
    }

    m_tableWidget->installEventFilter(this);
}

void Datatable::datatableConfigSave() const {
    g_config["datatableConfig"] = m_datatableConfig;
}

bool Datatable::datatableWrite(const QString &key, const QString &value) {
    int column = -1;
    for (int index = 0; index < m_tableWidget->columnCount(); index++) {
        if (m_tableWidget->horizontalHeaderItem(index)->text() == key) {
            column = index;
            break;
        }
    }
    if (column == -1) {
        return false;
    }
    int row = -1;
    for (int index = 0; index < m_tableWidget->rowCount(); index++) {
        if (m_tableWidget->item(index, column) == nullptr || m_tableWidget->item(index, column)->text().isEmpty()) {
            row = index;
            break;
        }
    }
    if (row == -1) {
        row = m_tableWidget->rowCount();
        m_tableWidget->setRowCount(row + 1);
    }
    m_tableWidget->setItem(row, column, new QTableWidgetItem(value));
    if (!m_data[key].basetime.isValid()) {
        m_data[key].basetime = QDateTime::currentDateTime();
        m_data[key].x.append(0.0);
    } else {
        const double time = m_data[key].basetime.msecsTo(QDateTime::currentDateTime()) / 1000.0;
        m_data[key].x.append(time);
    }
    m_data[key].y.append(value.toDouble());
    return true;
}

void Datatable::datatableAddGraph(const QString &key) {
    if (!m_data.contains(key)) return;
    m_data[key].enable = true;
    emit addGraphDataPlot(m_data[key].x, m_data[key].y);
}

// Datatable protected
void Datatable::contextMenuEvent(QContextMenuEvent *event) {
    const auto *vp = m_tableWidget->viewport();
    const QPoint vpPos = vp->mapFromGlobal(event->globalPos());
    if (!vp->rect().contains(vpPos)) return; // only show menu inside table(not header)
    const int logicalColumn = m_tableWidget->indexAt(vpPos).column();
    const int visualColumn = m_tableWidget->horizontalHeader()->visualIndex(logicalColumn);
    QMenu menu(this);
    if (logicalColumn == -1) {
        menu.addAction(tr("new"), [this] {
            if (m_datatableConfig.isEmpty()) {
                datatableInsert(0);
            } else {
                datatableInsert(m_datatableConfig.size());
            }
        });
    } else {
        menu.addAction(tr("insert left (Ins)"), [this, visualColumn] {
            datatableInsert(visualColumn);
        });
        menu.addAction(tr("insert right (Ctrl+Ins)"), [this, visualColumn] {
            datatableInsert(visualColumn + 1);
        });
        menu.addAction(tr("remove (Del)"), [this, visualColumn] {
            datatableRemove(visualColumn);
        });
    }
    menu.exec(event->globalPos());
}

bool Datatable::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_tableWidget && event->type() == QEvent::KeyPress) {
        switch (static_cast<QKeyEvent *>(event)->key()) {
            case Qt::Key_Insert: {
                const int logicalColumn = m_tableWidget->currentColumn();
                const int visualColumn = m_tableWidget->horizontalHeader()->visualIndex(logicalColumn);
                if (const auto keyEvent = static_cast<QKeyEvent *>(event); keyEvent->modifiers() & Qt::ControlModifier) {
                    datatableInsert(visualColumn + 1);
                } else {
                    datatableInsert(visualColumn);
                }
                return true;
            }
            case Qt::Key_Delete: {
                const int logicalColumn = m_tableWidget->currentColumn();
                const int visualColumn = m_tableWidget->horizontalHeader()->visualIndex(logicalColumn);
                datatableRemove(visualColumn);
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

// Datatable private
void Datatable::datatableRename(const int visualColumn) {
    const int logicalColumn = m_tableWidget->horizontalHeader()->logicalIndex(visualColumn);
    m_datatableConfig[visualColumn] = m_tableWidget->horizontalHeaderItem(logicalColumn)->text();
    qDebug() << m_datatableConfig;
}

void Datatable::datatableInsert(const int visualColumn) {
    m_datatableConfig.insert(visualColumn, "");
    m_tableWidget->insertColumn(visualColumn);
    m_tableWidget->setHorizontalHeaderItem(visualColumn, new QTableWidgetItem(""));
    qDebug() << m_datatableConfig;
}

void Datatable::datatableRemove(const int visualColumn) {
    const int logicalColumn = m_tableWidget->horizontalHeader()->logicalIndex(visualColumn);
    m_tableWidget->removeColumn(logicalColumn);
    m_datatableConfig.removeAt(visualColumn);
    qDebug() << m_datatableConfig;
}
