#include "scriptModule/scriptModule.h"

#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QShortcut>
#include <QTableWidget>
#include <QTextBrowser>

#include "configModule.h"
#include "globals.h"
#include "luaModule/luaControl.h"
#include "utils.h"
#include "portModule/portModule.h"
#include "scriptModule/scriptPage.h"
#include "scriptModule/welcomePage.h"

// ScriptModule public
ScriptModule::ScriptModule()
    : m_scriptConfig(g_config["scriptConfig"].toObject()),
      m_welcomePage(new WelcomePage()) {
    // clear invalid script url
    QJsonArray validScriptList;
    for (const auto &value: m_scriptConfig["scriptList"].toArray()) {
        if (const auto scriptUrl = QUrl(value.toString()); QFileInfo::exists(scriptUrl.toLocalFile())) {
            validScriptList.append(value);
        }
    }
    m_scriptConfig["scriptList"] = validScriptList;
    m_welcomePage->setObjectName("welcomePage");
    connect(m_welcomePage, &WelcomePage::openWorkspace, this, &ScriptModule::openWorkspace);
}

void ScriptModule::workspaceOpen(const QUrl &rootUrl) {
    m_rootUrl = rootUrl;
    m_diagnosticsHash.clear();
}

void ScriptModule::scriptLoad() {
    for (const auto &value: m_scriptConfig["scriptList"].toArray()) {
        scriptOpen(QUrl(value.toString()));
    }
}

void ScriptModule::scriptConfigSave() {
    // save config
    auto scriptList = QJsonArray();
    for (const QUrl &url: m_scriptPageHash.keys()) {
        if (ScriptPage *scriptPage = m_scriptPageHash[url]; scriptPage->isVisible()) {
            scriptPage->scriptSave();
            scriptList.append(url.toString());
        }
    }
    m_scriptConfig["scriptList"] = scriptList;
    g_config["scriptConfig"] = m_scriptConfig;
}

void ScriptModule::scriptOpen(const QUrl &scriptUrl) {
    // check if tab exists
    if (!m_scriptPageHash.contains(scriptUrl)) {
        // create script page
        auto *scriptPage = new ScriptPage(m_scriptConfig, scriptUrl);
        scriptPage->setObjectName(scriptUrl.toString());
        m_scriptPageHash[scriptUrl] = scriptPage;
        connect(scriptPage, &KDDockWidgets::QtWidgets::DockWidget::isOpenChanged, this, [scriptPage] { scriptClose(scriptPage); });
        connect(scriptPage, &KDDockWidgets::QtWidgets::DockWidget::isFocusedChanged, this, [this, scriptPage](const bool status) {
            if (status) scriptFocus(scriptPage);
        });
        connect(scriptPage, &ScriptPage::modifyScript, this, [scriptPage](const bool status) { scriptModify(scriptPage, status); });
        connect(scriptPage, &ScriptPage::insertBreakpoint, this, &ScriptModule::insertBreakpoint);
        connect(scriptPage, &ScriptPage::removeBreakpoint, this, &ScriptModule::removeBreakpoint);
        connect(scriptPage, &ScriptPage::requestJson, this, &ScriptModule::requestJson);
        connect(scriptPage, &ScriptPage::notificationJson, this, &ScriptModule::notificationJson);
        if (m_focusedPage == nullptr) {
            m_welcomePage->addDockWidgetAsTab(scriptPage);
            m_welcomePage->close();
        } else {
            m_focusedPage->addDockWidgetAsTab(scriptPage);
        }
        m_focusedPage = scriptPage;
        scriptPage->diagnosticsReturn(m_diagnosticsHash[scriptUrl]);
    } else {
        m_scriptPageHash[scriptUrl]->show();
    }
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, scriptUrl.toString(), "opened");
}

void ScriptModule::cursorPositionSet(const QUrl &scriptUrl, const int startLine, const int startCharacter) {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_scriptEditor->setCursorPosition(startLine, startCharacter);
}

void ScriptModule::cursorPositionGet() const {
    // if (const auto scriptPage = static_cast<ScriptPage *>(m_scriptTabWidget->currentWidget())) {
    //     const QUrl scriptUrl = scriptPage->m_scriptUrl;
    //     int line, index;
    //     scriptPage->m_scriptEditor->getCursorPosition(&line, &index);
    //     g_cursorPosition = {
    //         {"url", scriptUrl},
    //         {"line", line + 1},
    //         {"character", index}
    //     };
    // }
}

void ScriptModule::indicatorShow(const QUrl &scriptUrl, const int startLine, const int startCharacter, const int endLine, const int endCharacter, const int time) {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_scriptEditor->fillIndicatorRange(startLine, startCharacter, endLine, endCharacter, INDICATOR_HIGHLIGHT);
    QTimer::singleShot(time, [scriptPage, startLine, startCharacter, endLine, endCharacter] {
        scriptPage->m_scriptEditor->clearIndicatorRange(startLine, startCharacter, endLine, endCharacter, INDICATOR_HIGHLIGHT);
    });
}

void ScriptModule::markerShow(const QUrl &scriptUrl, const int type, const int line, const int time) const {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    if (line == -1) {
        scriptPage->m_scriptEditor->markerDeleteAll(type);
        return;
    }
    scriptPage->m_scriptEditor->markerAdd(line - 1, type);
    if (time == -1) return;
    QTimer::singleShot(time, [scriptPage, line, type] {
        scriptPage->m_scriptEditor->markerDelete(line - 1, type);
    });
}

void ScriptModule::diagnosticsReturn(const QUrl &scriptUrl, const QJsonArray &diagnosticsArray) {
    m_diagnosticsHash.insert(scriptUrl, diagnosticsArray);
    if (m_scriptPageHash.contains(scriptUrl)) {
        m_scriptPageHash[scriptUrl]->diagnosticsReturn(diagnosticsArray);
    }
}

void ScriptModule::completionReturn(const QUrl &scriptUrl, const QJsonArray &items) const {
    m_scriptPageHash[scriptUrl]->completionReturn(items);
}

void ScriptModule::foldingRangeReturn(const QUrl &scriptUrl, const QJsonArray &result) const {
    m_scriptPageHash[scriptUrl]->foldingRangeReturn(result);
}

void ScriptModule::formattingReturn(const QUrl &scriptUrl, const QString &newText) const {
    m_scriptPageHash[scriptUrl]->formattingReturn(newText);
}

void ScriptModule::hoverReturn(const QUrl &scriptUrl, const QString &message) const {
    m_scriptPageHash[scriptUrl]->hoverReturn(message);
}

void ScriptModule::semanticTokensReturn(const QUrl &scriptUrl, const QJsonArray &data) const {
    m_scriptPageHash[scriptUrl]->semanticTokensReturn(data);
}

void ScriptModule::signatureHelpReturn(const QUrl &scriptUrl, const QJsonObject &signature) const {
    m_scriptPageHash[scriptUrl]->signatureHelpReturn(signature);
}

// ScriptModule private
void ScriptModule::scriptFocus(ScriptPage *scriptPage) {
    m_focusedPage = scriptPage;
    emit switchScript(scriptPage->m_scriptUrl);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, scriptPage->m_scriptUrl.toString(), "focused");
}

void ScriptModule::scriptModify(ScriptPage *scriptPage, const bool status) {
    const QString pageName = scriptPage->title();
    if (status) {
        scriptPage->setTitle(pageName + "*");
    } else {
        scriptPage->setTitle(pageName.chopped(1));
    }
}

void ScriptModule::scriptClose(ScriptPage *scriptPage) {
    // ask for saving
    if (scriptPage->m_modified) {
        const QMessageBox::StandardButton reply =
                QMessageBox::question(nullptr, tr("Close Script"), tr("The script has been edited. Save changes?"), QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            scriptPage->scriptSave();
        }
    }
}

// // TooltipCompletion public
// TooltipCompletion::TooltipCompletion(QWidget *parent) : QWidget(parent), m_tableWidget(new QTableWidget(this)) {
//     setWindowFlags(Qt::ToolTip);
//     auto *layout = new QVBoxLayout(this); //NOLINT
//     layout->setContentsMargins(0, 0, 0, 0);
//     layout->addWidget(m_tableWidget);
//     m_tableWidget->setFixedWidth(600);
//     m_tableWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
//     m_tableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
//     m_tableWidget->setFont(QFont("Consolas", 12));
//     m_tableWidget->setShowGrid(false);
//     m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
//     m_tableWidget->setColumnCount(3);
//     m_tableWidget->horizontalHeader()->setVisible(false);
//     m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
//     m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
//     m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
//     m_tableWidget->verticalHeader()->setVisible(false);
//     m_kindList = {
//         "0", "Text", "Method", "Function", "Constructor", "Field", "Variable", "Class", "Interface", "Module", "Property", "Unit", "Value", "Enum", "Keyword", "Snippet", "Color",
//         "File", "Reference", "Folder", "EnumMember", "Constant", "Struct", "Event", "Operator", "TypeParameter"
//     };
// }
//
// void TooltipCompletion::showTooltip(const QJsonArray &items) {
//     m_tableWidget->setRowCount(0);
//     int row = 0;
//     for (const QJsonValue &value: items) {
//         QJsonObject item = value.toObject();
//         const QString kind = m_kindList[item["kind"].toInt()];
//         const QString label = item["label"].toString();
//         const QString insertText = item["insertText"].toString(label);
//         m_tableWidget->insertRow(row);
//         auto *insertTextItem = new QTableWidgetItem(insertText); // NOLINT
//         auto *kindItem = new QTableWidgetItem(kind); // NOLINT
//         auto *labelItem = new QTableWidgetItem(label); // NOLINT
//         m_tableWidget->setItem(row, 0, insertTextItem);
//         m_tableWidget->setItem(row, 1, kindItem);
//         m_tableWidget->setItem(row, 2, labelItem);
//         row++;
//     }
//     if (m_tableWidget->rowCount() > 0) {
//         m_currentRow = 0;
//         m_tableWidget->selectRow(m_currentRow);
//         m_insertText = m_tableWidget->item(m_currentRow, 0)->text();
//         m_kind = m_tableWidget->item(m_currentRow, 1)->text();
//     } else {
//         m_currentRow = -1;
//         m_kind.clear();
//         m_insertText.clear();
//     }
//     m_tableWidget->resizeRowsToContents();
//     this->adjustSize();
//     this->show();
// }
//
// void TooltipCompletion::hideTooltip() {
//     this->hide();
// }
//
// // TooltipCompletion protected
// bool TooltipCompletion::eventFilter(QObject *obj, QEvent *event) {
//     if (event->type() == QEvent::KeyPress && this->isVisible()) {
//         auto *keyEvent = static_cast<QKeyEvent *>(event);
//         switch (keyEvent->key()) {
//             case Qt::Key_Tab:
//                 if (!m_insertText.isEmpty()) emit replaceText(m_insertText, m_kind);
//                 return true;
//             case Qt::Key_Return:
//                 if (!m_insertText.isEmpty()) emit insertText(m_insertText, m_kind);
//                 return true;
//             case Qt::Key_Escape:
//                 hideTooltip();
//                 return true;
//             case Qt::Key_Up:
//                 moveUp();
//                 return true;
//             case Qt::Key_Down:
//                 moveDown();
//                 return true;
//             case Qt::Key_Left:
//                 return true;
//             case Qt::Key_Right:
//                 return true;
//             default:
//                 return false;
//         }
//     }
//     return QWidget::eventFilter(obj, event);
// }
//
// // TooltipCompletion private
// void TooltipCompletion::moveUp() {
//     if (m_currentRow == -1) return;
//     if (m_currentRow > 0) {
//         m_currentRow--;
//         m_tableWidget->selectRow(m_currentRow);
//         m_insertText = m_tableWidget->item(m_currentRow, 0)->text();
//         m_kind = m_tableWidget->item(m_currentRow, 1)->text();
//     }
// }
//
// void TooltipCompletion::moveDown() {
//     if (m_currentRow == -1) return;
//     if (m_currentRow < m_tableWidget->rowCount() - 1) {
//         m_currentRow++;
//         m_tableWidget->selectRow(m_currentRow);
//         m_insertText = m_tableWidget->item(m_currentRow, 0)->text();
//         m_kind = m_tableWidget->item(m_currentRow, 1)->text();
//     }
// }
//
// // TooltipHover public
// TooltipHover::TooltipHover(QWidget *parent)
//     : QWidget(parent),
//       m_textBrowser(new QTextBrowser(this)) {
//     setWindowFlags(Qt::ToolTip);
//     auto *layout = new QVBoxLayout(this); //NOLINT
//     layout->setContentsMargins(0, 0, 0, 0);
//     layout->addWidget(m_textBrowser);
//     m_textBrowser->setFixedWidth(600);
//     m_textBrowser->setFont(QFont("Consolas", 10));
//     m_textBrowser->setOpenExternalLinks(true);
//     m_textBrowser->installEventFilter(this);
// }
//
// // TooltipHover protected
// bool TooltipHover::eventFilter(QObject *obj, QEvent *event) {
//     if (event->type() == QEvent::Leave) {
//         hideTooltip();
//         return true;
//     }
//     return QWidget::eventFilter(obj, event);
// }
//
// // TooltipHover private
// void TooltipHover::showTooltip(const QString &message) {
//     emit switchDwell(false);
//     m_textBrowser->setMarkdown(message);
//     this->adjustSize();
//     this->move(QCursor::pos() + QPoint(15, 15));
//     this->show();
// }
//
// void TooltipHover::hideTooltip() {
//     emit switchDwell(true);
//     this->hide();
// }
//
// // TooltipPosition public
// TooltipPosition::TooltipPosition(QWidget *parent)
//     : QWidget(parent),
//       m_timer(new QTimer(this)),
//       m_label(new QLabel(this)) {
//     qApp->installEventFilter(this);
//     setWindowFlags(Qt::Popup);
//     auto *layout = new QVBoxLayout(this); //NOLINT
//     layout->setContentsMargins(0, 0, 0, 0);
//     layout->addWidget(m_label);
//     m_label->setFont(QFont("consolas", 12));
//     m_timer->setInterval(30);
//     connect(m_timer, &QTimer::timeout, [this] {
//         const QPoint logicalPos = QCursor::pos();
//         this->move(logicalPos + QPoint(15, 15));
//         POINT physicalPos;
//         GetCursorPos(&physicalPos);
//         m_label->setText(QString("X: %1, Y: %2").arg(QString::number(physicalPos.x), QString::number(physicalPos.y)));
//     });
// }
//
// void TooltipPosition::showTooltip() {
//     this->show();
//     m_timer->start();
// }
//
// void TooltipPosition::hideTooltip() {
//     this->hide();
//     m_timer->stop();
// }
//
// // TooltipPosition protected
// bool TooltipPosition::eventFilter(QObject *obj, QEvent *event) {
//     if (event->type() == QEvent::MouseButtonPress && this->isVisible()) {
//         const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
//         if (mouseEvent->button() == Qt::LeftButton) {
//             POINT physicalPos;
//             GetCursorPos(&physicalPos);
//             emit fillPosition(physicalPos.x, physicalPos.y);
//             hideTooltip();
//         }
//     }
//     return QWidget::eventFilter(obj, event);
// }
//
// // TooltipSignatureHelp public
// TooltipSignatureHelp::TooltipSignatureHelp(QWidget *parent) : QWidget(parent),
//                                                               m_label(new QLabel(this)) {
//     setWindowFlags(Qt::ToolTip);
//     auto *layout = new QVBoxLayout(this); //NOLINT
//     layout->setContentsMargins(0, 0, 0, 0);
//     layout->addWidget(m_label);
//     m_label->setFont(QFont("consolas", 12));
//     m_label->setStyleSheet("QLabel{background-color: white; border: 1px solid #d0d0d0;}");
// }
//
// void TooltipSignatureHelp::showTooltip(const QJsonObject &signature) {
//     QString helpText;
//     int index = 0;
//     const int activeParameter = signature["activeParameter"].toInt();
//     const QString label = signature["label"].toString();
//     const QJsonArray parameters = signature["parameters"].toArray();
//     for (const QJsonValue &value: parameters) {
//         const QJsonObject parameter = value.toObject();
//         const QJsonArray range = parameter["label"].toArray();
//         const int startIndex = range[0].toInt();
//         const int endIndex = range[1].toInt();
//         QString param = label.mid(startIndex, endIndex - startIndex);
//         if (index == activeParameter) {
//             param = QString("<span style='color: orange;'>%1</span>").arg(param);
//         }
//         helpText += param;
//         helpText += ", ";
//         index++;
//     }
//     helpText.chop(2);
//     m_label->setText(helpText);
//     this->show();
// }
//
// void TooltipSignatureHelp::hideTooltip() {
//     this->hide();
// }
//
// // TooltipSignatureHelp protected
// bool TooltipSignatureHelp::eventFilter(QObject *obj, QEvent *event) {
//     if (event->type() == QEvent::KeyPress && this->isVisible()) {
//         auto *keyEvent = static_cast<QKeyEvent *>(event);
//         switch (keyEvent->key()) {
//             case Qt::Key_Escape:
//                 hideTooltip();
//                 return true;
//             default:
//                 return false;
//         }
//     }
//     return QWidget::eventFilter(obj, event);
// }
