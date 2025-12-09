#include "scriptModule/codeAssistant/completionWidget.h"

#include <QHeaderView>
#include <QJsonArray>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>
#include <QVBoxLayout>

#include "globals.h"

// CompletionWidget public
CompletionWidget::CompletionWidget(QWidget *parent)
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
      m_fileButton(new QPushButton(this)),
      m_enummemberButton(new QPushButton(this)),
      m_resetButton(new QPushButton(this)),
      m_completionLabel(new QLabel(nullptr, Qt::ToolTip)) {
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("completionWidget");
    setWindowFlag(Qt::WindowDoesNotAcceptFocus, true);
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);
    layout->addWidget(m_completionListView);
    m_filterProxyModel->setSourceModel(m_completionModel);
    m_filterProxyModel->setFilterRole(Qt::UserRole + 1);
    m_completionListView->setFont(QFont("Consolas", 12));
    m_completionListView->setIconSize(QSize(16, 16));
    m_completionListView->setMinimumWidth(400);
    m_completionListView->setObjectName("completionListView");
    m_completionListView->setModel(m_filterProxyModel);
    connect(m_completionListView, &QListView::clicked, this, &CompletionWidget::labelShow);
    connect(m_completionListView, &QListView::doubleClicked, this, &CompletionWidget::textReplace);
    layout->addStretch();
    // completion filter
    {
        layout->addWidget(m_filterWidget);
        m_filterWidget->setObjectName("filterWidget");
        m_filterWidget->setFixedHeight(24);
        auto *filterLayout = new QHBoxLayout(m_filterWidget); // NOLINT
        filterLayout->setContentsMargins(0, 0, 0, 0);
        filterLayout->setSpacing(1);
        filterLayout->addWidget(m_textButton);
        m_textButton->setCheckable(true);
        m_textButton->setFixedSize(QSize(24, 24));
        m_textButton->setIcon(QIcon(":/icon/symbolString.svg"));
        m_textButton->setToolTip(tr("text"));
        connect(m_textButton, &QPushButton::clicked, this, &CompletionWidget::filterSet);
        m_filterButtonHash.insert(COMPLETION_KIND_TEXT, m_textButton);
        filterLayout->addWidget(m_functionButton);
        m_functionButton->setCheckable(true);
        m_functionButton->setFixedSize(QSize(24, 24));
        m_functionButton->setIcon(QIcon(":/icon/symbolMethod.svg"));
        m_functionButton->setToolTip(tr("function"));
        connect(m_functionButton, &QPushButton::clicked, this, &CompletionWidget::filterSet);
        m_filterButtonHash.insert(COMPLETION_KIND_FUNCTION, m_functionButton);
        filterLayout->addWidget(m_fieldButton);
        m_fieldButton->setCheckable(true);
        m_fieldButton->setFixedSize(QSize(24, 24));
        m_fieldButton->setIcon(QIcon(":/icon/symbolField.svg"));
        m_fieldButton->setToolTip(tr("field"));
        connect(m_fieldButton, &QPushButton::clicked, this, &CompletionWidget::filterSet);
        m_filterButtonHash.insert(COMPLETION_KIND_FIELD, m_fieldButton);
        filterLayout->addWidget(m_variableButton);
        m_variableButton->setCheckable(true);
        m_variableButton->setFixedSize(QSize(24, 24));
        m_variableButton->setIcon(QIcon(":/icon/symbolVariable.svg"));
        m_variableButton->setToolTip(tr("variable"));
        connect(m_variableButton, &QPushButton::clicked, this, &CompletionWidget::filterSet);
        m_filterButtonHash.insert(COMPLETION_KIND_VARIABLE, m_variableButton);
        filterLayout->addWidget(m_enumButton);
        m_enumButton->setCheckable(true);
        m_enumButton->setFixedSize(QSize(24, 24));
        m_enumButton->setIcon(QIcon(":/icon/symbolEnum.svg"));
        m_enumButton->setToolTip(tr("enum"));
        connect(m_enumButton, &QPushButton::clicked, this, &CompletionWidget::filterSet);
        m_filterButtonHash.insert(COMPLETION_KIND_ENUM, m_enumButton);
        filterLayout->addWidget(m_keywordButton);
        m_keywordButton->setCheckable(true);
        m_keywordButton->setFixedSize(QSize(24, 24));
        m_keywordButton->setIcon(QIcon(":/icon/symbolKeyword.svg"));
        m_keywordButton->setToolTip(tr("keyword"));
        connect(m_keywordButton, &QPushButton::clicked, this, &CompletionWidget::filterSet);
        m_filterButtonHash.insert(COMPLETION_KIND_KEYWORD, m_keywordButton);
        filterLayout->addWidget(m_fileButton);
        m_fileButton->setCheckable(true);
        m_fileButton->setFixedSize(QSize(24, 24));
        m_fileButton->setIcon(QIcon(":/icon/symbolFile.svg"));
        m_fileButton->setToolTip(tr("enum member"));
        connect(m_fileButton, &QPushButton::clicked, this, &CompletionWidget::filterSet);
        m_filterButtonHash.insert(COMPLETION_KIND_FILE, m_fileButton);
        filterLayout->addWidget(m_enummemberButton);
        m_enummemberButton->setCheckable(true);
        m_enummemberButton->setFixedSize(QSize(24, 24));
        m_enummemberButton->setIcon(QIcon(":/icon/symbolEnumMember.svg"));
        m_enummemberButton->setToolTip(tr("enum member"));
        connect(m_enummemberButton, &QPushButton::clicked, this, &CompletionWidget::filterSet);
        m_filterButtonHash.insert(COMPLETION_KIND_ENUMMEMBER, m_enummemberButton);
        filterLayout->addStretch();
        filterLayout->addWidget(m_resetButton);
        m_resetButton->setFixedSize(QSize(24, 24));
        m_resetButton->setIcon(QIcon(":/icon/reset.svg"));
        m_resetButton->setToolTip(tr("reset filter"));
        connect(m_resetButton, &QPushButton::clicked, this, [this] { filterInit(COMPLETION_MODE_FULL); });
    }
    m_completionLabel->setAttribute(Qt::WA_StyledBackground, true);
    m_completionLabel->setFont(QFont("Consolas", 12));
    m_completionLabel->setObjectName("completionLabel");
    // stylesheets
    setStyleSheet(
        "#completionWidget { background-color: white; border: 1px solid #cccccc; border-radius: 10px; }"
        "#completionListView { background: transparent; border: none;}"
        "#filterWidget { background-color: #fafafa; border: none; border-bottom-left-radius: 9px; border-bottom-right-radius: 9px; padding: 0px; }"
        "#filterWidget QPushButton { border: none; background-color: #fafafa; border-radius: 8px; padding: 4px; }"
        "#filterWidget QPushButton:checked { background-color: #cccccc; }");
    m_completionLabel->setStyleSheet("#completionLabel { background-color: white; border: 1px solid #cccccc; border-radius: 10px; padding: 2px; }");
}

void CompletionWidget::completionShow(const QVariantMap &completionSession, const QJsonArray &items) {
    m_completionSession = completionSession;
    m_completionModel->clear();
    m_completionKinds.clear();
    int row = 0;
    int completionMode = COMPLETION_MODE_FULL;
    for (const auto &value: items) {
        QJsonObject item = value.toObject();
        const int kind = item["kind"].toInt();
        m_completionKinds.insert(kind);
        const QString label = item["label"].toString();
        const QString insertText = item["insertText"].toString(label);
        if (insertText == "_ENV") completionMode = COMPLETION_MODE_SIMPLE;
        auto *completionItem = new QStandardItem(insertText); // NOLINT
        m_completionModel->appendRow(completionItem);
        completionItem->setData(kind, Qt::UserRole + 1);
        completionItem->setData(label, Qt::UserRole + 2);
        switch (kind) {
            case COMPLETION_KIND_TEXT: {
                completionItem->setIcon(QIcon(":/icon/symbolString.svg"));
            }
            break;
            case COMPLETION_KIND_METHOD: {
                completionItem->setIcon(QIcon(":/icon/symbolMethod.svg"));
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
            case COMPLETION_KIND_FILE: {
                completionItem->setIcon(QIcon(":/icon/symbolFile.svg"));
            }
            break;
            case COMPLETION_KIND_ENUMMEMBER: {
                completionItem->setIcon(QIcon(":/icon/symbolEnumMember.svg"));
            }
            break;
            default: {
                completionItem->setIcon(QIcon(":/icon/symbolMisc.svg"));
                qDebug() << "WIP completion kind:" << kind << insertText;
            }
            break;
        }
        row++;
    }
    if (m_completionModel->rowCount() > 0) {
        filterInit(completionMode);
    }
}

void CompletionWidget::completionHide() {
    hide();
}

void CompletionWidget::completionPrev() const {
    const QModelIndex currentIndex = m_completionListView->currentIndex();
    if (!currentIndex.isValid() || currentIndex.row() == 0) return;
    const QModelIndex prevIndex = m_filterProxyModel->index(currentIndex.row() - 1, 0);
    m_completionListView->setCurrentIndex(prevIndex);
    labelShow();
}

void CompletionWidget::completionNext() const {
    const QModelIndex currentIndex = m_completionListView->currentIndex();
    if (!currentIndex.isValid() || currentIndex.row() == m_filterProxyModel->rowCount() - 1) return;
    const QModelIndex nextIndex = m_filterProxyModel->index(currentIndex.row() + 1, 0);
    m_completionListView->setCurrentIndex(nextIndex);
    labelShow();
}

void CompletionWidget::textReplace() {
    const QModelIndex currentIndex = m_completionListView->currentIndex();
    if (!currentIndex.isValid()) return;
    const QModelIndex sourceIndex = m_filterProxyModel->mapToSource(currentIndex);
    const int kind = m_completionModel->data(sourceIndex, Qt::UserRole + 1).toInt();
    QString insertText = m_completionModel->data(sourceIndex, Qt::DisplayRole).toString();
    if (kind == COMPLETION_KIND_FUNCTION) {
        insertText += "()";
    } else if (kind == COMPLETION_KIND_FIELD) {
        insertText += ".";
    } else if (kind == COMPLETION_KIND_ENUMMEMBER) {
        if (insertText == "\"Add New Port\"") {
            emit insertPort();
            return;
        }
        if (insertText == "\"Add New Database Key\"") {
            emit insertDatabase();
            return;
        }
        if (insertText == "\"Add New Datatable Key\"") {
            emit insertDatatable();
            return;
        }
        if (insertText == "\"Position Hint\"") {
            const QVariantMap gotoSession = {
                {"scriptUrl", m_completionSession["scriptUrl"].toUrl()},
                {"line", m_completionSession["line"].toInt()},
                {"index", m_completionSession["indexFrom"].toInt()}
            };
            emit showPosition(gotoSession);
            return;
        }
        insertText.replace("\\", "\\\\");
    }
    emit replaceText(
        m_completionSession["scriptUrl"].toUrl(),
        insertText,
        m_completionSession["line"].toInt(),
        m_completionSession["indexFrom"].toInt(),
        m_completionSession["line"].toInt(),
        m_completionSession["indexTo"].toInt());
    int cursorPosition = 0;
    if (kind == COMPLETION_KIND_FUNCTION) {
        cursorPosition = m_completionSession["indexFrom"].toInt() + insertText.length() - 1;
    } else {
        cursorPosition = m_completionSession["indexFrom"].toInt() + insertText.length();
    }
    emit setCursorPosition(
        m_completionSession["scriptUrl"].toUrl(),
        m_completionSession["line"].toInt(),
        cursorPosition);
    if (kind == COMPLETION_KIND_FUNCTION) {
        emit addChar(m_completionSession["scriptUrl"].toUrl(), '(');
    } else if (kind == COMPLETION_KIND_FIELD) {
        emit addChar(m_completionSession["scriptUrl"].toUrl(), '.');
    }
}

// CompletionWidget protected
void CompletionWidget::hideEvent(QHideEvent *event) {
    m_completionLabel->hide();
    QWidget::hideEvent(event);
}

// CompletionWidget private
void CompletionWidget::filterClear() const {
    m_textButton->setChecked(false);
    m_functionButton->setChecked(false);
    m_fieldButton->setChecked(false);
    m_variableButton->setChecked(false);
    m_enumButton->setChecked(false);
    m_keywordButton->setChecked(false);
    m_enummemberButton->setChecked(false);
}

void CompletionWidget::filterInit(const int mode) {
    filterClear();
    if (mode == COMPLETION_MODE_FULL) {
        for (auto it = m_filterButtonHash.begin(); it != m_filterButtonHash.end(); ++it) {
            if (m_completionKinds.contains(it.key())) {
                it.value()->setEnabled(true);
                it.value()->setChecked(true);
            } else {
                it.value()->setEnabled(false);
            }
        }
    } else if (mode == COMPLETION_MODE_SIMPLE) {
        m_enummemberButton->setChecked(true);
    }
    filterSet(true);
}

void CompletionWidget::filterSet(const bool status) {
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
    if (m_filterProxyModel->rowCount() > 0) {
        m_completionListView->setCurrentIndex(m_filterProxyModel->index(0, 0));
        // calc height
        const int rowHeight = m_completionListView->sizeHintForRow(0);
        const int rowCount = m_filterProxyModel->rowCount();
        const int totalHeight = qMin(300, rowHeight * rowCount);
        show();
        labelShow();
        move(m_completionSession["position"].toPoint());
        QTimer::singleShot(0, this, [this, totalHeight] {
            m_completionListView->setFixedHeight(totalHeight);
            adjustSize();
        });
    } else {
        completionHide();
    }
}

void CompletionWidget::labelShow() const {
    const QModelIndex currentIndex = m_completionListView->currentIndex();
    if (!currentIndex.isValid()) {
        m_completionLabel->hide();
        return;
    }
    const auto sourceIndex = m_filterProxyModel->mapToSource(currentIndex);
    const QString label = m_completionModel->data(sourceIndex, Qt::UserRole + 2).toString();
    m_completionLabel->setText(label);
    m_completionLabel->show();
    const int y = m_completionListView->visualRect(currentIndex).top();
    QTimer::singleShot(0, this, [this, y] {
        m_completionLabel->adjustSize();
        m_completionLabel->move(mapToGlobal(QPoint(width(), y)));
    });
}
