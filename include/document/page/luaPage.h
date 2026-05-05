#ifndef UNICOMM_LUAPAGE_H
#define UNICOMM_LUAPAGE_H

#include <QJsonArray>
#include <ScintillaTypes.h>

#include "basePage.h"

class QLabel;
class QLineEdit;
class QPushButton;

class SearchWidget;
class ReplaceWidget;
class ScintillaWidget;
class SymbolWidget;

class LuaPage final : public BasePage {
    Q_OBJECT

public:
    explicit LuaPage(const QJsonObject &documentConfig = QJsonObject(), const QUrl &documentUrl = QUrl());

    ~LuaPage() override = default;

    void propertySet(const QVariantMap &objects);

    void themeLoad(int theme) const;

    [[nodiscard]] QVariantHash menuGet(const QString &name) const;

    void menuRequest(const QString &request);

    // public: file
    void documentSave();

    void permissionGet() override;

    // public: lsp
    void diagnosticsNotification(const QJsonArray &diagnostics);

    void documentHighlightResponse(const QJsonArray &result) const;

    void documentSymbolResponse(const QJsonArray &result);

    void foldingRangeResponse(const QJsonArray &result) const;

    void formattingResponse(const QString &newText) const;

    void onTypeFormattingResponse(const QJsonObject &newText) const;

    void rangeFormattingResponse(const QString &newText) const;

    void semanticTokensResponse(const QJsonArray &data);

    // public: typo
    void spellCheckResponse(const QVariantList &typos);

    void charAdd(int ch);

    void assemblyToggle(bool status);

    bool eventFilter(QObject *watched, QEvent *event) override;

    ScintillaWidget *m_editorWidget{};

signals:
    void startThread(const QUrl &documentUrl, int mode, int startLine, int startCharacter, int endLine, int endCharacter);

    void changeSelection(const QHash<QString, int> &selection);

    void insertBreakpoint(const QUrl &documentUrl, int line, const QVariantHash &session);

    void removeBreakpoint(const QUrl &documentUrl, int line);

    void requestCompletion(const QUrl &documentUrl, int line, int character);

    void requestDefinition(const QUrl &documentUrl, int line, int character);

    void requestDocumentHighlight(const QUrl &documentUrl, int line, int character);

    void requestDocumentSymbol(const QUrl &documentUrl);

    void requestFoldingRange(const QUrl &documentUrl);

    void requestFormatting(const QUrl &documentUrl);

    void requestHover(const QUrl &documentUrl, int line, int character);

    void requestImplementation(const QUrl &documentUrl, int line, int character);

    void requestOnTypeFormatting(const QUrl &documentUrl, int line, int character);

    void requestReferences(const QUrl &documentUrl, int line, int character);

    void requestSemanticTokens(const QUrl &documentUrl);

    void requestSignatureHelp(const QUrl &documentUrl, int line, int character);

    void requestSpellCheck(const QUrl &documentUrl, const QString &script);

    void requestTypeDefinition(const QUrl &documentUrl, int line, int character);

    void notificationJson(const QString &method, const QJsonObject &params);

    void showDiagnostic(const QVariantHash &diagnosticSession, const QString &message);

protected:
    // void documentClose() override;

private:
    void marginClick(Scintilla::Position position, int mouseButton, Scintilla::KeyMod modifiers, int margin);

    void selectionChange();

    void contentChange();

    void dwellChange();

    void savepointChange(bool status);

    // private: file
    void permissionSet() const;

    void breakpointGet() const;

    void breakpointSet(int line) const;

    void regionGet() const;

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

    [[nodiscard]] bool navigable(Scintilla::Position position) const;

    void navigationToggle(Scintilla::Position position = -1) const;

    void symbolPair(QChar character);

    // private: search
    void searchToggle();

    void replaceToggle();

    void searchRequest(const QString &text);

    void searchResponse();

    void searchPrev();

    void searchNext();

    void searchClear();

    void textReplace(const QString &text);

    void allReplace(const QString &text);

    SearchWidget *m_searchWidget{};
    ReplaceWidget *m_replaceWidget{};
    SymbolWidget * m_symbolWidget{};
    ScintillaWidget *m_assemblyWidget{};

    QTimer *m_selectionTimer{};
    QTimer *m_contentTimer{};
    QTimer *m_dwellTimer{};

    QObject *m_global{};
    QObject *m_toolTip{};
    QObject *m_breakpointEditDialog{};
    QObject *m_systemPropertyDialog{};
    QObject *m_saveDialog{};
    QObject *m_editorMenu{};

    QSet<QChar> m_completionSet{};
    QSet<QChar> m_signatureHelpSet{};
    QSet<QChar> m_onTypeFormattingSet{};
    QHash<QChar, QChar> m_pairHash{};

    int m_version = 1;
    QJsonArray m_diagnostic{};
    QJsonArray m_symbol{};
    QVariantList m_typo{};
    QHash<QString, int> m_selection{};
    QVariantHash m_search{};
    QHash<int, int> m_l2aHash{};
};

#endif //UNICOMM_LUAPAGE_H
