#include "scriptModule/scriptModule.h"

#include <QFileInfo>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QShortcut>
#include <QTableWidget>
#include <QTextBrowser>

#include "configModule.h"
#include "globals.h"
#include "luaModule/luaControl.h"
#include "portModule/portModule.h"
#include "scriptModule/completionTooltip.h"
#include "scriptModule/hoverTooltip.h"
#include "scriptModule/scriptPage.h"
#include "scriptModule/signatureHelpTooltip.h"
#include "scriptModule/welcomePage.h"

// ScriptModule public
ScriptModule::ScriptModule()
    : m_scriptConfig(g_config["scriptConfig"].toObject()),
      m_welcomePage(new WelcomePage()),
      m_completionTooltip(new CompletionTooltip(g_mainWindow)),
      m_hoverTooltip(new HoverTooltip(g_mainWindow)),
      m_signatureHelpTooltip(new SignatureHelpTooltip(g_mainWindow))
//       m_tooltipPosition(new TooltipPosition(this)),
//
{
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

    connect(m_completionTooltip, &CompletionTooltip::replaceText, this, &ScriptModule::textReplace);
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
        connect(scriptPage, &KDDockWidgets::QtWidgets::DockWidget::isOpenChanged, this, [this, scriptPage] { scriptClose(scriptPage); });
        connect(scriptPage, &KDDockWidgets::QtWidgets::DockWidget::isFocusedChanged, this, [this, scriptPage](const bool status) {
            scriptFocus(scriptPage, status);
        });
        connect(scriptPage, &ScriptPage::modifyScript, this, [scriptPage](const bool status) { scriptModify(scriptPage, status); });
        connect(scriptPage, &ScriptPage::insertBreakpoint, this, &ScriptModule::insertBreakpoint);
        connect(scriptPage, &ScriptPage::removeBreakpoint, this, &ScriptModule::removeBreakpoint);
        connect(scriptPage, &ScriptPage::requestJson, this, &ScriptModule::requestJson);
        connect(scriptPage, &ScriptPage::notificationJson, this, &ScriptModule::notificationJson);
        scriptPage->m_scriptEditor->installEventFilter(m_completionTooltip);
        scriptPage->m_scriptEditor->installEventFilter(m_signatureHelpTooltip);
        if (m_focusedPage == nullptr) {
            m_welcomePage->open();
            m_welcomePage->addDockWidgetAsTab(scriptPage);
            m_welcomePage->close();
        } else {
            m_focusedPage->addDockWidgetAsTab(scriptPage);
        }
        scriptFocus(scriptPage, true);
        scriptPage->diagnosticsReturn(m_diagnosticsHash[scriptUrl]);
    } else {
        m_focusedPage->addDockWidgetAsTab(m_scriptPageHash[scriptUrl]);
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
    const QUrl scriptUrl = m_focusedPage->m_scriptUrl;
    int line, index;
    m_focusedPage->m_scriptEditor->getCursorPosition(&line, &index);
    g_cursorPosition = {
        {"url", scriptUrl},
        {"line", line + 1},
        {"character", index}
    };
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
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    const auto *editor = static_cast<QsciScintilla *>(scriptPage->m_scriptEditor);
    const long currentPos = editor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    const long wordStartPos = editor->SendScintilla(QsciScintilla::SCI_WORDSTARTPOSITION, currentPos, true);
    const int x = editor->SendScintilla(QsciScintilla::SCI_POINTXFROMPOSITION, 0, wordStartPos);
    const int y = editor->SendScintilla(QsciScintilla::SCI_POINTYFROMPOSITION, 0, wordStartPos);
    const QPoint cursorGlobalPos = editor->mapToGlobal(QPoint(x, y));
    const int lineHeight = editor->SendScintilla(QsciScintilla::SCI_TEXTHEIGHT, 0);
    m_completionTooltip->showTooltip(items);
    m_completionTooltip->move(cursorGlobalPos.x() - 2, cursorGlobalPos.y() + lineHeight);
}

void ScriptModule::foldingRangeReturn(const QUrl &scriptUrl, const QJsonArray &result) const {
    m_scriptPageHash[scriptUrl]->foldingRangeReturn(result);
}

void ScriptModule::formattingReturn(const QUrl &scriptUrl, const QString &newText) const {
    m_scriptPageHash[scriptUrl]->formattingReturn(newText);
}

void ScriptModule::hoverReturn(const QUrl &scriptUrl, const QString &message) const {
    m_hoverTooltip->showTooltip(message);
}

void ScriptModule::semanticTokensReturn(const QUrl &scriptUrl, const QJsonArray &data) const {
    m_scriptPageHash[scriptUrl]->semanticTokensReturn(data);
}

void ScriptModule::signatureHelpReturn(const QUrl &scriptUrl, const QJsonObject &signature) const {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    const auto *editor = static_cast<QsciScintilla *>(scriptPage->m_scriptEditor);
    long currentPos = editor->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
    while (true) {
        const int prevChar = editor->SendScintilla(QsciScintilla::SCI_GETCHARAT, currentPos - 1);
        if (prevChar == '(') break;
        currentPos--;
    }
    const int x = editor->SendScintilla(QsciScintilla::SCI_POINTXFROMPOSITION, 0, currentPos);
    const int y = editor->SendScintilla(QsciScintilla::SCI_POINTYFROMPOSITION, 0, currentPos);
    const QPoint cursorGlobalPos = editor->mapToGlobal(QPoint(x, y));
    const int lineHeight = editor->SendScintilla(QsciScintilla::SCI_TEXTHEIGHT, 0);
    m_signatureHelpTooltip->showTooltip(signature);
    m_signatureHelpTooltip->move(cursorGlobalPos.x() - 2, cursorGlobalPos.y() - lineHeight);
}

// ScriptModule private
void ScriptModule::scriptFocus(ScriptPage *scriptPage, const bool status) {
    m_completionTooltip->hideTooltip();
    m_signatureHelpTooltip->hideTooltip();
    if (status) {
        m_focusedPage = scriptPage;
        emit focusScript(scriptPage->m_scriptUrl);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3").arg(timestamp, scriptPage->m_scriptUrl.toString(), "focused");
    }
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
        if (m_focusedPage == scriptPage) {
            m_focusedPage = nullptr;
        }
        if (reply == QMessageBox::Yes) {
            scriptPage->scriptSave();
        } else {
            m_scriptPageHash.remove(scriptPage->m_scriptUrl);
            scriptPage->deleteLater();
        }
    }
}

void ScriptModule::textReplace(QString &text, const QString &kind) const {
    m_focusedPage->textReplace(text, kind);
}


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
