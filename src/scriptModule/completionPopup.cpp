#include "scriptModule/completionPopup.h"

#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QKeyEvent>
#include <QTableWidget>
#include <QVBoxLayout>

// CompletionPopup public
CompletionPopup::CompletionPopup(QWidget *parent)
    : QWidget(parent, Qt::ToolTip),
      m_tableWidget(new QTableWidget(this)) {
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_tableWidget);
    m_tableWidget->setFixedWidth(600);
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
    m_kindList = {
        "0", "Text", "Method", "Function", "Constructor", "Field", "Variable", "Class", "Interface", "Module", "Property", "Unit", "Value", "Enum", "Keyword", "Snippet", "Color",
        "File", "Reference", "Folder", "EnumMember", "Constant", "Struct", "Event", "Operator", "TypeParameter"
    };
}

void CompletionPopup::showTooltip(const QJsonArray &items) {
    m_tableWidget->setRowCount(0);
    int row = 0;
    for (const QJsonValue &value: items) {
        QJsonObject item = value.toObject();
        const QString kind = m_kindList[item["kind"].toInt()];
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
        m_currentRow = 0;
        m_tableWidget->selectRow(m_currentRow);
        m_insertText = m_tableWidget->item(m_currentRow, 0)->text();
        m_kind = m_tableWidget->item(m_currentRow, 1)->text();
    } else {
        m_currentRow = -1;
        m_kind.clear();
        m_insertText.clear();
    }
    m_tableWidget->resizeRowsToContents();
    this->adjustSize();
    this->show();
}

void CompletionPopup::hideTooltip() {
    this->hide();
}

// CompletionPopup protected
bool CompletionPopup::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress && this->isVisible()) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
            case Qt::Key_Tab:
                if (!m_insertText.isEmpty()) emit replaceText(m_insertText, m_kind);
                return true;
            case Qt::Key_Return:
                if (!m_insertText.isEmpty()) emit insertText(m_insertText, m_kind);
                return true;
            case Qt::Key_Escape:
                hideTooltip();
                return true;
            case Qt::Key_Up:
                moveUp();
                return true;
            case Qt::Key_Down:
                moveDown();
                return true;
            case Qt::Key_Left:
                return true;
            case Qt::Key_Right:
                return true;
            default:
                return false;
        }
    }
    return QWidget::eventFilter(obj, event);
}

// CompletionPopup private
void CompletionPopup::moveUp() {
    if (m_currentRow == -1) return;
    if (m_currentRow > 0) {
        m_currentRow--;
        m_tableWidget->selectRow(m_currentRow);
        m_insertText = m_tableWidget->item(m_currentRow, 0)->text();
        m_kind = m_tableWidget->item(m_currentRow, 1)->text();
    }
}

void CompletionPopup::moveDown() {
    if (m_currentRow == -1) return;
    if (m_currentRow < m_tableWidget->rowCount() - 1) {
        m_currentRow++;
        m_tableWidget->selectRow(m_currentRow);
        m_insertText = m_tableWidget->item(m_currentRow, 0)->text();
        m_kind = m_tableWidget->item(m_currentRow, 1)->text();
    }
}
