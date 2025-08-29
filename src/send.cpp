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
    m_tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_tableWidget->setColumnCount(2);
    m_tableWidget->horizontalHeader()->setVisible(false);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tableWidget->verticalHeader()->setSectionsMovable(true);
    connect(m_tableWidget->verticalHeader(), &QHeaderView::sectionMoved, this, [this](int logicalIndex, const int oldVisualIndex, const int newVisualIndex) {
        const QJsonValue tmp = m_sendConfig.takeAt(oldVisualIndex);
        m_sendConfig.insert(newVisualIndex, tmp);
        qDebug() << m_sendConfig;
    });
    connect(m_tableWidget->verticalHeader(), &QHeaderView::sectionDoubleClicked, this, [this](const int logicalIndex) {
        bool ok = false;
        const QString input = QInputDialog::getText(this, "Rename", "", QLineEdit::Normal, m_tableWidget->verticalHeaderItem(logicalIndex)->text(), &ok);
        if (ok) {
            m_tableWidget->verticalHeaderItem(logicalIndex)->setText(input);
            shortcutRename(logicalIndex, 0);
        }
    });

    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableWidget->setDragEnabled(true);
    m_tableWidget->setAcceptDrops(true);
    m_tableWidget->setDropIndicatorShown(true);
    m_tableWidget->setDragDropMode(QAbstractItemView::InternalMove);
    m_tableWidget->setDefaultDropAction(Qt::MoveAction);

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
        shortcutRename(row, 1);
    });
}

void Send::sendConfigSave() const {
    g_config["sendConfig"] = m_sendConfig;
}

void Send::commandSend(const QString &command) {
    emit writePort(-1, command, "");
}

// Send protected
void Send::contextMenuEvent(QContextMenuEvent *event) {
    const auto* vp = m_tableWidget->viewport();
    const QPoint vpPos = vp->mapFromGlobal(event->globalPos());
    if (!vp->rect().contains(vpPos)) return; // only show menu inside table(not header)
    const int logicalRow = m_tableWidget->indexAt(vpPos).row();
    const int visualRow = m_tableWidget->verticalHeader()->visualIndex(logicalRow);
    QMenu menu(this);
    if (logicalRow == -1) {
        menu.addAction(tr("new"), [this] {
            if (m_sendConfig.isEmpty()) {
                shortcutInsert(0);
            } else {
                shortcutInsert(m_sendConfig.size());
            }
        });
    } else {
        menu.addAction(tr("insert above (Ins)"), [this, visualRow] {
            shortcutInsert(visualRow);
        });
        menu.addAction(tr("insert below (Ctrl+Ins)"), [this, visualRow] {
            shortcutInsert(visualRow + 1);
        });
        menu.addAction(tr("remove (Del)"), [this, logicalRow] {
            shortcutRemove(logicalRow);
        });
    }
    menu.exec(event->globalPos());
}

bool Send::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_tableWidget && event->type() == QEvent::KeyPress) {
        switch (static_cast<QKeyEvent *>(event)->key()) {
            case Qt::Key_Insert: {
                if (const auto keyEvent = static_cast<QKeyEvent *>(event); keyEvent->modifiers() & Qt::ControlModifier) {
                    shortcutInsert(m_tableWidget->currentRow() + 1);
                } else {
                    shortcutInsert(m_tableWidget->currentRow());
                }
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
void Send::shortcutRename(const int logicalRow, const int column) {
    const int visualRow = m_tableWidget->verticalHeader()->visualIndex(logicalRow);
    QJsonArray newShortcut = m_sendConfig[visualRow].toArray();
    if (column == 0) {
        newShortcut[0] = m_tableWidget->verticalHeaderItem(logicalRow)->text();
    } else {
        newShortcut[1] = m_tableWidget->item(logicalRow, 0)->text();
    }
    m_sendConfig.replace(visualRow, newShortcut);
    qDebug() << m_sendConfig;
}

void Send::shortcutInsert(const int index) {
    QJsonArray newShortcut;
    newShortcut.append("");
    newShortcut.append("");
    m_sendConfig.insert(index, newShortcut);
    m_tableWidget->insertRow(index);
    m_tableWidget->setVerticalHeaderItem(index, new QTableWidgetItem(""));
    m_tableWidget->setItem(index, 0, new QTableWidgetItem(""));
    auto *shortcutSendButton = new QPushButton(); // NOLINT
    m_tableWidget->setCellWidget(index, 1, shortcutSendButton);
    shortcutSendButton->setFixedSize(30, 30);
    shortcutSendButton->setIcon(QIcon(":/icon/send.svg"));
    connect(shortcutSendButton, &QPushButton::clicked, this, [this, index] {
        commandSend(m_tableWidget->item(index, 0)->text());
    });
    qDebug() << m_sendConfig;
}

void Send::shortcutRemove(const int logicalIndex) {
    const int visualIndex = m_tableWidget->verticalHeader()->visualIndex(logicalIndex);
    m_tableWidget->removeRow(logicalIndex);
    m_sendConfig.removeAt(visualIndex);
    qDebug() << m_sendConfig;
}
