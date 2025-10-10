#include "dataModule/datatableModule.h"

#include <QContextMenuEvent>
#include <QFile>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QPushButton>
#include <QTableWidget>

#include "globals.h"

// DatatableModule public
DatatableModule::DatatableModule(QWidget *parent)
    : QDockWidget("data table", parent),
      m_datatableConfig(g_config["datatableConfig"].toArray()),
      m_tableWidget(new QTableWidget()) {
    setWidget(m_tableWidget);
    m_tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_tableWidget->horizontalHeader()->setMinimumWidth(30);
    m_tableWidget->horizontalHeader()->setSectionsMovable(true);
    connect(m_tableWidget->horizontalHeader(), &QHeaderView::sectionMoved, this, [this](int logicalIndex, const int oldVisualIndex, const int newVisualIndex) {
        // config
        const QJsonValue tmp = m_datatableConfig.takeAt(oldVisualIndex);
        m_datatableConfig.insert(newVisualIndex, tmp);
        qDebug() << m_datatableConfig;
    });
    auto *clearButton = new QPushButton(); // NOLINT
    clearButton->setIcon(QIcon(":/icon/delete.svg"));
    m_tableWidget->setCornerWidget(clearButton);
    connect(clearButton, &QPushButton::clicked, this, [this] {
        datatableClear("all");
    });

    for (const QJsonValue &value: m_datatableConfig) {
        const QString key = value.toString();
        const int logicalIndex = m_tableWidget->columnCount();
        m_tableWidget->insertColumn(logicalIndex);
        m_tableWidget->setHorizontalHeaderItem(logicalIndex, new QTableWidgetItem(key));
        m_data[key] = DataMap{
            /*index*/ logicalIndex,
            /*enable*/ false,
            /*basetime*/ {},
            /*x*/ {},
            /*y*/ {}
        };
    }
    for (auto &key: m_data.keys()) {
        qDebug() << key << m_data[key].index;
    }
    m_tableWidget->installEventFilter(this);
}

void DatatableModule::datatableConfigSave() const {
    g_config["datatableConfig"] = m_datatableConfig;
}

void DatatableModule::datatableWrite(const QString &key, const QString &value) {
    if (!m_data.contains(key)) {
        qDebug() << "key not found in datatable";
        return;
    }

    double time = 0.0;
    if (!m_data[key].basetime.isValid()) {
        m_data[key].basetime = QDateTime::currentDateTime();
    } else {
        time = m_data[key].basetime.msecsTo(QDateTime::currentDateTime()) / 1000.0;
    }
    m_data[key].x.append(time);
    m_data[key].y.append(value.toDouble());
    if (m_data[key].enable) emit addPointDataPlot(key, time, value.toDouble());

    const int row = m_data[key].x.size() - 1;
    const int column = m_data.find(key).value().index;
    m_tableWidget->setRowCount(qMax(m_tableWidget->rowCount(), row + 1));
    m_tableWidget->setItem(row, column, new QTableWidgetItem(value));
    m_tableWidget->scrollToBottom();
}

void DatatableModule::datatableClear(const QString &key) {
    if (key == "all") {
        for (auto &data: m_data) {
            data.enable = false;
            data.basetime = {};
            data.x = {};
            data.y = {};
        }
        m_tableWidget->setRowCount(0);
    } else {
        if (!m_data.contains(key)) {
            qDebug() << "key not found in datatable";
            return;
        }

        m_data[key].enable = false;
        m_data[key].basetime = {};
        m_data[key].x = {};
        m_data[key].y = {};

        const int column = m_data.find(key).value().index;
        for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
            if (QTableWidgetItem *item = m_tableWidget->item(row, column)) {
                item->setText("");
            }
        }
    }
}

void DatatableModule::datatableAddGraph(const QString &key, const int position) {
    if (!m_data.contains(key)) {
        qDebug() << "key not found in datatable";
        return;
    }
    m_data[key].enable = true;
    emit addGraphDataPlot(key, m_data[key].x, m_data[key].y, position);
}

void DatatableModule::datatableExport() {
    const QString defaultName = "data_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".csv";
    QFile file(defaultName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    // write header
    const QList<QString> keyList = m_data.keys();
    const QString header = keyList.join(", ") + "\n";
    out << header;
    // calc length
    int rowCount = 0;
    foreach(const QString &key, keyList) {
        rowCount = qMax(rowCount, m_data[key].y.size());
    }
    // write data(y)
    for (int row = 0; row < rowCount; ++row) {
        QStringList rowData;
        foreach(const QString &key, keyList) {
            if (row < m_data[key].y.size()) {
                rowData << QString::number(m_data[key].y[row]);
            } else {
                rowData << "";
            }
        }
        out << rowData.join(",") << "\n";
    }
    file.close();
    // logging
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] data exported").arg(timestamp);
}

// DatatableModule protected
void DatatableModule::contextMenuEvent(QContextMenuEvent *event) {
    const QPoint globalPos = event->globalPos();
    const auto *header = m_tableWidget->horizontalHeader();
    const QPoint headerPos = header->mapFromGlobal(globalPos);
    if (header->rect().contains(headerPos)) {
        const int logicalIndex = header->logicalIndexAt(headerPos);
        const int visualIndex = header->visualIndex(logicalIndex);
        QMenu menu(this);
        if (logicalIndex == -1) {
            menu.addAction(tr("new"), [this] {
                if (m_datatableConfig.isEmpty()) {
                    datatableInsert(0);
                } else {
                    datatableInsert(m_datatableConfig.size());
                }
            });
        } else {
            menu.addAction(tr("rename"), [this, visualIndex] {
                datatableRename(visualIndex);
            });
            menu.addAction(tr("insert left (Ins)"), [this, visualIndex] {
                datatableInsert(visualIndex);
            });
            menu.addAction(tr("insert right (Ctrl+Ins)"), [this, visualIndex] {
                datatableInsert(visualIndex + 1);
            });
            menu.addAction(tr("remove (Del)"), [this, visualIndex] {
                datatableRemove(visualIndex);
            });
        }
        menu.exec(event->globalPos());
    }
}

bool DatatableModule::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_tableWidget && event->type() == QEvent::KeyPress) {
        switch (static_cast<QKeyEvent *>(event)->key()) {
            case Qt::Key_Insert: {
                const int logicalIndex = m_tableWidget->currentColumn();
                const int visualIndex = m_tableWidget->horizontalHeader()->visualIndex(logicalIndex);
                if (const auto keyEvent = static_cast<QKeyEvent *>(event); keyEvent->modifiers() & Qt::ControlModifier) {
                    datatableInsert(visualIndex + 1);
                } else {
                    datatableInsert(visualIndex);
                }
                return true;
            }
            case Qt::Key_Delete: {
                const int logicalIndex = m_tableWidget->currentColumn();
                const int visualIndex = m_tableWidget->horizontalHeader()->visualIndex(logicalIndex);
                datatableRemove(visualIndex);
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

// DatatableModule private
void DatatableModule::datatableRename(const int visualIndex) {
    const int logicalIndex = m_tableWidget->horizontalHeader()->logicalIndex(visualIndex);
    const QString oldKey = m_tableWidget->horizontalHeaderItem(logicalIndex)->text();
    // gui
    bool ok = false;
    const QString newKey = QInputDialog::getText(this, "Rename", "", QLineEdit::Normal, m_tableWidget->horizontalHeaderItem(logicalIndex)->text(), &ok);
    if (!ok) return;
    m_tableWidget->horizontalHeaderItem(logicalIndex)->setText(newKey);
    // config
    m_datatableConfig[visualIndex] = newKey;
    qDebug() << m_datatableConfig;
    // data
    m_data[newKey] = m_data.take(oldKey);
    for (auto &key: m_data.keys()) {
        qDebug() << key << m_data[key].index;
    }
}

void DatatableModule::datatableInsert(const int visualIndex) {
    const QString newKey = "";
    // gui
    m_tableWidget->insertColumn(visualIndex);
    m_tableWidget->setHorizontalHeaderItem(visualIndex, new QTableWidgetItem(newKey));
    // config
    m_datatableConfig.insert(visualIndex, newKey);
    qDebug() << m_datatableConfig;
    // data
    m_data[newKey] = DataMap{
        /*index*/ 0,
        /*enable*/ false,
        /*basetime*/ {},
        /*x*/ {},
        /*y*/ {}
    };
    int index = 0;
    for (const QJsonValue &value: m_datatableConfig) {
        const QString key = value.toString();
        m_data[key].index = m_tableWidget->horizontalHeader()->logicalIndex(index);
        index++;
    }
    for (auto &key: m_data.keys()) {
        qDebug() << key << m_data[key].index;
    }
}

void DatatableModule::datatableRemove(const int visualIndex) {
    const int logicalIndex = m_tableWidget->horizontalHeader()->logicalIndex(visualIndex);
    const QString oldKey = m_datatableConfig[visualIndex].toString();
    // gui
    m_tableWidget->removeColumn(logicalIndex);
    // config
    m_datatableConfig.removeAt(visualIndex);
    qDebug() << m_datatableConfig;
    // data
    m_data.remove(oldKey);
    int index = 0;
    for (const QJsonValue &value: m_datatableConfig) {
        const QString key = value.toString();
        m_data[key].index = m_tableWidget->horizontalHeader()->logicalIndex(index);
        index++;
    }
    for (auto &key: m_data.keys()) {
        qDebug() << key << m_data[key].index;
    }
}
