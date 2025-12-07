#include "dataModule/databaseModule.h"

#include <QContextMenuEvent>
#include <QDir>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QQmlContext>
#include <QQuickWidget>
#include <QStandardItemModel>
#include <QTableWidget>

#include "globals.h"

// DatabaseModule public
DatabaseModule::DatabaseModule()
    : DockWidget("database"),
      m_databaseWidget(new QQuickWidget()),
      m_databaseModel(new QStandardItemModel(this)),
      m_tableWidget(new QTableWidget()) {
    setWidget(m_tableWidget);
    m_tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_tableWidget->setColumnCount(1);
    m_tableWidget->horizontalHeader()->setVisible(false);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tableWidget->verticalHeader()->setMinimumWidth(30);
    m_tableWidget->verticalHeader()->setSectionsMovable(true);
    for (const auto &value: g_workspaceConfig["databaseConfig"].toArray()) {
        const QString key = value.toString();
        databaseInsert(-1, key);
    }
    databaseAnnotate();
    connect(m_tableWidget->verticalHeader(), &QHeaderView::sectionMoved, this, &DatabaseModule::databaseSwap);
    connect(m_tableWidget->verticalHeader(), &QHeaderView::sectionDoubleClicked, this, [this](const int logicalIndex) {
        const int visualIndex = m_tableWidget->verticalHeader()->visualIndex(logicalIndex);
        databaseRename(visualIndex);
    });
    m_tableWidget->installEventFilter(this);
}

void DatabaseModule::databaseConfigSave() const {
    g_workspaceConfig["databaseConfig"] = m_databaseConfig;
}

QVariantList DatabaseModule::databaseList() const {
    QVariantList databaseList{};
    for (const QString &portName: m_databaseHash.keys()) {
        databaseList.append(portName);
    }
    return databaseList;
}

void DatabaseModule::databaseInsert(int visualIndex, QString key) {
    if (visualIndex == -1) {
        visualIndex = m_databaseConfig.size();
    }
    if (key.isEmpty()) {
        bool ok = false;
        key = QInputDialog::getText(this, tr("Input Name"), "", QLineEdit::Normal, "", &ok);
        if (!ok) return;
        if (m_databaseHash.contains(key)) {
            QMessageBox::critical(this, tr("Error"), tr("Key already exists."));
            return;
        }
    }
    m_databaseHash.insert(key, m_databaseHash.size() - 1);
    // frontend
    m_tableWidget->insertRow(visualIndex);
    m_tableWidget->setVerticalHeaderItem(visualIndex, new QTableWidgetItem(key));
    m_tableWidget->setItem(visualIndex, 0, new QTableWidgetItem(""));
    // backend
    m_databaseConfig.insert(visualIndex, key);
    m_databaseHash.clear();
    for (int index = 0; index < m_tableWidget->rowCount(); ++index) {
        const QTableWidgetItem *headerItem = m_tableWidget->verticalHeaderItem(index);
        m_databaseHash.insert(headerItem->text(), index);
    }
    // logging
    emit appendLog(QString("%1 inserted").arg(key), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 inserted").arg(timestamp, key);
}

void DatabaseModule::databaseAnnotate() const {
    QString annotation;
    annotation += "--- @meta\n\n";
    annotation += "--- @alias database\n";
    for (const QString &databaseKey: m_databaseHash.keys()) {
        annotation += QString("--- | '\"%1\"'\n").arg(databaseKey);
    }
    annotation += QString("--- | '\"Add New Database Key\"'\n");
    annotation += "\n";

    const QString rootPath = g_workspaceUrl.toLocalFile();
    const QString annotationPath = QDir(rootPath).filePath("lib/database.d.lua");
    QFile file(annotationPath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&file);
    stream << annotation;
    file.close();
}

bool DatabaseModule::databaseWrite(const QString &key, const QString &value) const {
    if (!m_databaseHash.contains(key)) return false;
    m_tableWidget->item(m_databaseHash[key], 0)->setText(value);
    return true;
}

void DatabaseModule::databaseClear() const {
    for (int index = 0; index < m_tableWidget->rowCount(); index++) {
        m_tableWidget->item(index, 0)->setText("");
    }
}

// DatabaseModule protected
void DatabaseModule::contextMenuEvent(QContextMenuEvent *event) {
    const QPoint globalPos = event->globalPos();
    const auto *header = m_tableWidget->verticalHeader();
    const QPoint headerPos = header->mapFromGlobal(globalPos);
    if (header->rect().contains(headerPos)) {
        const int logicalIndex = header->logicalIndexAt(headerPos);
        const int visualIndex = m_tableWidget->verticalHeader()->visualIndex(logicalIndex);
        QMenu menu(this);
        if (logicalIndex == -1) {
            menu.addAction(tr("New"), [this] {
                databaseInsert(-1);
                databaseAnnotate();
            });
        } else {
            menu.addAction(tr("Rename"), [this, visualIndex] {
                databaseRename(visualIndex);
            });
            menu.addAction(tr("Insert Above \t Ins"), [this, visualIndex] {
                databaseInsert(visualIndex);
                databaseAnnotate();
            });
            menu.addAction(tr("Insert Below \t Ctrl+Ins"), [this, visualIndex] {
                databaseInsert(visualIndex + 1);
                databaseAnnotate();
            });
            menu.addAction(tr("Remove \t Del"), [this, visualIndex] {
                databaseRemove(visualIndex);
                databaseAnnotate();
            });
        }
        menu.exec(event->globalPos());
    }
}

bool DatabaseModule::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_tableWidget && event->type() == QEvent::KeyPress) {
        switch (static_cast<QKeyEvent *>(event)->key()) {
            case Qt::Key_Insert: {
                const int logicalIndex = m_tableWidget->currentRow();
                const int visualIndex = m_tableWidget->verticalHeader()->visualIndex(logicalIndex);
                if (const auto keyEvent = static_cast<QKeyEvent *>(event); keyEvent->modifiers() & Qt::ControlModifier) {
                    databaseInsert(visualIndex + 1);
                    databaseAnnotate();
                } else {
                    databaseInsert(visualIndex);
                    databaseAnnotate();
                }
                return true;
            }
            case Qt::Key_Delete: {
                const int logicalIndex = m_tableWidget->currentRow();
                const int visualIndex = m_tableWidget->verticalHeader()->visualIndex(logicalIndex);
                databaseRemove(visualIndex);
                databaseAnnotate();
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

// DatabaseModule private
void DatabaseModule::databaseRemove(const int visualIndex) {
    // frontend
    const int logicalIndex = m_tableWidget->verticalHeader()->logicalIndex(visualIndex);
    const QString key = m_tableWidget->verticalHeaderItem(logicalIndex)->text();
    m_tableWidget->removeRow(logicalIndex);
    // backend
    m_databaseConfig.removeAt(visualIndex);
    m_databaseHash.clear();
    for (int index = 0; index < m_tableWidget->rowCount(); ++index) {
        const QTableWidgetItem *headerItem = m_tableWidget->verticalHeaderItem(index);
        m_databaseHash.insert(headerItem->text(), index);
    }
    // logging
    emit appendLog(QString("%1 removed").arg(key), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 removed").arg(timestamp, key);
}

void DatabaseModule::databaseRename(const int visualIndex) {
    // frontend
    const int logicalIndex = m_tableWidget->verticalHeader()->logicalIndex(visualIndex);
    const QString oldKey = m_tableWidget->verticalHeaderItem(logicalIndex)->text();
    bool ok = false;
    const QString newKey = QInputDialog::getText(this, tr("Rename"), "", QLineEdit::Normal, oldKey, &ok);
    if (!ok) return;
    if (m_databaseHash.contains(newKey)) {
        QMessageBox::critical(this, tr("Error"), tr("Key already exists."));
        return;
    }
    m_tableWidget->verticalHeaderItem(logicalIndex)->setText(newKey);
    // backend
    m_databaseConfig[visualIndex] = newKey;
    m_databaseHash.clear();
    for (int index = 0; index < m_tableWidget->rowCount(); ++index) {
        const QTableWidgetItem *headerItem = m_tableWidget->verticalHeaderItem(index);
        m_databaseHash.insert(headerItem->text(), index);
    }
    databaseAnnotate();
    // logging
    emit appendLog(QString("%1 renamed").arg(newKey), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 renamed").arg(timestamp, newKey);
}

void DatabaseModule::databaseSwap(int logicalIndex, const int oldVisualIndex, const int newVisualIndex) {
    const QJsonValue tmp = m_databaseConfig.takeAt(oldVisualIndex);
    m_databaseConfig.insert(newVisualIndex, tmp);
}
