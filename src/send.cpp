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
        commandSend(m_lineEdit->text());
    });

    m_tableWidget = new QTableWidget(); // NOLINT
    layout->addWidget(m_tableWidget);
    m_tableWidget->setColumnCount(3);
    m_tableWidget->horizontalHeader()->setVisible(false);
    m_tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableWidget->setDragEnabled(true);
    m_tableWidget->setAcceptDrops(true);
    m_tableWidget->setDragDropMode(QAbstractItemView::InternalMove);
    m_tableWidget->setDefaultDropAction(Qt::MoveAction);

    for (const QJsonValue &shortcut: m_sendConfig) {
        const int index = m_tableWidget->rowCount();
        m_tableWidget->insertRow(index);
        m_tableWidget->setItem(index, 0, new QTableWidgetItem(shortcut[0].toString()));
        m_tableWidget->setItem(index, 1, new QTableWidgetItem(shortcut[1].toString()));
        auto *shortcutSendButton = new QPushButton(); // NOLINT
        m_tableWidget->setCellWidget(index, 2, shortcutSendButton);
        shortcutSendButton->setFixedSize(30, 30);
        shortcutSendButton->setIcon(QIcon(":/icon/send.svg"));
        connect(shortcutSendButton, &QPushButton::clicked, this, [this, shortcutSendButton] {
            const int buttonIndex = m_tableWidget->indexAt(shortcutSendButton->pos()).row();
            commandSend(m_tableWidget->item(buttonIndex, 1)->text());
        });
    }

    m_tableWidget->installEventFilter(this);
    m_tableWidget->viewport()->installEventFilter(this);
    connect(m_tableWidget, &QTableWidget::cellChanged, this, [this](const int row, const int column) {
        shortcutRename(row, column);
    });
}

void Send::sendConfigSave() const {
    g_config["sendConfig"] = m_sendConfig;
}

void Send::commandSend(const QString &command) {
    emit writePort(-1, command, "");
}

// Send protected
bool Send::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_tableWidget->viewport()) {
        switch (event->type()) {
            case QEvent::DragMove: {
                m_tableWidget->clearSelection();
                m_currentIndex = m_tableWidget->indexAt(static_cast<QDragMoveEvent *>(event)->position().toPoint()).row();
                if (m_currentIndex == -1) break;
                if (m_previousIndex == -1) {
                    m_sourceKey = m_tableWidget->item(m_currentIndex, 0)->text();
                    m_sourceValue = m_tableWidget->item(m_currentIndex, 1)->text();
                    m_sourceIndex = m_currentIndex;
                    m_previousIndex = m_currentIndex;
                }
                if (m_previousIndex != m_currentIndex) {
                    m_tableWidget->removeRow(m_previousIndex);
                    m_tableWidget->insertRow(m_currentIndex);
                    m_tableWidget->setItem(m_currentIndex, 0, new QTableWidgetItem(m_sourceKey));
                    m_tableWidget->setItem(m_currentIndex, 1, new QTableWidgetItem(m_sourceValue));
                    auto *shortcutSendButton = new QPushButton(); // NOLINT
                    m_tableWidget->setCellWidget(m_currentIndex, 2, shortcutSendButton);
                    shortcutSendButton->setFixedSize(30, 30);
                    shortcutSendButton->setIcon(QIcon(":/icon/send.svg"));
                    connect(shortcutSendButton, &QPushButton::clicked, this, [this, shortcutSendButton] {
                        const int buttonIndex = m_tableWidget->indexAt(shortcutSendButton->pos()).row();
                        commandSend(m_tableWidget->item(buttonIndex, 1)->text());
                    });
                    m_previousIndex = m_currentIndex;
                }
                break;
            }
            case QEvent::Drop: {
                m_tableWidget->clearSelection();
                m_tableWidget->setCurrentItem(nullptr);
                m_tableWidget->clearFocus();
                m_previousIndex = -1;
                shortcutRename(m_sourceIndex, -1);
                return true;
            }
            default:
                break;
        }
    } else if (obj == m_tableWidget && event->type() == QEvent::KeyPress) {
        switch (static_cast<QKeyEvent *>(event)->key()) {
            case Qt::Key_Insert: {
                shortcutInsert(m_tableWidget->currentRow());
                return true;
            }
            case Qt::Key_Delete: {
                shortcutRemove(m_tableWidget->currentRow());
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
void Send::shortcutRename(const int row, const int column) {
    QJsonArray newShortcut = m_sendConfig[row].toArray();
    if (column == -1) {
        newShortcut[0] = m_tableWidget->item(row, 0)->text();
        newShortcut[1] = m_tableWidget->item(row, 1)->text();
    } else if (column == 0) {
        newShortcut[0] = m_tableWidget->item(row, 0)->text();
    } else {
        newShortcut[1] = m_tableWidget->item(row, 1)->text();
    }
    m_sendConfig.replace(row, newShortcut);
}

void Send::shortcutInsert(const int index) {
    m_tableWidget->insertRow(index);
    QJsonArray newShortcut;
    newShortcut.append("");
    newShortcut.append("");
    m_sendConfig.insert(index, newShortcut);
}

void Send::shortcutRemove(const int index) {
    m_tableWidget->removeRow(index);
    m_sendConfig.removeAt(index);
}
