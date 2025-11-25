#include "scriptModule/completionTooltip.h"

#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QKeyEvent>
#include <QTableWidget>
#include <QVBoxLayout>

#include "globals.h"

// CompletionTooltip public
CompletionTooltip::CompletionTooltip(QWidget *parent)
    : QWidget(parent, Qt::ToolTip),
      m_tableWidget(new QTableWidget(this)) {
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("completionTooltip");
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addWidget(m_tableWidget);
    m_tableWidget->setMinimumWidth(400);
    m_tableWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_tableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_tableWidget->setFont(QFont("Consolas", 12));
    m_tableWidget->setObjectName("tableWidget");
    m_tableWidget->setShowGrid(false);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setColumnCount(2);
    m_tableWidget->horizontalHeader()->setVisible(false);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableWidget->verticalHeader()->setVisible(false);
    connect(m_tableWidget, &QTableWidget::cellClicked, this, &CompletionTooltip::codeComplete);
    // stylesheets
    setStyleSheet(
        "#completionTooltip { background-color: white; border: 1px solid #cccccc; border-radius: 10px; }"
        "#tableWidget { border: none;}");
}

void CompletionTooltip::tooltipShow(const QJsonArray &items) {
    m_tableWidget->setRowCount(0);
    int row = 0;
    for (const auto &value: items) {
        QJsonObject item = value.toObject();
        const int kind = item["kind"].toInt();
        if (!m_fullComplete && kind != COMPLETION_KIND_ENUMMEMBER) continue;
        const QString label = item["label"].toString();
        const QString insertText = item["insertText"].toString(label);
        m_tableWidget->insertRow(row);
        auto *kindItem = new QTableWidgetItem(); // NOLINT
        kindItem->setData(Qt::UserRole + 1, kind);
        switch (kind) {
            case COMPLETION_KIND_TEXT: {
                kindItem->setIcon(QIcon(":/icon/symbolString.svg"));
            }
            break;
            case COMPLETION_KIND_FUNCTION: {
                kindItem->setIcon(QIcon(":/icon/symbolMethod.svg"));
            }
            break;
            case COMPLETION_KIND_FIELD: {
                kindItem->setIcon(QIcon(":/icon/symbolField.svg"));
            }
            break;
            case COMPLETION_KIND_VARIABLE: {
                kindItem->setIcon(QIcon(":/icon/symbolVariable.svg"));
            }
            break;
            case COMPLETION_KIND_ENUM: {
                kindItem->setIcon(QIcon(":/icon/symbolEnum.svg"));
            }
            break;
            case COMPLETION_KIND_KEYWORD: {
                kindItem->setIcon(QIcon(":/icon/symbolKeyword.svg"));
            }
            break;
            case COMPLETION_KIND_ENUMMEMBER: {
                kindItem->setIcon(QIcon(":/icon/symbolEnumMember.svg"));
            }
            break;
            default: {
                qDebug() << "WIP completion kind:" << kind << insertText;
            }
            break;
        }
        auto *insertTextItem = new QTableWidgetItem(insertText); // NOLINT
        m_tableWidget->setItem(row, 0, kindItem);
        m_tableWidget->setItem(row, 1, insertTextItem);
        row++;
    }
    if (m_tableWidget->rowCount() > 0) {
        m_tableWidget->resizeColumnsToContents();
        m_tableWidget->resizeRowsToContents();
        m_tableWidget->selectRow(0);
        adjustSize();
        show();
    }
}

void CompletionTooltip::tooltipHide() {
    hide();
}

void CompletionTooltip::tooltipFull(const bool status) {
    m_fullComplete = status;
}

// CompletionTooltip protected
bool CompletionTooltip::eventFilter(QObject *obj, QEvent *event) {
    if (!this->isVisible()) {
        return QWidget::eventFilter(obj, event);
    }
    if (event->type() == QEvent::KeyPress) {
        const auto *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
            case Qt::Key_Tab: {
                codeComplete();
                tooltipHide();
            }
                return true;
            case Qt::Key_Up: {
                moveUp();
            }
                return true;
            case Qt::Key_Down: {
                moveDown();
            }
                return true;
            case Qt::Key_Return:
            case Qt::Key_Escape:
            case Qt::Key_Backspace:
            case Qt::Key_Left:
            case Qt::Key_Right: {
                tooltipHide();
            }
                return false;
            default:
                return false;
        }
    }
    return QWidget::eventFilter(obj, event);
}

// CompletionTooltip private
void CompletionTooltip::moveUp() const {
    int currentRow = m_tableWidget->currentRow();
    if (currentRow == -1 || currentRow == 0) return;
    currentRow--;
    m_tableWidget->selectRow(currentRow);
}

void CompletionTooltip::moveDown() const {
    int currentRow = m_tableWidget->currentRow();
    if (currentRow == -1 || currentRow == m_tableWidget->rowCount() - 1) return;
    currentRow++;
    m_tableWidget->selectRow(currentRow);
}

void CompletionTooltip::codeComplete() {
    const int currentRow = m_tableWidget->currentRow();
    if (currentRow == -1) return;
    const int kind = m_tableWidget->item(currentRow, 0)->data(Qt::UserRole + 1).toInt();
    QString insertText = m_tableWidget->item(currentRow, 1)->text();
    if (!insertText.isEmpty()) emit completeCode(insertText, kind);
}
