#include "scriptModule/completionTooltip.h"

#include <QHeaderView>
#include <QJsonArray>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>
#include <QVBoxLayout>

#include "globals.h"

// CompletionTooltip public
CompletionTooltip::CompletionTooltip(QWidget *parent)
    : QWidget(parent, Qt::ToolTip),
      m_completionListView(new QListView(this)),
      m_completionModel(new QStandardItemModel(this)),
      m_filterProxyModel(new QSortFilterProxyModel(this)),
      m_filterWidget(new QWidget(this)),
      m_textButton(new QPushButton(this)),
      m_functionButton(new QPushButton(this)),
      m_fieldButton(new QPushButton(this)),
      m_variableButton(new QPushButton(this)),
      m_enumButton(new QPushButton(this)),
      m_keywordButton(new QPushButton(this)),
      m_enummemberButton(new QPushButton(this)),
      m_resetButton(new QPushButton(this)),
      m_completionLabel(new QLabel(nullptr, Qt::ToolTip)) {
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("completionTooltip");
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);
    layout->addWidget(m_completionListView);
    m_filterProxyModel->setSourceModel(m_completionModel);
    m_filterProxyModel->setFilterRole(Qt::UserRole + 1);
    m_completionListView->setFocusPolicy(Qt::NoFocus);
    m_completionListView->setFont(QFont("Consolas", 12));
    m_completionListView->setIconSize(QSize(16, 16));
    m_completionListView->setMinimumWidth(400);
    m_completionListView->setObjectName("completionListView");
    m_completionListView->setModel(m_filterProxyModel);
    connect(m_completionListView, &QListView::doubleClicked, this, &CompletionTooltip::codeComplete);
    connect(m_completionListView->selectionModel(), &QItemSelectionModel::currentChanged, this, &CompletionTooltip::labelShow);
    layout->addStretch();
    layout->addWidget(m_filterWidget);
    m_filterWidget->setObjectName("filterWidget");
    m_filterWidget->setFixedHeight(24);
    auto *filterLayout = new QHBoxLayout(m_filterWidget); // NOLINT
    filterLayout->setContentsMargins(0, 0, 0, 0);
    filterLayout->setSpacing(1);
    filterLayout->addWidget(m_textButton);
    m_textButton->setCheckable(true);
    m_textButton->setFixedSize(QSize(24, 24));
    m_textButton->setFocusPolicy(Qt::NoFocus);
    m_textButton->setIcon(QIcon(":/icon/symbolString.svg"));
    m_textButton->setToolTip(tr("text"));
    connect(m_textButton, &QPushButton::clicked, this, &CompletionTooltip::filterSet);
    m_filterButtonHash.insert(COMPLETION_KIND_TEXT, m_textButton);
    filterLayout->addWidget(m_functionButton);
    m_functionButton->setCheckable(true);
    m_functionButton->setFixedSize(QSize(24, 24));
    m_functionButton->setFocusPolicy(Qt::NoFocus);
    m_functionButton->setIcon(QIcon(":/icon/symbolMethod.svg"));
    m_functionButton->setToolTip(tr("function"));
    connect(m_functionButton, &QPushButton::clicked, this, &CompletionTooltip::filterSet);
    m_filterButtonHash.insert(COMPLETION_KIND_FUNCTION, m_functionButton);
    filterLayout->addWidget(m_fieldButton);
    m_fieldButton->setCheckable(true);
    m_fieldButton->setFixedSize(QSize(24, 24));
    m_fieldButton->setFocusPolicy(Qt::NoFocus);
    m_fieldButton->setIcon(QIcon(":/icon/symbolField.svg"));
    m_fieldButton->setToolTip(tr("field"));
    connect(m_fieldButton, &QPushButton::clicked, this, &CompletionTooltip::filterSet);
    m_filterButtonHash.insert(COMPLETION_KIND_FIELD, m_fieldButton);
    filterLayout->addWidget(m_variableButton);
    m_variableButton->setCheckable(true);
    m_variableButton->setFixedSize(QSize(24, 24));
    m_variableButton->setFocusPolicy(Qt::NoFocus);
    m_variableButton->setIcon(QIcon(":/icon/symbolVariable.svg"));
    m_variableButton->setToolTip(tr("variable"));
    connect(m_variableButton, &QPushButton::clicked, this, &CompletionTooltip::filterSet);
    m_filterButtonHash.insert(COMPLETION_KIND_VARIABLE, m_variableButton);
    filterLayout->addWidget(m_enumButton);
    m_enumButton->setCheckable(true);
    m_enumButton->setFixedSize(QSize(24, 24));
    m_enumButton->setFocusPolicy(Qt::NoFocus);
    m_enumButton->setIcon(QIcon(":/icon/symbolEnum.svg"));
    m_enumButton->setToolTip(tr("enum"));
    connect(m_enumButton, &QPushButton::clicked, this, &CompletionTooltip::filterSet);
    m_filterButtonHash.insert(COMPLETION_KIND_ENUM, m_enumButton);
    filterLayout->addWidget(m_keywordButton);
    m_keywordButton->setCheckable(true);
    m_keywordButton->setFixedSize(QSize(24, 24));
    m_keywordButton->setFocusPolicy(Qt::NoFocus);
    m_keywordButton->setIcon(QIcon(":/icon/symbolKeyword.svg"));
    m_keywordButton->setToolTip(tr("keyword"));
    connect(m_keywordButton, &QPushButton::clicked, this, &CompletionTooltip::filterSet);
    m_filterButtonHash.insert(COMPLETION_KIND_KEYWORD, m_keywordButton);
    filterLayout->addWidget(m_enummemberButton);
    m_enummemberButton->setCheckable(true);
    m_enummemberButton->setFixedSize(QSize(24, 24));
    m_enummemberButton->setFocusPolicy(Qt::NoFocus);
    m_enummemberButton->setIcon(QIcon(":/icon/symbolEnumMember.svg"));
    m_enummemberButton->setToolTip(tr("enum member"));
    connect(m_enummemberButton, &QPushButton::clicked, this, &CompletionTooltip::filterSet);
    m_filterButtonHash.insert(COMPLETION_KIND_ENUMMEMBER, m_enummemberButton);
    filterLayout->addStretch();
    filterLayout->addWidget(m_resetButton);
    m_resetButton->setFixedSize(QSize(24, 24));
    m_resetButton->setFocusPolicy(Qt::NoFocus);
    m_resetButton->setIcon(QIcon(":/icon/reset.svg"));
    m_resetButton->setToolTip(tr("reset filter"));
    connect(m_resetButton, &QPushButton::clicked, this, &CompletionTooltip::filterInit);
    // stylesheets
    setStyleSheet(
        "#completionTooltip { background-color: white; border: 1px solid #cccccc; border-radius: 10px; }"
        "#completionListView { background: transparent; border: none;}"
        "#filterWidget { background-color: #fafafa; border: none; border-bottom-left-radius: 9px; border-bottom-right-radius: 9px; padding: 0px; }"
        "#filterWidget QPushButton { border: none; background-color: #fafafa; border-radius: 8px; padding: 4px; }"
        "#filterWidget QPushButton:checked { background-color: #cccccc; }");
    m_completionLabel->setAttribute(Qt::WA_StyledBackground, true);
    m_completionLabel->setFocusPolicy(Qt::NoFocus);
    m_completionLabel->setFont(QFont("Consolas", 12));
    m_completionLabel->setObjectName("completionLabel");
    m_completionLabel->setStyleSheet("#completionLabel { background-color: white; border: 1px solid #cccccc; border-radius: 10px; padding: 2px; }");
}

void CompletionTooltip::tooltipShow(const QJsonArray &items) {
    m_completionModel->clear();
    m_completionKinds.clear();
    int row = 0;
    for (const auto &value: items) {
        QJsonObject item = value.toObject();
        const int kind = item["kind"].toInt();
        if (!m_fullComplete && kind != COMPLETION_KIND_ENUMMEMBER) continue;
        m_completionKinds.insert(kind);
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
        filterInit();
        // calc height
        const int rowHeight = m_completionListView->sizeHintForRow(0);
        const int rowCount = m_completionModel->rowCount();
        const int totalHeight = rowHeight * rowCount;
        show();
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
    if (event->type() == QEvent::MouseButtonPress) {
        const auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (!geometry().contains(mapFromGlobal(mouseEvent->globalPosition().toPoint()))) {
            tooltipHide();
        }
    }
    return QWidget::eventFilter(obj, event);
}

void CompletionTooltip::hideEvent(QHideEvent *event) {
    m_completionLabel->hide();
    QWidget::hideEvent(event);
}

// CompletionTooltip private
void CompletionTooltip::moveUp() const {
    const QModelIndex currentIndex = m_completionListView->currentIndex();
    if (!currentIndex.isValid() || currentIndex.row() == 0) return;
    const QModelIndex prevIndex = m_filterProxyModel->index(currentIndex.row() - 1, 0);
    m_completionListView->setCurrentIndex(prevIndex);
}

void CompletionTooltip::moveDown() const {
    const QModelIndex currentIndex = m_completionListView->currentIndex();
    if (!currentIndex.isValid() || currentIndex.row() == m_filterProxyModel->rowCount() - 1) return;
    const QModelIndex nextIndex = m_filterProxyModel->index(currentIndex.row() + 1, 0);
    m_completionListView->setCurrentIndex(nextIndex);
}

void CompletionTooltip::codeComplete() {
    const QModelIndex currentIndex = m_completionListView->currentIndex();
    if (!currentIndex.isValid()) return;
    const QModelIndex sourceIndex = m_filterProxyModel->mapToSource(currentIndex);
    const int kind = m_completionModel->data(sourceIndex, Qt::UserRole + 1).toInt();
    QString insertText = m_completionModel->data(sourceIndex, Qt::DisplayRole).toString();
    if (!insertText.isEmpty()) emit completeCode(insertText, kind);
    tooltipHide();
}

void CompletionTooltip::filterClear() const {
    m_textButton->setChecked(false);
    m_functionButton->setChecked(false);
    m_fieldButton->setChecked(false);
    m_variableButton->setChecked(false);
    m_enumButton->setChecked(false);
    m_keywordButton->setChecked(false);
    m_enummemberButton->setChecked(false);
}

void CompletionTooltip::filterInit() {
    filterClear();
    for (auto it = m_filterButtonHash.begin(); it != m_filterButtonHash.end(); ++it) {
        if (m_completionKinds.contains(it.key())) {
            it.value()->setEnabled(true);
            it.value()->setChecked(true);
        } else {
            it.value()->setEnabled(false);
        }
    }
    filterSet(true);
}

void CompletionTooltip::filterSet(const bool status) {
    QString regExp{};
    for (auto it = m_filterButtonHash.begin(); it != m_filterButtonHash.end(); ++it) {
        if (it.value()->isChecked()) {
            regExp += QString::number(it.key());
            regExp += "|";
        }
    }
    if (!regExp.isEmpty()) regExp.chop(1);
    else regExp = "(?!.*)";
    m_filterProxyModel->setFilterRegularExpression(regExp);
    m_completionListView->setCurrentIndex(m_filterProxyModel->index(0, 0));
}

void CompletionTooltip::labelShow(const QModelIndex &currentIndex, const QModelIndex &previousIndex) const {
    if (!currentIndex.isValid()) {
        m_completionLabel->hide();
        return;
    }
    const auto sourceIndex = m_filterProxyModel->mapToSource(currentIndex);
    const QString label = m_completionModel->data(sourceIndex, Qt::UserRole + 2).toString();
    m_completionLabel->setText(label);
    m_completionLabel->show();
    const int y = m_completionListView->visualRect(currentIndex).top() + 1;
    QTimer::singleShot(0, this, [this, y] {
        m_completionLabel->adjustSize();
        m_completionLabel->move(mapToGlobal(QPoint(width(), y)));
    });
}
