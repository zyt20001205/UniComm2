#ifndef UNICOMM_SCRIPT_H
#define UNICOMM_SCRIPT_H

#include <QJsonObject>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

#include "welcomePage.h"

class QLabel;
class QTableWidget;
class QTabWidget;
class QTextBrowser;

class CodeAssistant;
class ScriptPage;
class EditorWidget;
class SignatureWidget;

class ScriptModule final : public QObject {
    Q_OBJECT

public:
    explicit ScriptModule(QWidget *parent = nullptr);

    ~ScriptModule() override;

    void propertySet(const QVariantMap &objects);

    KDDockWidgets::QtWidgets::DockWidget *welcomePage() const {
        return m_welcomePage;
    }

    void scriptConfigSave();

    void scriptFontReload(const QJsonObject &fontConfigScript) const;

    void scriptFontSave(const QJsonObject &fontConfigScript);

    void scriptIndicatorReload(const QJsonObject &indicatorConfigScript) const;

    void scriptIndicatorSave(const QJsonObject &indicatorConfigScript);

    void scriptMarkerReload(const QJsonObject &markerConfigScript) const;

    void scriptMarkerSave(const QJsonObject &markerConfigScript);

    void scriptOpen(const QUrl &scriptUrl);

    Q_INVOKABLE void collapseAll(const QUrl &scriptUrl);

    Q_INVOKABLE void expandAll(const QUrl &scriptUrl);

    void cursorPositionSet(const QUrl &scriptUrl, int startLine, int startCharacter);

    void cursorPositionGet() const;

    QString textGet(const QUrl &scriptUrl, int startLine = -1, int startCharacter = -1, int endLine = -1, int endCharacter = -1);

    void indicatorInsert(const QUrl &scriptUrl, int type, int lineFrom, int indexFrom, int lineTo, int indexTo, int time = -1);

    void indicatorRemove(const QUrl &scriptUrl, int type, int lineFrom = -1, int indexFrom = -1, int lineTo = -1, int indexTo = -1);

    void markerInsert(const QUrl &scriptUrl, int type, int line, int time = -1);

    void markerRemove(const QUrl &scriptUrl, int type, int line = -1);

    void annotationInsert(const QUrl &scriptUrl, int line, const QString &annotation);

    void annotationRemove(const QUrl &scriptUrl, int line = -1);

    void diagnosticsNotification(const QUrl &scriptUrl, const QJsonArray &diagnostics);

    void codeActionRequest(const QUrl &scriptUrl, int lineFrom, int indexFrom, int lineTo, int indexTo);

    void completionRequest(const QUrl &scriptUrl, int line, int character);

    void completionResponse(const QUrl &scriptUrl, const QJsonArray &items) const;

    Q_INVOKABLE void definitionRequest(const QUrl &scriptUrl, int line, int character);

    void definitionResponse(const QUrl &scriptUrl, const QJsonArray &definitions) const;

    void documentSymbolRequest(const QUrl &scriptUrl);

    void documentHighlightRequest(const QUrl &scriptUrl, int line, int character);

    void documentHighlightResponse(const QUrl &scriptUrl, const QJsonArray &result);

    void foldingRangeRequest(const QUrl &scriptUrl);

    void foldingRangeResponse(const QUrl &scriptUrl, const QJsonArray &result) const;

    Q_INVOKABLE void formattingRequest(const QUrl &scriptUrl);

    void formattingResponse(const QUrl &scriptUrl, const QString &newText) const;

    void hoverRequest(const QUrl &scriptUrl, int line, int character);

    void hoverResponse(const QUrl &scriptUrl, const QString &message) const;

    Q_INVOKABLE void implementationRequest(const QUrl &scriptUrl, int line, int character);

    void implementationResponse(const QUrl &scriptUrl, const QJsonArray &implementations) const;

    void onTypeFormattingRequest(const QUrl &scriptUrl, int line, int character);

    void onTypeFormattingResponse(const QUrl &scriptUrl, const QJsonObject &newText) const;

    Q_INVOKABLE void referencesRequest(const QUrl &scriptUrl, int line, int character);

    void referencesResponse(const QUrl &scriptUrl, const QJsonArray &references) const;

    void semanticTokensRequest(const QUrl &scriptUrl);

    void semanticTokensResponse(const QUrl &scriptUrl, const QJsonArray &data) const;

    void signatureHelpRequest(const QUrl &scriptUrl, int line, int character);

    void signatureHelpResponse(const QUrl &scriptUrl, const QJsonObject &signature) const;

    void spellCheckResponse(const QUrl &scriptUrl, const QVariantList &typos);

    Q_INVOKABLE void typeDefinitionRequest(const QUrl &scriptUrl, int line, int character);

    void typeDefinitionResponse(const QUrl &scriptUrl, const QJsonArray &typeDefinitions) const;

    ScriptPage *m_focusedPage{};
    QHash<QUrl, ScriptPage *> m_scriptPageHash{};
signals:
    void appendLog(const QString &message, const QString &level);

    void openWorkspace();

    void focusScript(const QUrl &scriptUrl);

    void changeSelection(const QHash<QString, int> &selection);

    void insertPort();

    void insertDatabase();

    void insertDatatable();

    void insertBreakpoint(const QUrl &scriptUrl, int line, const QVariantHash &session);

    void removeBreakpoint(const QUrl &scriptUrl, int line);

    void requestJson(const QString &method, const QJsonObject &params);

    void notificationJson(const QString &method, const QJsonObject &params);

    void responseCodeAction(const QUrl &scriptUrl, const QJsonArray &result);

    void requestSpellCheck(const QUrl &scriptUrl, const QString &script);

    void requestSpellSuggest(const QUrl &scriptUrl, const QString &word);

private:
    void scriptFocus(ScriptPage *scriptPage, bool status);

    void scriptClose(const QUrl &scriptUrl);

    void menuShow(const QUrl &scriptUrl, const QVariantHash &menuSession) const;

    void textInsert(const QUrl &scriptUrl, const QString &text, int line, int index);

    void textReplace(const QUrl &scriptUrl, const QString &text, int lineFrom, int indexFrom, int lineTo, int indexTo);

    void charAdd(const QUrl &scriptUrl, QChar character) const;

    QJsonObject m_scriptConfig{};
    QObject *m_editorMenu{};
    WelcomePage *m_welcomePage{};
    QHash<QUrl, QJsonArray> m_diagnosticsHash{};
    CodeAssistant *m_codeAssistant{};
};

#endif //UNICOMM_SCRIPT_H
