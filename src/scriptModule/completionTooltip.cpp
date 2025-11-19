#include "scriptModule/completionTooltip.h"

#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QKeyEvent>
#include <QTableWidget>
#include <QVBoxLayout>

// CompletionTooltip public
CompletionTooltip::CompletionTooltip(QWidget *parent)
    : QWidget(parent, Qt::ToolTip),
      m_tableWidget(new QTableWidget(this)),
      m_kindList{
          "0", "Text", "Method", "Function", "Constructor", "Field", "Variable", "Class", "Interface", "Module", "Property", "Unit", "Value",
          "Enum", "Keyword", "Snippet", "Color", "File", "Reference", "Folder", "EnumMember", "Constant", "Struct", "Event", "Operator", "TypeParameter"
      } {
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_tableWidget);
    // m_tableWidget->setFixedWidth(600);
    m_tableWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_tableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_tableWidget->setFont(QFont("Consolas", 12));
    m_tableWidget->setShowGrid(false);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setColumnCount(3);
    m_tableWidget->horizontalHeader()->setVisible(false);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_tableWidget->verticalHeader()->setVisible(false);
    connect(m_tableWidget, &QTableWidget::cellClicked, this, &CompletionTooltip::textReplace);
}

void CompletionTooltip::tooltipShow(const QJsonArray &items) {
    m_tableWidget->setRowCount(0);
    int row = 0;
    for (const auto &value: items) {
        QJsonObject item = value.toObject();
        const QString kind = m_kindList[item["kind"].toInt()];
        if (!m_fullComplete && kind != "EnumMember") continue;
        const QString label = item["label"].toString();
        const QString insertText = item["insertText"].toString(label);
        m_tableWidget->insertRow(row);
        auto *insertTextItem = new QTableWidgetItem(insertText); // NOLINT
        auto *kindItem = new QTableWidgetItem(kind); // NOLINT
        auto *labelItem = new QTableWidgetItem(label); // NOLINT
        m_tableWidget->setItem(row, 0, insertTextItem);
        m_tableWidget->setItem(row, 1, kindItem);
        m_tableWidget->setItem(row, 2, labelItem);
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
                textReplace();
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

void CompletionTooltip::textReplace() {
    const int currentRow = m_tableWidget->currentRow();
    if (currentRow == -1) return;
    QString insertText = m_tableWidget->item(currentRow, 0)->text();
    const QString kind = m_tableWidget->item(currentRow, 1)->text();
    if (!insertText.isEmpty() && !kind.isEmpty()) emit replaceText(insertText, kind);
}
