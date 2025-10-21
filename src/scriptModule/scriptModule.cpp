#include "scriptModule/scriptModule.h"

#include <QFileInfo>
#include <QHBoxLayout>
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
    if (m_rootUrl.isEmpty()) {
        // post initialization after workspace opened
        for (const auto &value: m_scriptConfig["scriptList"].toArray()) {
            scriptOpen(QUrl(value.toString()));
        }
    } else {

        for (const auto &value: m_scriptConfig["scriptList"].toArray()) {
            scriptClose(QUrl(value.toString()));
        }
        m_diagnosticsHash.clear();
    }
    m_rootUrl = rootUrl;
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
        connect(scriptPage, &KDDockWidgets::QtWidgets::DockWidget::isFocusedChanged, this, [this, scriptPage](const bool status) {
            scriptFocus(scriptPage, status);
        });
        connect(scriptPage, &ScriptPage::modifyScript, this, [scriptPage](const bool status) { scriptModify(scriptPage, status); });
        connect(scriptPage, &ScriptPage::closeScript, this, &ScriptModule::scriptClose);
        connect(scriptPage, &ScriptPage::insertPort, this, &ScriptModule::insertPort);
        connect(scriptPage, &ScriptPage::insertDatabase, this, &ScriptModule::insertDatabase);
        connect(scriptPage, &ScriptPage::insertDatatable, this, &ScriptModule::insertDatatable);
        connect(scriptPage, &ScriptPage::insertBreakpoint, this, &ScriptModule::insertBreakpoint);
        connect(scriptPage, &ScriptPage::removeBreakpoint, this, &ScriptModule::removeBreakpoint);
        connect(scriptPage, &ScriptPage::requestJson, this, &ScriptModule::requestJson);
        connect(scriptPage, &ScriptPage::notificationJson, this, &ScriptModule::notificationJson);
        connect(scriptPage, &ScriptPage::setFullCompletion, m_completionTooltip, &CompletionTooltip::fullCompleteSet);
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
        m_scriptPageHash[scriptUrl]->show();
        m_scriptPageHash[scriptUrl]->raise();
    }
    // logging
    emit appendLog(QString("<a href='%1'>%2</a> opened").arg(scriptUrl.toString(), scriptUrl.fileName()), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 opened").arg(timestamp, scriptUrl.fileName());
}

void ScriptModule::cursorPositionSet(const QUrl &scriptUrl, const int startLine, const int startCharacter) {
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_scriptEditor->setCursorPosition(startLine, startCharacter);
    scriptPage->m_scriptEditor->setFocus();
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
    if (!m_scriptPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
    scriptPage->m_scriptEditor->fillIndicatorRange(startLine, startCharacter, endLine, endCharacter, INDICATOR_HIGHLIGHT);
    QTimer::singleShot(time, [scriptPage, startLine, startCharacter, endLine, endCharacter] {
        scriptPage->m_scriptEditor->clearIndicatorRange(startLine, startCharacter, endLine, endCharacter, INDICATOR_HIGHLIGHT);
    });
}

void ScriptModule::markerShow(const QUrl &scriptUrl, const int type, const int line, const int time) {
    if (line == -1) {
        if (!m_scriptPageHash.contains(scriptUrl)) return;
        const auto *scriptPage = m_scriptPageHash[scriptUrl];
        scriptPage->m_scriptEditor->markerDeleteAll(type);
        return;
    }
    if (!m_scriptPageHash.contains(scriptUrl)) scriptOpen(scriptUrl);
    const auto *scriptPage = m_scriptPageHash[scriptUrl];
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

void ScriptModule::definitionReturn(const QUrl &scriptUrl, const QJsonArray &definitions) {
    if (definitions.size() != 1) {
        qDebug() << "multiple definitions WIP";
        return;
    }
    for (const auto &value: definitions) {
        const QJsonObject definition = value.toObject();
        // open definition script
        QString uri = definition["uri"].toString();
        uri = QUrl::fromPercentEncoding(uri.toUtf8());
        if (QChar &drive = uri[8]; drive.isLetter() && drive.isLower()) { drive = drive.toUpper(); }
        const QUrl definitionUrl(uri);
        scriptOpen(definitionUrl);
        // show indicator
        const QJsonObject rangeObject = definition["range"].toObject();
        const QJsonObject startObject = rangeObject["start"].toObject();
        const QJsonObject endObject = rangeObject["end"].toObject();
        const int startLine = startObject["line"].toInt();
        const int startCharacter = startObject["character"].toInt();
        const int endLine = endObject["line"].toInt();
        const int endCharacter = endObject["character"].toInt();
        indicatorShow(definitionUrl, startLine, startCharacter, endLine, endCharacter, 1000);
        // set cursor
        cursorPositionSet(definitionUrl, startLine, startCharacter);
    }
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
        // QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        // qDebug() << QString("[%1] %2 %3").arg(timestamp, scriptPage->m_scriptUrl.toString(), "focused");
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

void ScriptModule::scriptClose(const QUrl &scriptUrl) {
    m_scriptPageHash.remove(scriptUrl);
    if (m_scriptPageHash.isEmpty()) {
        m_welcomePage->open();
        m_focusedPage = nullptr;
    } else {
        m_focusedPage = m_scriptPageHash.values()[0];
    }
    // logging
    emit appendLog(QString("<a href='%1'>%2</a> closed").arg(scriptUrl.toString(), scriptUrl.fileName()), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 closed").arg(timestamp, scriptUrl.fileName());
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
