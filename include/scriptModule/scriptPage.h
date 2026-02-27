#ifndef UNICOMM_SCRIPTPAGE_H
#define UNICOMM_SCRIPTPAGE_H

#include <QJsonArray>
#include <QJsonObject>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QFileSystemWatcher;
class QLabel;
class QLineEdit;
class QPushButton;

class ScintillaWidget;
class SearchWidget;
class EditorWidget;

class ScriptPage final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit ScriptPage(const QJsonObject &scriptConfig = QJsonObject(), const QUrl &scriptUrl = QUrl());

    ~ScriptPage() override = default;

    void pathDisambiguation();

    void scriptReload();

    void scriptSave();

    void scriptClose();

    void diagnosticsResponse(const QJsonArray &diagnostics);

    void documentHighlightResponse(const QJsonArray &result) const;

    void foldingRangeResponse(const QJsonArray &result) const;

    void formattingResponse(const QString &newText) const;

    void onTypeFormattingResponse(const QJsonObject &newText) const;

    void semanticTokensResponse(const QJsonArray &data) const;

    void spellCheckResponse(const QVariantList &typos);

    EditorWidget *m_editorWidget{};
    QUrl m_scriptUrl{};

public slots:
    void charAdded(int ch);

    void scriptPosition(int r, int c);

signals:
    void appendLog(const QString &message, const QString &level);

    void closeScript(const QUrl &scriptUrl);

    void positionScript(const QVariantHash &positionSession);

    void showMenu(const QUrl &scriptUrl, const QVariantHash &menuSession);

    void insertMarker(const QUrl &scriptUrl, int type, int line, int time);

    void removeMarker(const QUrl &scriptUrl, int type, int line);

    void insertBreakpoint(const QUrl &scriptUrl, int line, const QVariantHash &session);

    void removeBreakpoint(const QUrl &scriptUrl, int line);

    void requestCompletion(const QUrl &scriptUrl, int line, int character);

    void requestDefinition(const QUrl &scriptUrl, int line, int character);

    void requestDocumentHighlight(const QUrl &scriptUrl, int line, int character);

    void requestDocumentSymbol(const QUrl &scriptUrl);

    void requestFoldingRange(const QUrl &scriptUrl);

    void requestFormatting(const QUrl &scriptUrl);

    void requestHover(const QUrl &scriptUrl, int line, int character);

    void requestImplementation(const QUrl &scriptUrl, int line, int character);
    
    void requestOnTypeFormatting(const QUrl &scriptUrl, int line, int character);

    void requestReferences(const QUrl &scriptUrl, int line, int character);

    void requestSemanticTokens(const QUrl &scriptUrl);

    void requestSignatureHelp(const QUrl &scriptUrl, int line, int character);

    void requestSpellCheck(const QUrl &scriptUrl, const QString &script);

    void requestTypeDefinition(const QUrl &scriptUrl, int line, int character);
    
    void notificationJson(const QString &method, const QJsonObject &params);

    void showDiagnostic(const QVariantHash &diagnosticSession, const QString &message);

    void hideDwell();

    void showPositionWidget();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void marginClick(int margin, int line, Qt::KeyboardModifiers state);

private:
    void scriptReadonly(bool status);

    void scriptModify(bool status);

    void permissionRequest();

    void idleRequest();

    void didOpenNotification();

    void didChangeNotification();

    void didSaveNotification();

    void didCloseNotification();

    void completionRequest();

    void definitionRequest();

    void documentHighlightRequest();

    void documentSymbolRequest();

    void foldingRangeRequest();

    void formattingRequest();

    void hoverRequest();

    void implementationRequest();

    void referencesRequest();

    void onTypeFormattingRequest();

    void semanticTokensRequest();

    void signatureHelpRequest();

    void spellCheckRequest();

    void typeDefinitionRequest();

    void positionFill(int x, int y) const;

    QFileSystemWatcher *m_fileWatcher{};
    ScintillaWidget *m_scintillaWidget{};
    SearchWidget *m_searchWidget{};
    bool m_readonly = false;
    bool m_modified = false;
    QByteArray m_scriptHash{};
    QJsonArray m_scriptDiagnostic{};
    QVariantList m_scriptTypo{};
    int m_version = 1;
    QSet<QChar> m_completionTrigger{};
    QSet<QChar> m_signatureHelpTrigger{};
    QSet<QChar> m_onTypeFormattingTrigger{};

    // semantic enum
    enum {
        TOKENTYPE_NAMESPACE,
        TOKENTYPE_TYPE,
        TOKENTYPE_CLASS,
        TOKENTYPE_ENUM,
        TOKENTYPE_INTERFACE,
        TOKENTYPE_STRUCT,
        TOKENTYPE_TYPEPARAMETER,
        TOKENTYPE_PARAMETER,
        TOKENTYPE_VARIABLE,
        TOKENTYPE_PROPERTY,
        TOKENTYPE_ENUMMEMBAER,
        TOKENTYPE_EVENT,
        TOKENTYPE_FUNCTION,
        TOKENTYPE_METHOD,
        TOKENTYPE_MACRO,
        TOKENTYPE_KEYWORD,
        TOKENTYPE_MODIFIER,
        TOKENTYPE_COMMENT,
        TOKENTYPE_STRING,
        TOKENTYPE_NUMBER,
        TOKENTYPE_REGEXP,
        TOKENTYPE_OPERATOR,
        TOKENTYOE_DECORATOR,
    };

    enum {
        TOKENMODIFIERS_DECLARATION = 1 << 0,
        TOKENMODIFIERS_DEFINITION = 1 << 1,
        TOKENMODIFIERS_READONLY = 1 << 2,
        TOKENMODIFIERS_STATIC = 1 << 3,
        TOKENMODIFIERS_DEPRECATED = 1 << 4,
        TOKENMODIFIERS_ABSTRACT = 1 << 5,
        TOKENMODIFIERS_ASYNC = 1 << 6,
        TOKENMODIFIERS_MODIFICATION = 1 << 7,
        TOKENMODIFIERS_DOCUMENTATION = 1 << 8,
        TOKENMODIFIERS_DEFAULTLIBRARY = 1 << 9,
        TOKENMODIFIERS_GLOBAL = 1 << 10,
    };
};

#endif //UNICOMM_SCRIPTPAGE_H
