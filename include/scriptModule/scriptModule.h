#ifndef UNICOMM_SCRIPT_H
#define UNICOMM_SCRIPT_H

#include <QJsonObject>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

#include "welcomePage.h"

class QLabel;
class QTableWidget;
class QTabWidget;
class QTextBrowser;

class ScriptPage;
class ScriptEditor;
class CompletionTooltip;
class HoverTooltip;
class SignatureHelpTooltip;

class ScriptModule final : public QObject {
    Q_OBJECT

public:
    explicit ScriptModule();

    ~ScriptModule() override = default;

    KDDockWidgets::QtWidgets::DockWidget *welcomePage() const {
        return m_welcomePage;
    }

    void workspaceOpen(const QUrl &rootUrl);

    void scriptConfigSave();

    void scriptOpen(const QUrl &scriptUrl);

    void cursorPositionSet(const QUrl &scriptUrl, int startLine, int startCharacter);

    void cursorPositionGet() const;

    void indicatorShow(const QUrl &scriptUrl, int startLine, int startCharacter, int endLine, int endCharacter, int time);

    void markerShow(const QUrl &scriptUrl, int type, int line = -1, int time = -1);

    void diagnosticsReturn(const QUrl &scriptUrl, const QJsonArray &diagnosticsArray);

    void completionReturn(const QUrl &scriptUrl, const QJsonArray &items) const;

    void definitionReturn(const QUrl &scriptUrl, const QJsonArray &definitions);

    void foldingRangeReturn(const QUrl &scriptUrl, const QJsonArray &result) const;

    void formattingReturn(const QUrl &scriptUrl, const QString &newText) const;

    void hoverReturn(const QUrl &scriptUrl, const QString &message) const;

    void semanticTokensReturn(const QUrl &scriptUrl, const QJsonArray &data) const;

    void signatureHelpReturn(const QUrl &scriptUrl, const QJsonObject &signature) const;

    ScriptPage *m_focusedPage{};
signals:
    void appendLog(const QString &message, const QString &level);

    void openWorkspace();

    void focusScript(const QUrl &scriptUrl);

    void insertBreakpoint(const QUrl &scriptUrl, int line);

    void removeBreakpoint(const QUrl &scriptUrl, int line);

    void requestJson(const QString &method, const QJsonObject &params);

    void notificationJson(const QString &method, const QJsonObject &params);

private:
    void scriptFocus(ScriptPage *scriptPage, bool status);

    static void scriptModify(ScriptPage *scriptPage, bool status);

    void scriptClose(const QUrl &scriptUrl);

    void textReplace(QString &text, const QString &kind) const;

    QJsonObject m_scriptConfig{};
    QUrl m_rootUrl{};
    WelcomePage *m_welcomePage{};
    QHash<QUrl, QJsonArray> m_diagnosticsHash{};
    QHash<QUrl, ScriptPage *> m_scriptPageHash{};
    CompletionTooltip *m_completionTooltip{};
    HoverTooltip *m_hoverTooltip{};
    SignatureHelpTooltip *m_signatureHelpTooltip{};
};


// class TooltipPosition final : public QWidget {
//     Q_OBJECT
//
// public:
//     explicit TooltipPosition(QWidget *parent = nullptr);
//
//     ~TooltipPosition() override = default;
//
//     void showTooltip();
//
//     void hideTooltip();
//
// signals:
//     void fillPosition(int x, int y);
//
// protected:
//     bool eventFilter(QObject *obj, QEvent *event) override;
//
// private:
//     QTimer *m_timer = nullptr;
//     QLabel *m_label = nullptr;
// };
//


#endif //UNICOMM_SCRIPT_H
