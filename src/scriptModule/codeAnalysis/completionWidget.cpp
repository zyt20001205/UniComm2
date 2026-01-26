#include "scriptModule/codeAnalysis/completionWidget.h"

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
    : QObject(parent),
      m_placeholderSet({"\"_PORT_PLACEHOLDER_\"", "\"_DATABASE_PLACEHOLDER_\"", "\"_DATATABLE_PLACEHOLDER_\""}),
      m_completionModel(new QStandardItemModel(this)),
      m_detailModel(new QStandardItemModel(this)) {
}

void CompletionWidget::propertySet(const QVariantMap &objects) {
    m_tooltip = qvariant_cast<QObject *>(objects["scriptModuleCompletionToolTip"]);
    m_tooltip->setProperty("completionWidget", QVariant::fromValue(this));
    m_tableView = qvariant_cast<QObject *>(objects["scriptModuleCompletionTableView"]);
    m_tableView->setProperty("model", QVariant::fromValue(m_completionModel));
    const auto detailTableView = qvariant_cast<QObject *>(objects["scriptModuleCompletionDetailTableView"]);
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
                case COMPLETION_KIND_FILE: {
                    iconSource = "qrc:/icon/symbolFile.svg";
                }
                break;
                case COMPLETION_KIND_ENUMMEMBER: {
                    iconSource = "qrc:/icon/symbolEnumMember.svg";
                }
                break;
                default: {
                    iconSource = "qrc:/icon/symbolMisc.svg";
                    qDebug() << "WIP completion kind:" << kind << insertText;
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

void CompletionWidget::placeholderExpand(const QString &placeholder) const {
    if (placeholder == "\"_PORT_PLACEHOLDER_\"") {
        for (int i = 0; i < g_portStandardItemModel->rowCount(); ++i) {
            const QString insertText = "\"" + g_portStandardItemModel->item(i, 0)->text() + "\"";
            auto *standardItem = new QStandardItem(insertText); // NOLINT
            standardItem->setData("qrc:/icon/symbolEnumMember.svg", Qt::DecorationRole);
            standardItem->setData(insertText, Qt::WhatsThisRole);
            standardItem->setData(COMPLETION_KIND_ENUMMEMBER, Qt::UserRole + 1);
            m_completionModel->appendRow(standardItem);
        }
    } else if (placeholder == "\"_DATABASE_PLACEHOLDER_\"") {
        for (int i = 0; i < g_databaseStandardItemModel->rowCount(); ++i) {
            const QString insertText = "\"" + g_databaseStandardItemModel->item(i, 0)->text() + "\"";
            auto *standardItem = new QStandardItem(insertText); // NOLINT
            standardItem->setData("qrc:/icon/symbolEnumMember.svg", Qt::DecorationRole);
            standardItem->setData(insertText, Qt::WhatsThisRole);
            standardItem->setData(COMPLETION_KIND_ENUMMEMBER, Qt::UserRole + 1);
            m_completionModel->appendRow(standardItem);
        }
    } else if (placeholder == "\"_DATATABLE_PLACEHOLDER_\"") {
        for (int i = 0; i < g_datatableHeaderItemModel->rowCount(); ++i) {
            const QString insertText = "\"" + g_datatableHeaderItemModel->item(i, 0)->text() + "\"";
            auto *standardItem = new QStandardItem(insertText); // NOLINT
            standardItem->setData("qrc:/icon/symbolEnumMember.svg", Qt::DecorationRole);
            standardItem->setData(insertText, Qt::WhatsThisRole);
            standardItem->setData(COMPLETION_KIND_ENUMMEMBER, Qt::UserRole + 1);
            m_completionModel->appendRow(standardItem);
        }
    }
}
