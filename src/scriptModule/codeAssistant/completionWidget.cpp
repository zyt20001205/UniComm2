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
    : QObject(parent),
      // m_completionListView(new QListView(this)),
      m_completionModel(new QStandardItemModel(this))
// m_filterProxyModel(new QSortFilterProxyModel(this)),
// m_filterWidget(new QWidget(this)),
// m_textButton(new QPushButton(this)),
// m_functionButton(new QPushButton(this)),
// m_fieldButton(new QPushButton(this)),
// m_variableButton(new QPushButton(this)),
// m_enumButton(new QPushButton(this)),
// m_keywordButton(new QPushButton(this)),
// m_fileButton(new QPushButton(this)),
// m_enummemberButton(new QPushButton(this)),
// m_resetButton(new QPushButton(this)),
// m_completionLabel(new QLabel(nullptr, Qt::ToolTip))
{
}

void CompletionWidget::propertySet(const QVariantMap &objects) {
    m_tooltip = qvariant_cast<QObject *>(objects["scriptModuleCompletionToolTip"]);
    m_tableView = qvariant_cast<QObject *>(objects["scriptModuleCompletionTableView"]);
    m_tableView->setProperty("model", QVariant::fromValue(m_completionModel));
    m_tableView->setProperty("completionWidget", QVariant::fromValue(this));
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
        auto *iconItem = new QStandardItem(); // NOLINT
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
        iconItem->setData(iconSource, Qt::DecorationRole);
        auto *textItem = new QStandardItem(insertText); // NOLINT
        textItem->setData(label, Qt::WhatsThisRole);
        textItem->setData(kind, Qt::UserRole + 1);
        m_completionModel->appendRow({iconItem, textItem});
        row++;
    }
    const auto position = completionSession["position"].toPoint();
    m_tooltip->setProperty("position", position);
    m_tableView->setProperty("selectedRow", 0);
    QMetaObject::invokeMethod(m_tooltip, "open"); {
        // for (const auto &value: items) {
        //     QJsonObject item = value.toObject();
        //     const int kind = item["kind"].toInt();
        //     m_completionKinds.insert(kind);
        //     const QString label = item["label"].toString();
        //     const QString insertText = item["insertText"].toString(label);
        //     if (insertText == "_ENV") completionMode = COMPLETION_MODE_SIMPLE;
        //     auto *completionItem = new QStandardItem(insertText); // NOLINT
        //     m_completionModel->appendRow(completionItem);
        //     completionItem->setData(kind, Qt::UserRole + 1);
        //     completionItem->setData(label, Qt::UserRole + 2);
        //     switch (kind) {
        //         case COMPLETION_KIND_TEXT: {
        //             completionItem->setIcon(QIcon(":/icon/symbolString.svg"));
        //         }
        //         break;
        //         case COMPLETION_KIND_METHOD: {
        //             completionItem->setIcon(QIcon(":/icon/symbolMethod.svg"));
        //         }
        //         break;
        //         case COMPLETION_KIND_FUNCTION: {
        //             completionItem->setIcon(QIcon(":/icon/symbolMethod.svg"));
        //         }
        //         break;
        //         case COMPLETION_KIND_FIELD: {
        //             completionItem->setIcon(QIcon(":/icon/symbolField.svg"));
        //         }
        //         break;
        //         case COMPLETION_KIND_VARIABLE: {
        //             completionItem->setIcon(QIcon(":/icon/symbolVariable.svg"));
        //         }
        //         break;
        //         case COMPLETION_KIND_ENUM: {
        //             completionItem->setIcon(QIcon(":/icon/symbolEnum.svg"));
        //         }
        //         break;
        //         case COMPLETION_KIND_KEYWORD: {
        //             completionItem->setIcon(QIcon(":/icon/symbolKeyword.svg"));
        //         }
        //         break;
        //         case COMPLETION_KIND_FILE: {
        //             completionItem->setIcon(QIcon(":/icon/symbolFile.svg"));
        //         }
        //         break;
        //         case COMPLETION_KIND_ENUMMEMBER: {
        //             completionItem->setIcon(QIcon(":/icon/symbolEnumMember.svg"));
        //         }
        //         break;
        //         default: {
        //             completionItem->setIcon(QIcon(":/icon/symbolMisc.svg"));
        //             qDebug() << "WIP completion kind:" << kind << insertText;
        //         }
        //         break;
        //     }
        //     row++;
        // }
        // if (m_completionModel->rowCount() > 0) {
        //     filterInit(completionMode);
        // }
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

void CompletionWidget::textReplace() {
    const int index = m_tableView->property("selectedRow").toInt();
    const int kind = m_completionModel->item(index, 1)->data(Qt::UserRole + 1).toInt();
    QString insertText = m_completionModel->item(index, 1)->text();
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

// CompletionWidget private
void CompletionWidget::filterClear() const {
    // m_textButton->setChecked(false);
    // m_functionButton->setChecked(false);
    // m_fieldButton->setChecked(false);
    // m_variableButton->setChecked(false);
    // m_enumButton->setChecked(false);
    // m_keywordButton->setChecked(false);
    // m_enummemberButton->setChecked(false);
}

void CompletionWidget::filterInit(const int mode) {
    // filterClear();
    // if (mode == COMPLETION_MODE_FULL) {
    //     for (auto it = m_filterButtonHash.begin(); it != m_filterButtonHash.end(); ++it) {
    //         if (m_completionKinds.contains(it.key())) {
    //             it.value()->setEnabled(true);
    //             it.value()->setChecked(true);
    //         } else {
    //             it.value()->setEnabled(false);
    //         }
    //     }
    // } else if (mode == COMPLETION_MODE_SIMPLE) {
    //     m_enummemberButton->setChecked(true);
    // }
    // filterSet(true);
}

void CompletionWidget::filterSet(const bool status) {
    // QString regExp{};
    // for (auto it = m_filterButtonHash.begin(); it != m_filterButtonHash.end(); ++it) {
    //     if (it.value()->isChecked()) {
    //         regExp += QString::number(it.key());
    //         regExp += "|";
    //     }
    // }
    // if (!regExp.isEmpty()) regExp.chop(1);
    // else regExp = "(?!.*)";
    // m_filterProxyModel->setFilterRegularExpression(regExp);
    // if (m_filterProxyModel->rowCount() > 0) {
    //     m_completionListView->setCurrentIndex(m_filterProxyModel->index(0, 0));
    //     // calc height
    //     const int rowHeight = m_completionListView->sizeHintForRow(0);
    //     const int rowCount = m_filterProxyModel->rowCount();
    //     const int totalHeight = qMin(300, rowHeight * rowCount);
    //     show();
    //     labelShow();
    //     move(m_completionSession["position"].toPoint());
    //     QTimer::singleShot(0, this, [this, totalHeight] {
    //         m_completionListView->setFixedHeight(totalHeight);
    //         adjustSize();
    //     });
    // } else {
    //     completionHide();
    // }
}

void CompletionWidget::labelShow() const {
}
