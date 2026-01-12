#include "scriptModule/codeAnalysis/navigationWidget.h"

#include <QJsonArray>
#include <QLabel>
#include <QListView>
#include <QStandardItemModel>
#include <QTimer>
#include <QVBoxLayout>

#include "globals.h"

// NavigationWidget public
NavigationWidget::NavigationWidget(QWidget *parent)
    : QWidget(parent, Qt::ToolTip),
      m_navigationListView(new QListView(this)),
      m_navigationModel(new QStandardItemModel(this)),
      m_navigationLabel(new QLabel(nullptr, Qt::ToolTip)) {
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("navigationWidget");
    setWindowFlag(Qt::WindowDoesNotAcceptFocus, true);
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);
    layout->addWidget(m_navigationListView);
    m_navigationListView->setFont(QFont("Consolas", 12));
    m_navigationListView->setIconSize(QSize(8, 8));
    m_navigationListView->setMinimumWidth(400);
    m_navigationListView->setObjectName("navigationListView");
    m_navigationListView->setModel(m_navigationModel);
    connect(m_navigationListView, &QListView::clicked, this, &NavigationWidget::navigationRequest);
    connect(m_navigationListView, &QListView::doubleClicked, this, &NavigationWidget::navigationJump);
    m_navigationLabel->setAttribute(Qt::WA_StyledBackground, true);
    m_navigationLabel->setFont(QFont("Consolas", 12));
    m_navigationLabel->setObjectName("navigationLabel");
    m_navigationLabel->setTextFormat(Qt::RichText);
    // stylesheets
    setStyleSheet(
        "#navigationWidget { background-color: white; border: 1px solid #cccccc; border-radius: 10px; }"
        "#navigationListView { background: transparent; border: none;}");
    m_navigationLabel->setStyleSheet("#navigationLabel { background-color: white; border: 1px solid #cccccc; border-radius: 10px; padding: 2px; }");
}

void NavigationWidget::navigationShow(const QVariantMap &navigationSession, const QJsonArray &navigations) {
    m_navigationSession = navigationSession;
    int row = 0;
    for (const auto &value: navigations) {
        const QJsonObject navigation = value.toObject();
        QString uri = navigation["uri"].toString();
        uri = QUrl::fromPercentEncoding(uri.toUtf8());
        if (QChar &drive = uri[8]; drive.isLetter() && drive.isLower()) { drive = drive.toUpper(); }
        const QUrl scriptUrl(uri);
        const QJsonObject range = navigation["range"].toObject();
        const QJsonObject start = range["start"].toObject();
        const QJsonObject end = range["end"].toObject();
        const int startLine = start["line"].toInt();
        const int startCharacter = start["character"].toInt();
        const int endLine = end["line"].toInt();
        const int endCharacter = end["character"].toInt();
        auto *navigationItem = new QStandardItem(scriptUrl.fileName()); // NOLINT
        const QString type = navigationSession["type"].toString();
        if (type == "definition") {
            navigationItem->setIcon(QIcon(":/icon/definition.svg"));
        } else if (type == "implementation") {
            navigationItem->setIcon(QIcon(":/icon/implementation.svg"));
        } else if (type == "reference") {
            navigationItem->setIcon(QIcon(":/icon/reference.svg"));
        } else if (type == "typeDefinition") {
            navigationItem->setIcon(QIcon(":/icon/typeDefinition.svg"));
        }
        m_navigationModel->appendRow(navigationItem);
        navigationItem->setData(scriptUrl, Qt::UserRole + 1);
        navigationItem->setData(startLine, Qt::UserRole + 2);
        navigationItem->setData(startCharacter, Qt::UserRole + 3);
        navigationItem->setData(endLine, Qt::UserRole + 4);
        navigationItem->setData(endCharacter, Qt::UserRole + 5);
        row++;
    }
    if (m_navigationModel->rowCount() > 0) {
        m_navigationListView->setCurrentIndex(m_navigationModel->index(0, 0));
        // calc height
        const int rowHeight = m_navigationListView->sizeHintForRow(0);
        const int rowCount = m_navigationModel->rowCount();
        const int totalHeight = qMin(300, rowHeight * rowCount);
        show();
        navigationRequest();
        move(m_navigationSession["position"].toPoint());
        QTimer::singleShot(0, this, [this, totalHeight] {
            m_navigationListView->setFixedHeight(totalHeight);
            adjustSize();
        });
    }
}

void NavigationWidget::navigationHide() {
    hide();
}

void NavigationWidget::navigationPrev() {
    const QModelIndex currentIndex = m_navigationListView->currentIndex();
    if (!currentIndex.isValid() || currentIndex.row() == 0) return;
    const QModelIndex prevIndex = m_navigationModel->index(currentIndex.row() - 1, 0);
    m_navigationListView->setCurrentIndex(prevIndex);
    navigationRequest();
}

void NavigationWidget::navigationNext() {
    const QModelIndex currentIndex = m_navigationListView->currentIndex();
    if (!currentIndex.isValid() || currentIndex.row() == m_navigationModel->rowCount() - 1) return;
    const QModelIndex nextIndex = m_navigationModel->index(currentIndex.row() + 1, 0);
    m_navigationListView->setCurrentIndex(nextIndex);
    navigationRequest();
}

void NavigationWidget::navigationResponse(const QString &hint) const {
    const QModelIndex index = m_navigationListView->currentIndex();
    if (!index.isValid()) {
        m_navigationLabel->hide();
        return;
    }
    const int indexFrom = m_navigationModel->data(index, Qt::UserRole + 3).toInt();
    const int indexTo = m_navigationModel->data(index, Qt::UserRole + 5).toInt();
    const auto hintText = QString("<span style='white-space: pre;'>%1<span style='color: orange;'>%2</span>%3</span>").arg(
        hint.left(indexFrom).toHtmlEscaped(),
        hint.mid(indexFrom, indexTo - indexFrom).toHtmlEscaped(),
        hint.mid(indexTo).toHtmlEscaped());
    m_navigationLabel->setText(hintText);
    m_navigationLabel->show();
    const int y = m_navigationListView->visualRect(index).top();
    QTimer::singleShot(0, this, [this, y] {
        m_navigationLabel->adjustSize();
        m_navigationLabel->move(mapToGlobal(QPoint(width(), y)));
    });
}

// NavigationWidget protected
void NavigationWidget::hideEvent(QHideEvent *event) {
    m_navigationModel->clear();
    m_navigationLabel->hide();
    QWidget::hideEvent(event);
}

void NavigationWidget::leaveEvent(QEvent *event) {
    navigationHide();
    QWidget::leaveEvent(event);
}

// NavigationWidget private
void NavigationWidget::navigationJump(const QModelIndex &index) {
    const QUrl scriptUrl = m_navigationModel->data(index, Qt::UserRole + 1).toUrl();
    const int lineFrom = m_navigationModel->data(index, Qt::UserRole + 2).toInt();
    const int indexFrom = m_navigationModel->data(index, Qt::UserRole + 3).toInt();
    const int lineTo = m_navigationModel->data(index, Qt::UserRole + 4).toInt();
    const int indexTo = m_navigationModel->data(index, Qt::UserRole + 5).toInt();
    emit insertIndicator(scriptUrl, INDICATOR_SELECTION, lineFrom, indexFrom, lineTo, indexTo, 1000);
    emit setCursorPosition(scriptUrl, lineFrom, indexFrom);
    navigationHide();
}

void NavigationWidget::navigationRequest() {
    const QModelIndex index = m_navigationListView->currentIndex();
    if (!index.isValid()) {
        m_navigationLabel->hide();
        return;
    }
    const QUrl scriptUrl = m_navigationModel->data(index, Qt::UserRole + 1).toUrl();
    const int lineFrom = m_navigationModel->data(index, Qt::UserRole + 2).toInt();
    emit getText(scriptUrl, lineFrom, -1, -1, -1);
}
