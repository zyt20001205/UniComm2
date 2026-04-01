#ifndef UNICOMM_SCRIPTPAGE_H
#define UNICOMM_SCRIPTPAGE_H

#include <QJsonArray>
#include <QJsonObject>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

#include "ScintillaTypes.h"


class QFileSystemWatcher;
class QLabel;
class QLineEdit;
class QPushButton;

class SearchWidget;
class ReplaceWidget;
class ScintillaWidget;
class SymbolWidget;

class ScriptPage final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit ScriptPage(const QJsonObject &scriptConfig = QJsonObject(), const QUrl &scriptUrl = QUrl());

    ~ScriptPage() override = default;

    // public: file
    void pathDisambiguation();

    void scriptReload();

    void scriptSave();

    void scriptClose();

    // public: lsp
    void diagnosticsResponse(const QJsonArray &diagnostics);

    void documentHighlightResponse(const QJsonArray &result) const;

    void documentSymbolResponse(const QJsonArray &result);

    void foldingRangeResponse(const QJsonArray &result) const;

    void formattingResponse(const QString &newText) const;

    void onTypeFormattingResponse(const QJsonObject &newText) const;

    void rangeFormattingResponse(const QString &newText) const;

    void semanticTokensResponse(const QJsonArray &data) const;

    // public: typo
    void spellCheckResponse(const QVariantList &typos);

    void charAdd(int ch);

    void assemblyToggle(bool status);

    bool eventFilter(QObject *watched, QEvent *event) override;

    QUrl m_scriptUrl{};
    QObject *m_toolTip{};
    ScintillaWidget *m_editorWidget{};

signals:
    void appendLog(const QString &message, const QString &level);

    void closeScript(const QUrl &scriptUrl);

    void startThread(const QUrl &scriptUrl, int mode, int startLine, int startCharacter, int endLine, int endCharacter);

    void changeSelection(const QHash<QString, int> &selection);

    void setPermission(const QUrl &scriptUrl, bool readonly);

    void editBreakpoint(const QUrl &scriptUrl, int line);

    void showMenu(const QUrl &scriptUrl, const QVariantHash &menuSession);

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

    void responseSearch(const QString &result);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void marginClick(Scintilla::Position position, int mouseButton, Scintilla::KeyMod modifiers, int margin);

    void selectionChange();

    void contentChange();

    void dwellChange();

    void savepointChange(bool status);

    // private: file
    void permissionLoad();

    void breakpointLoad() const;

    void regionLoad() const;

    // private: lsp
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

    void typeDefinitionRequest();

    // private: typo
    void spellCheckRequest();

    // private: misc
    void commentToggle();

    void navigationToggle(Scintilla::Position position = -1) const;

    void symbolPair(QChar character);

    void searchRequest(const QString &text);

    SearchWidget *m_searchWidget{};
    ReplaceWidget *m_replaceWidget{};
    SymbolWidget * m_symbolWidget{};
    ScintillaWidget *m_assemblyWidget{};

    QTimer *m_selectionTimer{};
    QTimer *m_contentTimer{};
    QTimer *m_dwellTimer{};

    QSet<QChar> m_completionSet{};
    QSet<QChar> m_signatureHelpSet{};
    QSet<QChar> m_onTypeFormattingSet{};
    QHash<QChar, QChar> m_pairHash{};

    QFileSystemWatcher *m_fileWatcher{};

    int m_version = 1;
    QJsonArray m_diagnostic{};
    QJsonArray m_symbol{};
    QVariantList m_typo{};
    QHash<QString, int> m_selection{};
    QVariantHash m_search{};
    QHash<int, int> m_l2aHash{};

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
