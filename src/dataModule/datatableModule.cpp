#include "dataModule/datatableModule.h"

#include <QContextMenuEvent>
#include <QDir>
#include <QFile>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <qtoolbutton.h>

#include "globals.h"

// DatatableModule public
DatatableModule::DatatableModule()
    : DockWidget("data table"),
      m_tableWidget(new QTableWidget()) {
    setWidget(m_tableWidget);
    m_tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_tableWidget->horizontalHeader()->setMinimumHeight(30);
    m_tableWidget->horizontalHeader()->setSectionsMovable(true);
    connect(m_tableWidget->horizontalHeader(), &QHeaderView::sectionMoved, this, &DatatableModule::datatableSwap);
    connect(m_tableWidget->horizontalHeader(), &QHeaderView::sectionDoubleClicked, this, [this](const int logicalIndex) {
        const int visualIndex = m_tableWidget->horizontalHeader()->visualIndex(logicalIndex);
        datatableRename(visualIndex);
    });
    m_tableWidget->installEventFilter(this);
    auto *moreButton = new QToolButton(); // NOLINT
    moreButton->setPopupMode(QToolButton::InstantPopup);
    m_tableWidget->setCornerWidget(moreButton);

    auto *cornerMenu = new QMenu(); // NOLINT
    moreButton->setMenu(cornerMenu);
    auto *exportAction = new QAction(QIcon(":/icon/share.svg"), tr("export"), cornerMenu); // NOLINT
    cornerMenu->addAction(exportAction);
    connect(exportAction, &QAction::triggered, this, [this] { datatableExport(); });
    auto *clearAction = new QAction(QIcon(":/icon/delete.svg"), tr("clear"), cornerMenu); // NOLINT
    cornerMenu->addAction(clearAction);
    connect(clearAction, &QAction::triggered, this, [this] { datatableClear(""); });
}

void DatatableModule::workspaceOpen(const QUrl &rootUrl) {
    if (m_annotationUrl.isEmpty()) {
        // post initialization after workspace opened
        for (const auto &value: g_config["datatableConfig"].toArray()) {
            const QString key = value.toString();
            datatableInsert(-1, key);
        }
    } else {
        // nothing to do here
    }
    const QString rootPath = rootUrl.toLocalFile();
    const QString annotationPath = QDir(rootPath).filePath("lib/datatable.d.lua");
    m_annotationUrl = QUrl::fromLocalFile(annotationPath);
    datatableAnnotate();
}

void DatatableModule::datatableConfigSave() const {
    g_config["datatableConfig"] = m_datatableConfig;
}

QVariantList DatatableModule::datatableList() const {
    QVariantList datatableList{};
    for (const QString &portName: m_datatableHash.keys()) {
        datatableList.append(portName);
    }
    return datatableList;
}

void DatatableModule::datatableInsert(int visualIndex, QString key) {
    if (visualIndex == -1) {
        visualIndex = m_datatableConfig.size();
    }
    if (key.isEmpty()) {
        bool ok = false;
        key = QInputDialog::getText(this, tr("Input Name"), "", QLineEdit::Normal, "", &ok);
        if (!ok) return;
        if (m_datatableHash.contains(key)) {
            QMessageBox::critical(this, tr("Error"), tr("Key already exists."));
            return;
        }
    }
    // frontend
    m_tableWidget->insertColumn(visualIndex);
    m_tableWidget->setHorizontalHeaderItem(visualIndex, new QTableWidgetItem(key));
    // backend
    m_datatableConfig.insert(visualIndex, key);
    m_data.insert(key,
                  DataMap{
                      /*enable*/ false,
                      /*basetime*/ {},
                      /*x*/ {},
                      /*y*/ {}
                  });
    m_datatableHash.clear();
    for (int index = 0; index < m_tableWidget->columnCount(); ++index) {
        const QTableWidgetItem *headerItem = m_tableWidget->horizontalHeaderItem(index);
        m_datatableHash.insert(headerItem->text(), index);
    }
    datatableAnnotate();
    // logging
    emit appendLog(QString("%1 inserted").arg(key), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 inserted").arg(timestamp, key);
}

bool DatatableModule::datatableWrite(const QString &key, const QString &value) {
    if (!m_datatableHash.contains(key)) return false;

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
    const int column = m_datatableHash["key"];
    m_tableWidget->setRowCount(qMax(m_tableWidget->rowCount(), row + 1));
    m_tableWidget->setItem(row, column, new QTableWidgetItem(value));
    m_tableWidget->scrollToBottom();
    return true;
}

bool DatatableModule::datatableClear(const QString &key) {
    if (key.isEmpty()) {
        for (auto &data: m_data) {
            data.enable = false;
            data.basetime = {};
            data.x = {};
            data.y = {};
        }
        m_tableWidget->setRowCount(0);
        return true;
    }

    if (!m_datatableHash.contains(key)) return false;

    m_data[key].enable = false;
    m_data[key].basetime = {};
    m_data[key].x = {};
    m_data[key].y = {};
    const int column = m_datatableHash["key"];
    for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
        if (QTableWidgetItem *item = m_tableWidget->item(row, column)) {
            item->setText("");
        }
    }
    return true;
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
    const QUrl fileUrl = QUrl::fromLocalFile(file.fileName());
    emit appendLog(QString("data exported to <a href='%1'>%2</a>").arg(fileUrl.toString(), defaultName), "info");
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
            menu.addAction(tr("New"), [this] {
                datatableInsert(-1);
            });
        } else {
            menu.addAction(tr("Rename"), [this, visualIndex] {
                datatableRename(visualIndex);
            });
            menu.addAction(tr("Insert Left \t Ins"), [this, visualIndex] {
                datatableInsert(visualIndex);
            });
            menu.addAction(tr("Insert Right \t Ctrl+Ins"), [this, visualIndex] {
                datatableInsert(visualIndex + 1);
            });
            menu.addAction(tr("Remove \t Del"), [this, visualIndex] {
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
    return DockWidget::eventFilter(obj, event);
}

// DatatableModule private
void DatatableModule::datatableRemove(const int visualIndex) {
    // frontend
    const int logicalIndex = m_tableWidget->horizontalHeader()->logicalIndex(visualIndex);
    const QString key = m_datatableConfig[visualIndex].toString();
    m_tableWidget->removeColumn(logicalIndex);
    // backend
    m_datatableConfig.removeAt(visualIndex);
    m_data.remove(key);
    m_datatableHash.clear();
    for (int index = 0; index < m_tableWidget->columnCount(); ++index) {
        const QTableWidgetItem *headerItem = m_tableWidget->horizontalHeaderItem(index);
        m_datatableHash.insert(headerItem->text(), index);
    }
    datatableAnnotate();
    // logging
    emit appendLog(QString("%1 removed").arg(key), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 removed").arg(timestamp, key);
}

void DatatableModule::datatableRename(const int visualIndex) {
    // frontend
    const int logicalIndex = m_tableWidget->horizontalHeader()->logicalIndex(visualIndex);
    const QString oldKey = m_tableWidget->horizontalHeaderItem(logicalIndex)->text();
    bool ok = false;
    const QString newKey = QInputDialog::getText(this, "Rename", "", QLineEdit::Normal, m_tableWidget->horizontalHeaderItem(logicalIndex)->text(), &ok);
    if (!ok) return;
    if (m_datatableHash.contains(newKey)) {
        QMessageBox::critical(this, tr("Error"), tr("Key already exists."));
        return;
    }
    m_tableWidget->horizontalHeaderItem(logicalIndex)->setText(newKey);
    // backend
    m_datatableConfig[visualIndex] = newKey;
    m_data.insert(newKey, m_data.take(oldKey));
    m_datatableHash.clear();
    for (int index = 0; index < m_tableWidget->columnCount(); ++index) {
        const QTableWidgetItem *headerItem = m_tableWidget->horizontalHeaderItem(index);
        m_datatableHash.insert(headerItem->text(), index);
    }
    datatableAnnotate();
    // logging
    emit appendLog(QString("%1 renamed").arg(newKey), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 renamed").arg(timestamp, newKey);
}

void DatatableModule::datatableSwap(int logicalIndex, const int oldVisualIndex, const int newVisualIndex) {
    const QJsonValue tmp = m_datatableConfig.takeAt(oldVisualIndex);
    m_datatableConfig.insert(newVisualIndex, tmp);
}

void DatatableModule::datatableAnnotate() const {
    QString annotation;
    annotation += "--- @meta\n\n";
    annotation += "--- @alias datatable\n";
    for (const QString &databaseKey: m_datatableHash.keys()) {
        annotation += QString("--- | '\"%1\"'\n").arg(databaseKey);
    }
    annotation += QString("--- | '\"Add New Datatable Key\"'\n");
    annotation += "\n";

    QFile file(m_annotationUrl.toLocalFile());
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&file);
    stream << annotation;
    file.close();
}
