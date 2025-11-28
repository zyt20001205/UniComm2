#include "scriptModule/codeAssistant/gotoWidget.h"

#include <QJsonArray>
#include <QLabel>
#include <QListView>
#include <QStandardItemModel>
#include <QTimer>
#include <QVBoxLayout>

#include "globals.h"

// GotoWidget public
GotoWidget::GotoWidget(QWidget *parent)
    : QWidget(parent, Qt::ToolTip),
      m_gotoListView(new QListView(this)),
      m_gotoModel(new QStandardItemModel(this)),
      m_gotoLabel(new QLabel(nullptr, Qt::ToolTip)) {
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("gotoWidget");
    setWindowFlag(Qt::WindowDoesNotAcceptFocus, true);
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);
    layout->addWidget(m_gotoListView);
    m_gotoListView->setFont(QFont("Consolas", 12));
    m_gotoListView->setIconSize(QSize(8, 8));
    m_gotoListView->setMinimumWidth(400);
    m_gotoListView->setObjectName("gotoListView");
    m_gotoListView->setModel(m_gotoModel);
    connect(m_gotoListView, &QListView::clicked, this, &GotoWidget::gotoRequest);
    connect(m_gotoListView, &QListView::doubleClicked, this, &GotoWidget::gotoJump);
    m_gotoLabel->setAttribute(Qt::WA_StyledBackground, true);
    m_gotoLabel->setFont(QFont("Consolas", 12));
    m_gotoLabel->setObjectName("gotoLabel");
    m_gotoLabel->setTextFormat(Qt::RichText);
    // stylesheets
    setStyleSheet(
        "#gotoWidget { background-color: white; border: 1px solid #cccccc; border-radius: 10px; }"
        "#gotoListView { background: transparent; border: none;}");
    m_gotoLabel->setStyleSheet("#gotoLabel { background-color: white; border: 1px solid #cccccc; border-radius: 10px; padding: 2px; }");
}

void GotoWidget::gotoShowDefinition(const QVariantMap &gotoSession, const QJsonArray &definitions) {
    m_gotoSession = gotoSession;
    int row = 0;
    for (const auto &value: definitions) {
        const QJsonObject definition = value.toObject();
        QString uri = definition["uri"].toString();
        uri = QUrl::fromPercentEncoding(uri.toUtf8());
        if (QChar &drive = uri[8]; drive.isLetter() && drive.isLower()) { drive = drive.toUpper(); }
        const QUrl scriptUrl(uri);
        const QJsonObject rangeObject = definition["range"].toObject();
        const QJsonObject startObject = rangeObject["start"].toObject();
        const QJsonObject endObject = rangeObject["end"].toObject();
        const int startLine = startObject["line"].toInt();
        const int startCharacter = startObject["character"].toInt();
        const int endLine = endObject["line"].toInt();
        const int endCharacter = endObject["character"].toInt();
        auto *gotoItem = new QStandardItem(QIcon(":/icon/definition.svg"), scriptUrl.fileName()); // NOLINT
        m_gotoModel->appendRow(gotoItem);
        gotoItem->setData(scriptUrl, Qt::UserRole + 1);
        gotoItem->setData(startLine, Qt::UserRole + 2);
        gotoItem->setData(startCharacter, Qt::UserRole + 3);
        gotoItem->setData(endLine, Qt::UserRole + 4);
        gotoItem->setData(endCharacter, Qt::UserRole + 5);
        row++;
    }
    if (m_gotoModel->rowCount() > 0) {
        m_gotoListView->setCurrentIndex(m_gotoModel->index(0, 0));
        // calc height
        const int rowHeight = m_gotoListView->sizeHintForRow(0);
        const int rowCount = m_gotoModel->rowCount();
        const int totalHeight = qMin(300, rowHeight * rowCount);
        show();
        gotoRequest();
        move(m_gotoSession["position"].toPoint());
        QTimer::singleShot(0, this, [this, totalHeight] {
            m_gotoListView->setFixedHeight(totalHeight);
            adjustSize();
        });
    }
}

void GotoWidget::gotoShowImplementation(const QVariantMap &gotoSession, const QJsonArray &implementations) {
    m_gotoSession = gotoSession;
    int row = 0;
    for (const auto &value: implementations) {
        const QJsonObject definition = value.toObject();
        QString uri = definition["uri"].toString();
        uri = QUrl::fromPercentEncoding(uri.toUtf8());
        if (QChar &drive = uri[8]; drive.isLetter() && drive.isLower()) { drive = drive.toUpper(); }
        const QUrl scriptUrl(uri);
        const QJsonObject rangeObject = definition["range"].toObject();
        const QJsonObject startObject = rangeObject["start"].toObject();
        const QJsonObject endObject = rangeObject["end"].toObject();
        const int startLine = startObject["line"].toInt();
        const int startCharacter = startObject["character"].toInt();
        const int endLine = endObject["line"].toInt();
        const int endCharacter = endObject["character"].toInt();
        auto *gotoItem = new QStandardItem(QIcon(":/icon/implementation.svg"), scriptUrl.fileName()); // NOLINT
        m_gotoModel->appendRow(gotoItem);
        gotoItem->setData(scriptUrl, Qt::UserRole + 1);
        gotoItem->setData(startLine, Qt::UserRole + 2);
        gotoItem->setData(startCharacter, Qt::UserRole + 3);
        gotoItem->setData(endLine, Qt::UserRole + 4);
        gotoItem->setData(endCharacter, Qt::UserRole + 5);
        row++;
    }
    if (m_gotoModel->rowCount() > 0) {
        m_gotoListView->setCurrentIndex(m_gotoModel->index(0, 0));
        // calc height
        const int rowHeight = m_gotoListView->sizeHintForRow(0);
        const int rowCount = m_gotoModel->rowCount();
        const int totalHeight = qMin(300, rowHeight * rowCount);
        show();
        gotoRequest();
        move(m_gotoSession["position"].toPoint());
        QTimer::singleShot(0, this, [this, totalHeight] {
            m_gotoListView->setFixedHeight(totalHeight);
            adjustSize();
        });
    }
}

void GotoWidget::gotoShowReferences(const QVariantMap &gotoSession, const QJsonArray &references) {
    m_gotoSession = gotoSession;
    int row = 0;
    for (const auto &value: references) {
        const QJsonObject definition = value.toObject();
        QString uri = definition["uri"].toString();
        uri = QUrl::fromPercentEncoding(uri.toUtf8());
        if (QChar &drive = uri[8]; drive.isLetter() && drive.isLower()) { drive = drive.toUpper(); }
        const QUrl scriptUrl(uri);
        const QJsonObject rangeObject = definition["range"].toObject();
        const QJsonObject startObject = rangeObject["start"].toObject();
        const QJsonObject endObject = rangeObject["end"].toObject();
        const int startLine = startObject["line"].toInt();
        const int startCharacter = startObject["character"].toInt();
        const int endLine = endObject["line"].toInt();
        const int endCharacter = endObject["character"].toInt();
        auto *gotoItem = new QStandardItem(QIcon(":/icon/reference.svg"), scriptUrl.fileName()); // NOLINT
        m_gotoModel->appendRow(gotoItem);
        gotoItem->setData(scriptUrl, Qt::UserRole + 1);
        gotoItem->setData(startLine, Qt::UserRole + 2);
        gotoItem->setData(startCharacter, Qt::UserRole + 3);
        gotoItem->setData(endLine, Qt::UserRole + 4);
        gotoItem->setData(endCharacter, Qt::UserRole + 5);
        row++;
    }
    if (m_gotoModel->rowCount() > 0) {
        m_gotoListView->setCurrentIndex(m_gotoModel->index(0, 0));
        // calc height
        const int rowHeight = m_gotoListView->sizeHintForRow(0);
        const int rowCount = m_gotoModel->rowCount();
        const int totalHeight = qMin(300, rowHeight * rowCount);
        show();
        gotoRequest();
        move(m_gotoSession["position"].toPoint());
        QTimer::singleShot(0, this, [this, totalHeight] {
            m_gotoListView->setFixedHeight(totalHeight);
            adjustSize();
        });
    }
}

void GotoWidget::gotoShowTypeDefinition(const QVariantMap &gotoSession, const QJsonArray &typeDefinitions) {
    m_gotoSession = gotoSession;
    int row = 0;
    for (const auto &value: typeDefinitions) {
        const QJsonObject definition = value.toObject();
        QString uri = definition["uri"].toString();
        uri = QUrl::fromPercentEncoding(uri.toUtf8());
        if (QChar &drive = uri[8]; drive.isLetter() && drive.isLower()) { drive = drive.toUpper(); }
        const QUrl scriptUrl(uri);
        const QJsonObject rangeObject = definition["range"].toObject();
        const QJsonObject startObject = rangeObject["start"].toObject();
        const QJsonObject endObject = rangeObject["end"].toObject();
        const int startLine = startObject["line"].toInt();
        const int startCharacter = startObject["character"].toInt();
        const int endLine = endObject["line"].toInt();
        const int endCharacter = endObject["character"].toInt();
        auto *gotoItem = new QStandardItem(QIcon(":/icon/typeDefinition.svg"), scriptUrl.fileName()); // NOLINT
        m_gotoModel->appendRow(gotoItem);
        gotoItem->setData(scriptUrl, Qt::UserRole + 1);
        gotoItem->setData(startLine, Qt::UserRole + 2);
        gotoItem->setData(startCharacter, Qt::UserRole + 3);
        gotoItem->setData(endLine, Qt::UserRole + 4);
        gotoItem->setData(endCharacter, Qt::UserRole + 5);
        row++;
    }
    if (m_gotoModel->rowCount() > 0) {
        m_gotoListView->setCurrentIndex(m_gotoModel->index(0, 0));
        // calc height
        const int rowHeight = m_gotoListView->sizeHintForRow(0);
        const int rowCount = m_gotoModel->rowCount();
        const int totalHeight = qMin(300, rowHeight * rowCount);
        show();
        gotoRequest();
        move(m_gotoSession["position"].toPoint());
        QTimer::singleShot(0, this, [this, totalHeight] {
            m_gotoListView->setFixedHeight(totalHeight);
            adjustSize();
        });
    }
}

void GotoWidget::gotoHide() {
    hide();
}

void GotoWidget::gotoPrev() {
    const QModelIndex currentIndex = m_gotoListView->currentIndex();
    if (!currentIndex.isValid() || currentIndex.row() == 0) return;
    const QModelIndex prevIndex = m_gotoModel->index(currentIndex.row() - 1, 0);
    m_gotoListView->setCurrentIndex(prevIndex);
    gotoRequest();
}

void GotoWidget::gotoNext() {
    const QModelIndex currentIndex = m_gotoListView->currentIndex();
    if (!currentIndex.isValid() || currentIndex.row() == m_gotoModel->rowCount() - 1) return;
    const QModelIndex nextIndex = m_gotoModel->index(currentIndex.row() + 1, 0);
    m_gotoListView->setCurrentIndex(nextIndex);
    gotoRequest();
}

void GotoWidget::gotoResponse(const QString &hint) const {
    const QModelIndex index = m_gotoListView->currentIndex();
    if (!index.isValid()) {
        m_gotoLabel->hide();
        return;
    }
    const int indexFrom = m_gotoModel->data(index, Qt::UserRole + 3).toInt();
    const int indexTo = m_gotoModel->data(index, Qt::UserRole + 5).toInt();
    const auto hintText = QString("<span style='white-space: pre;'>%1<span style='color: orange;'>%2</span>%3</span>").arg(
        hint.left(indexFrom).toHtmlEscaped(),
        hint.mid(indexFrom, indexTo - indexFrom).toHtmlEscaped(),
        hint.mid(indexTo).toHtmlEscaped());
    m_gotoLabel->setText(hintText);
    m_gotoLabel->show();
    const int y = m_gotoListView->visualRect(index).top();
    QTimer::singleShot(0, this, [this, y] {
        m_gotoLabel->adjustSize();
        m_gotoLabel->move(mapToGlobal(QPoint(width(), y)));
    });
}

// GotoWidget protected
void GotoWidget::hideEvent(QHideEvent *event) {
    m_gotoModel->clear();
    m_gotoLabel->hide();
    QWidget::hideEvent(event);
}

void GotoWidget::leaveEvent(QEvent *event) {
    gotoHide();
    QWidget::leaveEvent(event);
}

// GotoWidget private
void GotoWidget::gotoJump(const QModelIndex &index) {
    const QUrl scriptUrl = m_gotoModel->data(index, Qt::UserRole + 1).toUrl();
    const int lineFrom = m_gotoModel->data(index, Qt::UserRole + 2).toInt();
    const int indexFrom = m_gotoModel->data(index, Qt::UserRole + 3).toInt();
    const int lineTo = m_gotoModel->data(index, Qt::UserRole + 4).toInt();
    const int indexTo = m_gotoModel->data(index, Qt::UserRole + 5).toInt();
    emit insertIndicator(scriptUrl, INDICATOR_SELECTION, lineFrom, indexFrom, lineTo, indexTo, 1000);
    emit setCursorPosition(scriptUrl, lineFrom, indexFrom);
    gotoHide();
}

void GotoWidget::gotoRequest() {
    const QModelIndex index = m_gotoListView->currentIndex();
    if (!index.isValid()) {
        m_gotoLabel->hide();
        return;
    }
    const QUrl scriptUrl = m_gotoModel->data(index, Qt::UserRole + 1).toUrl();
    const int lineFrom = m_gotoModel->data(index, Qt::UserRole + 2).toInt();
    emit getText(scriptUrl, lineFrom, -1, -1, -1);
}
