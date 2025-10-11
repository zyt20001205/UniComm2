#include "dataModule/databaseModule.h"

#include <QContextMenuEvent>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QTableWidget>

#include "globals.h"

// DatabaseModule public
DatabaseModule::DatabaseModule(QWidget *parent)
    : QDockWidget("database", parent),
      m_databaseConfig(g_config["databaseConfig"].toArray()),
      m_tableWidget(new QTableWidget()) {
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

    for (const QJsonValue &key: m_databaseConfig) {
        const int logicalIndex = m_tableWidget->rowCount();
        m_tableWidget->insertRow(logicalIndex);
        m_tableWidget->setVerticalHeaderItem(logicalIndex, new QTableWidgetItem(key.toString()));
        m_tableWidget->setItem(logicalIndex, 0, new QTableWidgetItem(""));
    }

    m_tableWidget->installEventFilter(this);
}

void DatabaseModule::databaseConfigSave() const {
    g_config["databaseConfig"] = m_databaseConfig;
}

void DatabaseModule::databaseWrite(const QString &key, const QString &value) const {
    for (int index = 0; index < m_tableWidget->rowCount(); index++) {
        if (m_tableWidget->verticalHeaderItem(index)->text() == key) {
            m_tableWidget->item(index, 0)->setText(value);
            return;
        }
    }
    qDebug() << "key not found in database";
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
            menu.addAction(tr("new"), [this] {
                if (m_databaseConfig.isEmpty()) {
                    databaseInsert(0);
                } else {
                    databaseInsert(m_databaseConfig.size());
                }
            });
        } else {
            menu.addAction(tr("Rename"), [this, visualIndex] {
                databaseRename(visualIndex);
            });
            menu.addAction(tr("Insert Above \t Ins"), [this, visualIndex] {
                databaseInsert(visualIndex);
            });
            menu.addAction(tr("Insert Below \t Ctrl+Ins"), [this, visualIndex] {
                databaseInsert(visualIndex + 1);
            });
            menu.addAction(tr("Remove \t Del"), [this, visualIndex] {
                databaseRemove(visualIndex);
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
                } else {
                    databaseInsert(visualIndex);
                }
                return true;
            }
            case Qt::Key_Delete: {
                const int logicalIndex = m_tableWidget->currentRow();
                const int visualIndex = m_tableWidget->verticalHeader()->visualIndex(logicalIndex);
                databaseRemove(visualIndex);
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

// DatabaseModule private
void DatabaseModule::databaseRename(const int visualIndex) {
    const int logicalIndex = m_tableWidget->verticalHeader()->logicalIndex(visualIndex);
    const QString oldKey = m_tableWidget->verticalHeaderItem(logicalIndex)->text();
    // gui
    bool ok = false;
    const QString newKey = QInputDialog::getText(this, "Rename", "", QLineEdit::Normal, m_tableWidget->verticalHeaderItem(logicalIndex)->text(), &ok);
    if (!ok) return;
    m_tableWidget->verticalHeaderItem(logicalIndex)->setText(newKey);
    // config
    m_databaseConfig[visualIndex] = m_tableWidget->verticalHeaderItem(logicalIndex)->text();
    qDebug() << m_databaseConfig;
}

void DatabaseModule::databaseInsert(const int visualIndex) {
    m_databaseConfig.insert(visualIndex, "");
    m_tableWidget->insertRow(visualIndex);
    m_tableWidget->setVerticalHeaderItem(visualIndex, new QTableWidgetItem(""));
    m_tableWidget->setItem(visualIndex, 0, new QTableWidgetItem(""));
    qDebug() << m_databaseConfig;
}

void DatabaseModule::databaseRemove(const int visualIndex) {
    const int logicalIndex = m_tableWidget->verticalHeader()->logicalIndex(visualIndex);
    m_tableWidget->removeRow(logicalIndex);
    m_databaseConfig.removeAt(visualIndex);
    qDebug() << m_databaseConfig;
}