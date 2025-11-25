#include "scriptModule/completionTooltip.h"

#include <QHeaderView>
#include <QJsonArray>
#include <QLabel>
#include <QListView>
#include <QTimer>
#include <QVBoxLayout>

#include "globals.h"

// CompletionTooltip public
CompletionTooltip::CompletionTooltip(QWidget *parent)
    : QWidget(parent, Qt::ToolTip),
      m_completionListView(new QListView(this)),
      m_completionModel(new QStandardItemModel(this)),
      m_completionLabel(new QLabel(nullptr, Qt::ToolTip)) {
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("completionTooltip");
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addWidget(m_completionListView);
    m_completionListView->setFont(QFont("Consolas", 12));
    m_completionListView->setIconSize(QSize(16, 16));
    m_completionListView->setMinimumWidth(400);
    m_completionListView->setObjectName("completionListView");
    m_completionListView->setModel(m_completionModel);
    connect(m_completionListView, &QListView::clicked, this, &CompletionTooltip::codeComplete);
    // stylesheets
    setStyleSheet(
        "#completionTooltip { background-color: white; border: 1px solid #cccccc; border-radius: 10px; }"
        "#completionListView { border: none;}");
    m_completionLabel->setAttribute(Qt::WA_StyledBackground, true);
    m_completionLabel->setFont(QFont("Consolas", 12));
    m_completionLabel->setObjectName("completionLabel");
    m_completionLabel->setStyleSheet("#completionLabel { background-color: white; border: 1px solid #cccccc; border-radius: 10px; padding: 2px; }");
}

void CompletionTooltip::tooltipShow(const QJsonArray &items) {
    m_completionModel->clear();
    int row = 0;
    for (const auto &value: items) {
        QJsonObject item = value.toObject();
        const int kind = item["kind"].toInt();
        if (!m_fullComplete && kind != COMPLETION_KIND_ENUMMEMBER) continue;
        const QString label = item["label"].toString();
        const QString insertText = item["insertText"].toString(label);
        auto *completionItem = new QStandardItem(insertText); // NOLINT
        m_completionModel->appendRow(completionItem);
        completionItem->setData(kind, Qt::UserRole + 1);
        completionItem->setData(label, Qt::UserRole + 2);
        switch (kind) {
            case COMPLETION_KIND_TEXT: {
                completionItem->setIcon(QIcon(":/icon/symbolString.svg"));
            }
            break;
            case COMPLETION_KIND_FUNCTION: {
                completionItem->setIcon(QIcon(":/icon/symbolMethod.svg"));
            }
            break;
            case COMPLETION_KIND_FIELD: {
                completionItem->setIcon(QIcon(":/icon/symbolField.svg"));
            }
            break;
            case COMPLETION_KIND_VARIABLE: {
                completionItem->setIcon(QIcon(":/icon/symbolVariable.svg"));
            }
            break;
            case COMPLETION_KIND_ENUM: {
                completionItem->setIcon(QIcon(":/icon/symbolEnum.svg"));
            }
            break;
            case COMPLETION_KIND_KEYWORD: {
                completionItem->setIcon(QIcon(":/icon/symbolKeyword.svg"));
            }
            break;
            case COMPLETION_KIND_ENUMMEMBER: {
                completionItem->setIcon(QIcon(":/icon/symbolEnumMember.svg"));
            }
            break;
            default: {
                qDebug() << "WIP completion kind:" << kind << insertText;
            }
            break;
        }
        row++;
    }
    if (m_completionModel->rowCount() > 0) {
        m_completionListView->setCurrentIndex(m_completionModel->index(0, 0));
        // calc height
        const int rowHeight = m_completionListView->sizeHintForRow(0);
        const int rowCount = m_completionModel->rowCount();
        const int totalHeight = rowHeight * rowCount;
        show();
        labelShow();
        QTimer::singleShot(0, this, [this, totalHeight] {
            m_completionListView->setFixedHeight(totalHeight);
            adjustSize();
        });
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

void CompletionTooltip::hideEvent(QHideEvent *event) {
    m_completionLabel->hide();
    QWidget::hideEvent(event);
}

// CompletionTooltip private
void CompletionTooltip::moveUp() {
    const QModelIndex currentIndex = m_completionListView->currentIndex();
    if (!currentIndex.isValid() || currentIndex.row() == 0) return;
    const QModelIndex prev = m_completionModel->index(currentIndex.row() - 1, 0);
    m_completionListView->setCurrentIndex(prev);
    labelShow();
}

void CompletionTooltip::moveDown() {
    const QModelIndex currentIndex = m_completionListView->currentIndex();
    if (!currentIndex.isValid() || currentIndex.row() == m_completionModel->rowCount() - 1) return;
    const QModelIndex next = m_completionModel->index(currentIndex.row() + 1, 0);
    m_completionListView->setCurrentIndex(next);
    labelShow();
}

void CompletionTooltip::codeComplete() {
    const QModelIndex currentIndex = m_completionListView->currentIndex();
    if (!currentIndex.isValid()) return;
    const int kind = m_completionModel->data(currentIndex, Qt::UserRole + 1).toInt();
    QString insertText = m_completionModel->data(currentIndex, Qt::DisplayRole).toString();
    if (!insertText.isEmpty()) emit completeCode(insertText, kind);
}

void CompletionTooltip::labelShow() {
    const QModelIndex currentIndex = m_completionListView->currentIndex();
    if (!currentIndex.isValid()) return;
    const QString label = m_completionModel->data(currentIndex, Qt::UserRole + 2).toString();
    m_completionLabel->setText(label);
    m_completionLabel->show();
    QTimer::singleShot(0, this, [this, currentIndex] {
        const int y = m_completionListView->visualRect(currentIndex).top() + 4;
        m_completionLabel->adjustSize();
        m_completionLabel->move(mapToGlobal(QPoint(width(), y)));
    });
}
