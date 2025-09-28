#include "../include/send.h"

// Send public
Send::Send(QObject *parent)
    : QDockWidget("send", qobject_cast<QWidget *>(parent)) {
    auto *widget = new QWidget(); // NOLINT
    setWidget(widget);
    auto *layout = new QVBoxLayout(widget); // NOLINT

    auto *sendWidget = new QWidget(); // NOLINT
    layout->addWidget(sendWidget);
    auto *sendLayout = new QHBoxLayout(sendWidget); // NOLINT
    sendLayout->setContentsMargins(0, 0, 0, 0);
    m_lineEdit = new QLineEdit();
    sendLayout->addWidget(m_lineEdit);
    auto *sendButton = new QPushButton(); // NOLINT
    sendLayout->addWidget(sendButton);
    sendButton->setFixedSize(24, 24);
    sendButton->setIcon(QIcon(":/icon/send.svg"));
    connect(sendButton, &QPushButton::clicked, this, [this] {
        QString txText = m_lineEdit->text();
        auto *portObject = m_port->portObject(-1);
        QMetaObject::invokeMethod(portObject, [portObject, txText] {
            portObject->writeText(txText);
        }, Qt::BlockingQueuedConnection);
    });

    m_tableWidget = new QTableWidget(); // NOLINT
    layout->addWidget(m_tableWidget);
    m_tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_tableWidget->setColumnCount(2);
    m_tableWidget->horizontalHeader()->setVisible(false);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tableWidget->verticalHeader()->setMinimumWidth(30);
    m_tableWidget->verticalHeader()->setSectionsMovable(true);
    connect(m_tableWidget->verticalHeader(), &QHeaderView::sectionMoved, this, [this](int logicalIndex, const int oldVisualIndex, const int newVisualIndex) {
        const QJsonValue tmp = m_sendConfig.takeAt(oldVisualIndex);
        m_sendConfig.insert(newVisualIndex, tmp);
        qDebug() << m_sendConfig;
    });

    for (const QJsonValue &shortcut: m_sendConfig) {
        const int logicalIndex = m_tableWidget->rowCount();
        m_tableWidget->insertRow(logicalIndex);
        m_tableWidget->setVerticalHeaderItem(logicalIndex, new QTableWidgetItem(shortcut[0].toString()));
        m_tableWidget->setItem(logicalIndex, 0, new QTableWidgetItem(shortcut[1].toString()));
        auto *shortcutSendButton = new QPushButton(); // NOLINT
        m_tableWidget->setCellWidget(logicalIndex, 1, shortcutSendButton);
        shortcutSendButton->setFixedSize(30, 30);
        shortcutSendButton->setIcon(QIcon(":/icon/send.svg"));
        connect(shortcutSendButton, &QPushButton::clicked, this, [this, logicalIndex] {
            commandSend(m_tableWidget->item(logicalIndex, 0)->text());
        });
    }

    m_tableWidget->installEventFilter(this);
    connect(m_tableWidget, &QTableWidget::cellChanged, this, [this](const int row, const int column) {
        const int visualIndex = m_tableWidget->verticalHeader()->visualIndex(row);
        shortcutRename(visualIndex, 1);
    });
}

void Send::sendConfigSave() const {
    g_config["sendConfig"] = m_sendConfig;
}

void Send::commandSend(const QString &txText) const {
    auto *portObject = m_port->portObject(-1);
    QMetaObject::invokeMethod(portObject, [portObject, txText] {
        portObject->writeText(txText);
    }, Qt::BlockingQueuedConnection);
}

// Send protected
void Send::contextMenuEvent(QContextMenuEvent *event) {
    const QPoint globalPos = event->globalPos();
    const auto *header = m_tableWidget->verticalHeader();
    const QPoint headerPos = header->mapFromGlobal(globalPos);
    if (header->rect().contains(headerPos)) {
        const int logicalIndex = header->logicalIndexAt(headerPos);
        const int visualIndex = m_tableWidget->verticalHeader()->visualIndex(logicalIndex);
        QMenu menu(this);
        if (logicalIndex == -1) {
            menu.addAction(tr("new"), [this] {
                if (m_sendConfig.isEmpty()) {
                    shortcutInsert(0);
                } else {
                    shortcutInsert(m_sendConfig.size());
                }
            });
        } else {
            menu.addAction(tr("rename"), [this, visualIndex] {
                shortcutRename(visualIndex, 0);
            });
            menu.addAction(tr("insert above (Ins)"), [this, visualIndex] {
                shortcutInsert(visualIndex);
            });
            menu.addAction(tr("insert below (Ctrl+Ins)"), [this, visualIndex] {
                shortcutInsert(visualIndex + 1);
            });
            menu.addAction(tr("remove (Del)"), [this, visualIndex] {
                shortcutRemove(visualIndex);
            });
        }
        menu.exec(event->globalPos());
    }
}

bool Send::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_tableWidget && event->type() == QEvent::KeyPress) {
        switch (static_cast<QKeyEvent *>(event)->key()) {
            case Qt::Key_Insert: {
                const int logicalIndex = m_tableWidget->currentRow();
                const int visualIndex = m_tableWidget->verticalHeader()->visualIndex(logicalIndex);
                if (const auto keyEvent = static_cast<QKeyEvent *>(event); keyEvent->modifiers() & Qt::ControlModifier) {
                    shortcutInsert(visualIndex + 1);
                } else {
                    shortcutInsert(visualIndex);
                }
                return true;
            }
            case Qt::Key_Delete: {
                const int logicalIndex = m_tableWidget->currentRow();
                const int visualIndex = m_tableWidget->verticalHeader()->visualIndex(logicalIndex);
                shortcutRemove(visualIndex);
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

// Send private
void Send::shortcutRename(const int visualIndex, const int type) {
    const int logicalIndex = m_tableWidget->verticalHeader()->logicalIndex(visualIndex);
    QJsonArray newShortcut = m_sendConfig[visualIndex].toArray();
    // gui
    if (type == 0) {
        bool ok = false;
        const QString newKey = QInputDialog::getText(this, "Rename", "", QLineEdit::Normal, m_tableWidget->verticalHeaderItem(logicalIndex)->text(), &ok);
        if (!ok) return;
        m_tableWidget->verticalHeaderItem(logicalIndex)->setText(newKey);
        newShortcut[0] = m_tableWidget->verticalHeaderItem(logicalIndex)->text();
    } else {
        newShortcut[1] = m_tableWidget->item(logicalIndex, 0)->text();
    }
    // config
    m_sendConfig.replace(visualIndex, newShortcut);
    qDebug() << m_sendConfig;
}

void Send::shortcutInsert(const int visualIndex) {
    QJsonArray newShortcut;
    newShortcut.append("");
    newShortcut.append("");
    m_sendConfig.insert(visualIndex, newShortcut);
    m_tableWidget->insertRow(visualIndex);
    m_tableWidget->setVerticalHeaderItem(visualIndex, new QTableWidgetItem(""));
    m_tableWidget->setItem(visualIndex, 0, new QTableWidgetItem(""));
    auto *shortcutSendButton = new QPushButton(); // NOLINT
    m_tableWidget->setCellWidget(visualIndex, 1, shortcutSendButton);
    shortcutSendButton->setFixedSize(30, 30);
    shortcutSendButton->setIcon(QIcon(":/icon/send.svg"));
    connect(shortcutSendButton, &QPushButton::clicked, this, [this, visualIndex] {
        commandSend(m_tableWidget->item(visualIndex, 0)->text());
    });
    qDebug() << m_sendConfig;
}

void Send::shortcutRemove(const int visualIndex) {
    const int logicalIndex = m_tableWidget->verticalHeader()->logicalIndex(visualIndex);
    m_tableWidget->removeRow(logicalIndex);
    m_sendConfig.removeAt(visualIndex);
    qDebug() << m_sendConfig;
}
