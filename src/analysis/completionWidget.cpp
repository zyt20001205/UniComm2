#include "analysis/completionWidget.h"

#include <QHeaderView>
#include <QJsonArray>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>

#include "globals.h"

// public
CompletionWidget::CompletionWidget(QWidget *parent)
    : QObject(parent),
      m_placeholderSet({
          "\"__PLACEHOLDER__PORTNAME__\"",
          "\"__PLACEHOLDER__DATABASEKEY__\"",
          "\"__PLACEHOLDER__DATATABLEKEY__\"",
          "\"__PLACEHOLDER__PASSWORD__\""
      }),
      m_completionModel(new QStandardItemModel(this)),
      m_detailModel(new QStandardItemModel(this)) {
}

void CompletionWidget::propertySet(const QVariantMap &objects) {
    m_tooltip = qvariant_cast<QObject *>(objects["documentModuleCompletionToolTip"]);
    m_tooltip->setProperty("completionWidget", QVariant::fromValue(this));
    m_tableView = qvariant_cast<QObject *>(objects["documentModuleCompletionTableView"]);
    m_tableView->setProperty("model", QVariant::fromValue(m_completionModel));
    const auto detailTableView = qvariant_cast<QObject *>(objects["documentModuleCompletionDetailTableView"]);
    detailTableView->setProperty("model", QVariant::fromValue(m_detailModel));
}

void CompletionWidget::fontSet(const QString &family, const int pointSize) const {
    if (!m_tooltip) return;
    auto font = m_tooltip->property("font").value<QFont>();
    font.setFamily(family);
    font.setPointSize(pointSize);
    m_tooltip->setProperty("font", font);
}

bool CompletionWidget::isVisible() const {
    if (!m_tooltip) return false;
    return m_tooltip->property("visible").toBool();
}

void CompletionWidget::completionShow(const QVariantHash &completionSession, const QJsonArray &items) {
    m_tooltip->setProperty("typed", QVariant::fromValue(completionSession["typed"]));
    m_completionSession = completionSession;
    m_completionModel->clear();
    int completionMode = COMPLETION_MODE_FULL;
    for (const auto &value: items) {
        QJsonObject item = value.toObject();
        const QString label = item["label"].toString();
        const QString insertText = item["insertText"].toString(label);
        if (insertText == "_ENV") {
            completionMode = COMPLETION_MODE_SIMPLE;
            break;
        }
    }
    QString recordInsertText{};
    for (const auto &value: items) {
        QJsonObject item = value.toObject();
        const int kind = item["kind"].toInt();
        if (completionMode == COMPLETION_MODE_SIMPLE && kind != COMPLETION_KIND_ENUMMEMBER) continue;
        const QString label = item["label"].toString();
        const QString insertText = item["insertText"].toString(label);
        // placeholder check
        if (m_placeholderSet.contains(insertText)) {
            placeholderExpand(insertText);
            continue;
        }
        // duplicate item check
        if (insertText != recordInsertText) {
            recordInsertText = insertText;
            auto *standardItem = new QStandardItem(insertText); // NOLINT
            QUrl iconSource{};
            switch (kind) {
                case COMPLETION_KIND_TEXT: {
                    iconSource = "qrc:/icon/symbolString.svg";
                }
                break;
                case COMPLETION_KIND_METHOD:
                case COMPLETION_KIND_FUNCTION: {
                    iconSource = "qrc:/icon/symbolMethod.svg";
                }
                break;
                case COMPLETION_KIND_FIELD: {
                    iconSource = "qrc:/icon/symbolField.svg";
                }
                break;
                case COMPLETION_KIND_VARIABLE: {
                    iconSource = "qrc:/icon/symbolVariable.svg";
                }
                break;
                case COMPLETION_KIND_ENUM: {
                    iconSource = "qrc:/icon/symbolEnum.svg";
                }
                break;
                case COMPLETION_KIND_KEYWORD: {
                    iconSource = "qrc:/icon/symbolKeyword.svg";
                }
                break;
                case COMPLETION_KIND_SNIPPET: {
                    iconSource = "qrc:/icon/symbolSnippet.svg";
                }
                break;
                case COMPLETION_KIND_FILE: {
                    iconSource = "qrc:/icon/symbolFile.svg";
                }
                break;
                case COMPLETION_KIND_ENUMMEMBER: {
                    iconSource = "qrc:/icon/symbolEnumMember.svg";
                }
                break;
                case COMPLETION_KIND_EVENT: {
                    iconSource = "qrc:/icon/symbolEvent.svg";
                }
                break;
                default: {
                    iconSource = "qrc:/icon/symbolMisc.svg";
                    emit appendLog(LOG_WARNING, QString("contact author: unsupported completion(kind:%1, text:%2)").arg(QString::number(kind), insertText), "");
                }
                break;
            }
            standardItem->setData(iconSource, Qt::DecorationRole);
            standardItem->setData(QStringList({label}), Qt::WhatsThisRole);
            standardItem->setData(kind, Qt::UserRole + 1);
            m_completionModel->appendRow(standardItem);
        } else {
            auto labelList = m_completionModel->item(m_completionModel->rowCount() - 1, 0)->data(Qt::WhatsThisRole).toStringList();
            labelList.append(label);
            m_completionModel->item(m_completionModel->rowCount() - 1, 0)->setData(labelList, Qt::WhatsThisRole);
        }
    }
    if (m_completionModel->rowCount() > 0) {
        const auto position = completionSession["position"].toPoint();
        m_tooltip->setProperty("position", position);
        m_tableView->setProperty("selectedRow", 0);
        QMetaObject::invokeMethod(m_tooltip, "open");
    } else {
        completionHide();
    }
}

void CompletionWidget::completionHide() const {
    QMetaObject::invokeMethod(m_tooltip, "close");
    m_completionModel->clear();
    m_detailModel->clear();
}

void CompletionWidget::completionPrev() const {
    QMetaObject::invokeMethod(m_tableView, "completionPrev");
}

void CompletionWidget::completionNext() const {
    QMetaObject::invokeMethod(m_tableView, "completionNext");
}

void CompletionWidget::detailReload(const int index) const {
    m_detailModel->clear();
    if (!m_completionModel->item(index, 0)) return;
    const auto labelList = m_completionModel->item(index, 0)->data(Qt::WhatsThisRole).toStringList();
    for (const auto &label: labelList) {
        m_detailModel->appendRow(new QStandardItem(label));
    }
}

void CompletionWidget::textReplace() {
    const int index = m_tableView->property("selectedRow").toInt();
    const int kind = m_completionModel->item(index, 0)->data(Qt::UserRole + 1).toInt();
    QString insertText = m_completionModel->item(index, 0)->text();
    if (kind == COMPLETION_KIND_FUNCTION) {
        insertText += "()";
    } else if (kind == COMPLETION_KIND_FIELD) {
        insertText += ".";
    } else if (kind == COMPLETION_KIND_ENUMMEMBER) {
        if (insertText == "\"Get Position\"") {
            const QVariantMap gotoSession = {
                {"documentUrl", m_completionSession["documentUrl"].toUrl()},
                {"line", m_completionSession["startLine"].toInt()},
                {"character", m_completionSession["startCharacter"].toInt()}
            };
            emit showPosition(gotoSession);
            return;
        }
        insertText.replace("\\", "\\\\");
    }
    emit setText(
        m_completionSession["documentUrl"].toUrl(),
        insertText,
        m_completionSession["startLine"].toInt(),
        m_completionSession["startCharacter"].toInt(),
        m_completionSession["endLine"].toInt(),
        m_completionSession["endCharacter"].toInt());
    int cursorPosition = 0;
    if (kind == COMPLETION_KIND_FUNCTION) {
        cursorPosition = m_completionSession["startCharacter"].toInt() + insertText.size() - 1;
    } else {
        cursorPosition = m_completionSession["startCharacter"].toInt() + insertText.size();
    }
    emit setIndex(
        m_completionSession["documentUrl"].toUrl(),
        m_completionSession["startLine"].toInt(),
        cursorPosition);
    if (kind == COMPLETION_KIND_FUNCTION) {
        emit addChar(m_completionSession["documentUrl"].toUrl(), '(');
    } else if (kind == COMPLETION_KIND_FIELD) {
        emit addChar(m_completionSession["documentUrl"].toUrl(), '.');
    }
    completionHide();
}

void CompletionWidget::placeholderExpand(const QString &placeholder) const {
    if (placeholder == "\"__PLACEHOLDER__PORTNAME__\"") {
        for (int i = 0; i < g_portStandardItemModel->rowCount(); ++i) {
            const QString insertText = "\"" + g_portStandardItemModel->item(i, 0)->text() + "\"";
            auto *standardItem = new QStandardItem(insertText); // NOLINT
            standardItem->setData("qrc:/icon/symbolEnumMember.svg", Qt::DecorationRole);
            standardItem->setData(insertText, Qt::WhatsThisRole);
            standardItem->setData(COMPLETION_KIND_ENUMMEMBER, Qt::UserRole + 1);
            m_completionModel->appendRow(standardItem);
        }
    } else if (placeholder == "\"__PLACEHOLDER__DATABASEKEY__\"") {
        for (int i = 0; i < g_databaseStandardItemModel->rowCount(); ++i) {
            const QString insertText = "\"" + g_databaseStandardItemModel->item(i, 0)->text() + "\"";
            auto *standardItem = new QStandardItem(insertText); // NOLINT
            standardItem->setData("qrc:/icon/symbolEnumMember.svg", Qt::DecorationRole);
            standardItem->setData(insertText, Qt::WhatsThisRole);
            standardItem->setData(COMPLETION_KIND_ENUMMEMBER, Qt::UserRole + 1);
            m_completionModel->appendRow(standardItem);
        }
    } else if (placeholder == "\"__PLACEHOLDER__DATATABLEKEY__\"") {
        for (int i = 0; i < g_datatableHeaderItemModel->rowCount(); ++i) {
            const QString insertText = "\"" + g_datatableHeaderItemModel->item(i, 0)->text() + "\"";
            auto *standardItem = new QStandardItem(insertText); // NOLINT
            standardItem->setData("qrc:/icon/symbolEnumMember.svg", Qt::DecorationRole);
            standardItem->setData(insertText, Qt::WhatsThisRole);
            standardItem->setData(COMPLETION_KIND_ENUMMEMBER, Qt::UserRole + 1);
            m_completionModel->appendRow(standardItem);
        }
    } else if (placeholder == "\"__PLACEHOLDER__PASSWORD__\"") {
    }
}
