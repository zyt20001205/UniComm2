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
class PositionTooltip;
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

    void scriptFontReload(const QJsonObject &fontConfigScript) const;

    void scriptFontSave(const QJsonObject &fontConfigScript);

    void scriptIndicatorReload(const QJsonObject &indicatorConfigScript) const;

    void scriptIndicatorSave(const QJsonObject &indicatorConfigScript);

    void scriptOpen(const QUrl &scriptUrl);

    void cursorPositionSet(const QUrl &scriptUrl, int startLine, int startCharacter);

    void cursorPositionGet() const;

    void indicatorInsert(const QUrl &scriptUrl, int type, int lineFrom, int indexFrom, int lineTo, int indexTo, int time = -1);

    void indicatorRemove(const QUrl &scriptUrl, int type, int lineFrom = -1, int indexFrom = -1, int lineTo = -1, int indexTo = -1);

    void markerInsert(const QUrl &scriptUrl, int type, int line, int time = -1);

    void markerRemove(const QUrl &scriptUrl, int type, int line = -1);

    void annotationInsert(const QUrl &scriptUrl, int line, const QString &annotation);

    void annotationRemove(const QUrl &scriptUrl, int line = -1);

    void diagnosticsNotification(const QUrl &scriptUrl, const QJsonArray &diagnosticsArray);

    void completionResponse(const QUrl &scriptUrl, const QJsonArray &items) const;

    void definitionResponse(const QUrl &scriptUrl, const QJsonArray &definitions);

    void foldingRangeResponse(const QUrl &scriptUrl, const QJsonArray &result) const;

    void formattingResponse(const QUrl &scriptUrl, const QString &newText) const;

    void hoverResponse(const QUrl &scriptUrl, const QString &message) const;

    void semanticTokensResponse(const QUrl &scriptUrl, const QJsonArray &data) const;

    void signatureHelpResponse(const QUrl &scriptUrl, const QJsonObject &signature) const;

    ScriptPage *m_focusedPage{};
    QHash<QUrl, ScriptPage *> m_scriptPageHash{};
signals:
    void appendLog(const QString &message, const QString &level);

    void openWorkspace();

    void openScript(const QUrl &scriptUrl);

    void closeScript(const QUrl &scriptUrl);

    void focusScript(const QUrl &scriptUrl);

    void insertPort(int index, const QJsonObject &portConfig);

    void insertDatabase(int index, const QString &key);

    void insertDatatable(int index, const QString &key);

    void insertBreakpoint(const QUrl &scriptUrl, int line);

    void removeBreakpoint(const QUrl &scriptUrl, int line);

    void requestJson(const QString &method, const QJsonObject &params);

    void notificationJson(const QString &method, const QJsonObject &params);

private:
    void scriptFocus(ScriptPage *scriptPage, bool status);

    void scriptClose(const QUrl &scriptUrl);

    void textReplace(QString &text, const QString &kind) const;

    QJsonObject m_scriptConfig{};
    QUrl m_rootUrl{};
    WelcomePage *m_welcomePage{};
    QHash<QUrl, QJsonArray> m_diagnosticsHash{};
    CompletionTooltip *m_completionTooltip{};
    HoverTooltip *m_hoverTooltip{};
    PositionTooltip *m_positionTooltip{};
    SignatureHelpTooltip *m_signatureHelpTooltip{};
};

#endif //UNICOMM_SCRIPT_H
