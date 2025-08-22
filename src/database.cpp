#include "../include/database.h"

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

    for (const QJsonValue &key: m_databaseConfig) {
        const int index = m_tableWidget->rowCount();
        m_tableWidget->insertRow(index);
        m_tableWidget->setItem(index, 0, new QTableWidgetItem(key.toString()));
    }
}

void Database::databaseConfigSave() const {
    g_config["databaseConfig"] = m_databaseConfig;
}

void Database::databaseWrite(const QString &key, const QString &value) {
    for (int index = 0; index < m_tableWidget->rowCount(); ++index) {
        if (m_tableWidget->item(index, 0)->text() == key) {
            m_tableWidget->setItem(index, 1, new QTableWidgetItem(value));
            return;
        }
    }
    emit appendLog("key not found", "error");
}
