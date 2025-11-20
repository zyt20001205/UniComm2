#include "scriptModule/gotoPopup.h"

#include <QHeaderView>
#include <QJsonArray>
#include <QTableWidget>
#include <QVBoxLayout>

#include "globals.h"

// GotoPopup public
GotoPopup::GotoPopup(QWidget *parent)
    : QWidget(parent, Qt::Popup),
      m_tableWidget(new QTableWidget(this)),
      m_gotoColor{
          {DIAGNOSTIC, QColor("#e3f2fD")},
          {REFERENCES, QColor("#e8f5e9")}
      } {
    auto *layout = new QVBoxLayout(this); //NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_tableWidget);
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
    connect(m_tableWidget, &QTableWidget::cellClicked, this, [this](const int row) {
        popupGoto(row);
    });
}

void GotoPopup::popupShowDefinition(const QJsonArray &definitions) {
    int row = 0;
    for (const auto &value: definitions) {
        const QJsonObject definition = value.toObject();
        QString uri = definition["uri"].toString();
        uri = QUrl::fromPercentEncoding(uri.toUtf8());
        if (QChar &drive = uri[8]; drive.isLetter() && drive.isLower()) { drive = drive.toUpper(); }
        const QUrl definitionUrl(uri);
        const QJsonObject rangeObject = definition["range"].toObject();
        const QJsonObject startObject = rangeObject["start"].toObject();
        const QJsonObject endObject = rangeObject["end"].toObject();
        const int startLine = startObject["line"].toInt();
        const int startCharacter = startObject["character"].toInt();
        const int endLine = endObject["line"].toInt();
        const int endCharacter = endObject["character"].toInt();
        m_tableWidget->insertRow(row);
        auto *iconItem = new QTableWidgetItem(QIcon(":/icon/definition.svg"), ""); // NOLINT
        auto *fileItem = new QTableWidgetItem(definitionUrl.fileName()); // NOLINT
        auto *rangeItem = new QTableWidgetItem(QString("%1:%2 - %3:%4").arg(QString::number(startLine), QString::number(startCharacter), QString::number(endLine), QString::number(endCharacter))); // NOLINT
        iconItem->setData(Qt::UserRole + 1, definitionUrl);
        iconItem->setData(Qt::UserRole + 2, startLine);
        iconItem->setData(Qt::UserRole + 3, startCharacter);
        iconItem->setData(Qt::UserRole + 4, endLine);
        iconItem->setData(Qt::UserRole + 5, endCharacter);
        iconItem->setBackground(m_gotoColor[DIAGNOSTIC]);
        fileItem->setBackground(m_gotoColor[DIAGNOSTIC]);
        rangeItem->setBackground(m_gotoColor[DIAGNOSTIC]);
        m_tableWidget->setItem(row, 0, iconItem);
        m_tableWidget->setItem(row, 1, fileItem);
        m_tableWidget->setItem(row, 2, rangeItem);
        row++;
    }
    if (m_tableWidget->rowCount() > 0) {
        m_tableWidget->resizeColumnsToContents();
        m_tableWidget->resizeRowsToContents();
        adjustSize();
        show();
    }
}

void GotoPopup::popupShowReferences(const QJsonArray &references) {
    for (const auto &value: references) {
        const int row = m_tableWidget->rowCount();
        const QJsonObject reference = value.toObject();
        QString uri = reference["uri"].toString();
        uri = QUrl::fromPercentEncoding(uri.toUtf8());
        if (QChar &drive = uri[8]; drive.isLetter() && drive.isLower()) { drive = drive.toUpper(); }
        const QUrl referenceUrl(uri);
        const QJsonObject rangeObject = reference["range"].toObject();
        const QJsonObject startObject = rangeObject["start"].toObject();
        const QJsonObject endObject = rangeObject["end"].toObject();
        const int startLine = startObject["line"].toInt();
        const int startCharacter = startObject["character"].toInt();
        const int endLine = endObject["line"].toInt();
        const int endCharacter = endObject["character"].toInt();
        m_tableWidget->insertRow(row);
        auto *iconItem = new QTableWidgetItem(QIcon(":/icon/reference.svg"), ""); // NOLINT
        auto *fileItem = new QTableWidgetItem(referenceUrl.fileName()); // NOLINT
        auto *rangeItem = new QTableWidgetItem(QString("%1:%2 - %3:%4").arg(QString::number(startLine), QString::number(startCharacter), QString::number(endLine), QString::number(endCharacter))); // NOLINT
        iconItem->setData(Qt::UserRole + 1, referenceUrl);
        iconItem->setData(Qt::UserRole + 2, startLine);
        iconItem->setData(Qt::UserRole + 3, startCharacter);
        iconItem->setData(Qt::UserRole + 4, endLine);
        iconItem->setData(Qt::UserRole + 5, endCharacter);
        iconItem->setBackground(m_gotoColor[REFERENCES]);
        fileItem->setBackground(m_gotoColor[REFERENCES]);
        rangeItem->setBackground(m_gotoColor[REFERENCES]);
        m_tableWidget->setItem(row, 0, iconItem);
        m_tableWidget->setItem(row, 1, fileItem);
        m_tableWidget->setItem(row, 2, rangeItem);
    }
    if (m_tableWidget->rowCount() > 0) {
        m_tableWidget->resizeColumnsToContents();
        m_tableWidget->resizeRowsToContents();
        adjustSize();
        show();
    }
}

void GotoPopup::popupHide() {
    hide();
}

// GotoPopup protected
void GotoPopup::hideEvent(QHideEvent *event) {
    m_tableWidget->setRowCount(0);
    QWidget::hideEvent(event);
}

// GotoPopup private
void GotoPopup::popupGoto(const int row) {
    const QUrl scriptUrl = m_tableWidget->item(row, 0)->data(Qt::UserRole + 1).toUrl();
    const int startLine = m_tableWidget->item(row, 0)->data(Qt::UserRole + 2).toInt();
    const int startCharacter = m_tableWidget->item(row, 0)->data(Qt::UserRole + 3).toInt();
    const int endLine = m_tableWidget->item(row, 0)->data(Qt::UserRole + 4).toInt();
    const int endCharacter = m_tableWidget->item(row, 0)->data(Qt::UserRole + 5).toInt();
    emit insertIndicator(scriptUrl, INDICATOR_SELECTION, startLine, startCharacter, endLine, endCharacter, 1000);
    emit setCursorPosition(scriptUrl, startLine, startCharacter);
    popupHide();
}
