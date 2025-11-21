#ifndef UNICOMM_SCRIPTPAGE_H
#define UNICOMM_SCRIPTPAGE_H

#include <QJsonArray>
#include <QJsonObject>
#include <Qsci/qsciscintilla.h>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QFileSystemWatcher;
class QLabel;
class QLineEdit;
class QPushButton;

class SearchWidget;
class ScriptEditor;

class ScriptPage final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit ScriptPage(const QJsonObject &scriptConfig = QJsonObject(), const QUrl &scriptUrl = QUrl());

    ~ScriptPage() override = default;

    void pathDisambiguation();

    void scriptReload();

    void scriptSave();

    void scriptClose();

    void diagnosticsResponse(const QJsonArray &diagnosticsArray);

    void documentHighlightResponse(const QJsonArray &result) const;

    void foldingRangeResponse(const QJsonArray &result) const;

    void formattingResponse(const QString &newText) const;

    void onTypeFormattingResponse(const QJsonObject &newText) const;

    void semanticTokensResponse(const QJsonArray &data) const;

    void textReplace(QString &text, const QString &kind);

    ScriptEditor *m_scriptEditor{};
    QUrl m_scriptUrl{};

signals:
    void appendLog(const QString &message, const QString &level);

    void closeScript(const QUrl &scriptUrl);

    void insertPort();

    void insertDatabase();

    void insertDatatable();

    void insertMarker(const QUrl &scriptUrl, int type, int line, int time);

    void removeMarker(const QUrl &scriptUrl, int type, int line);

    void insertBreakpoint(const QUrl &scriptUrl, int line);

    void removeBreakpoint(const QUrl &scriptUrl, int line);

    void requestCompletion(const QUrl &scriptUrl, int line, int character);

    void requestDefinition(const QUrl &scriptUrl, int line, int character);

    void requestDocumentHighlight(const QUrl &scriptUrl, int line, int character);

    void requestDocumentSymbol(const QUrl &scriptUrl);

    void requestFoldingRange(const QUrl &scriptUrl);

    void requestFormatting(const QUrl &scriptUrl);

    void requestHover(const QUrl &scriptUrl, int line, int character);

    void requestOnTypeFormatting(const QUrl &scriptUrl, int line, int character);

    void requestReferences(const QUrl &scriptUrl, int line, int character);

    void requestSemanticTokens(const QUrl &scriptUrl);

    void requestSignatureHelp(const QUrl &scriptUrl, int line, int character);

    void notificationJson(const QString &method, const QJsonObject &params);

    void fullCompletionTooltip(bool status);

    void showHoverTooltip(const QString &message);

    void hideHoverTooltip();

    void leaveHoverTooltip();

    void showPositionTooltip();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void charAdded(int ch);

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

    void referencesRequest();

    void onTypeFormattingRequest();

    void semanticTokensRequest();

    void signatureHelpRequest();

    void positionFill(int x, int y) const;

    QFileSystemWatcher *m_fileWatcher{};
    SearchWidget *m_searchWidget{};
    bool m_readonly = false;
    bool m_modified = false;
    QByteArray m_scriptHash{};
    QJsonArray m_scriptDiagnostic{};
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

class SearchWidget final : public QWidget {
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = nullptr);

    ~SearchWidget() override = default;

    void toggle();

    void statSet(int current, int total) const;

signals:
    void searchText(const QString &text, int flag);

    void searchPrev();

    void searchNext();

    void replaceText(const QString &text);

    void replaceAllText(const QString &text);

private:
    QLineEdit *m_searchLineEdit{};
    int m_searchFlag = 0;
    QPushButton *m_wholeWordButton{};
    QPushButton *m_matchCaseButton{};
    QPushButton *m_wordStartButton{};
    QPushButton *m_regExpButton{};
    QLabel *m_statLabel{};
    QPushButton *m_prevButton{};
    QPushButton *m_nextButton{};
    QLineEdit *m_replaceLineEdit{};
    QPushButton *m_replaceButton{};
    QPushButton *m_replaceAllButton{};
};

class ScriptEditor final : public QsciScintilla {
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = nullptr);

    ~ScriptEditor() override = default;

    void textSearch(const QString &text, int flag);

    void prevSearch();

    void nextSearch();

    void textReplace(const QString &text);

    void textReplaceAll(const QString &text);

    void textReplace(const QString &text, int lineFrom, int indexFrom, int lineTo, int indexTo);

    void indicatorInsert(int type, int lineFrom, int indexFrom, int lineTo, int indexTo, int time = -1);

    void indicatorRemove(int type, int lineFrom = -1, int indexFrom = -1, int lineTo = -1, int indexTo = -1);

    void markerInsert(int type, int line, int time = -1);

    void markerRemove(int type, int line = -1);

signals:
    void dockRight();

    void dockLeft();

    void dockTop();

    void dockBottom();

    void openInExplorer();

    void openInApplication();

    void requestPermission();

    void requestIdle();

    void hideHoverTooltip();

    void leaveHoverTooltip();

    void requestDefinition();

    void requestDocumentHighlight();

    void requestFormatting();

    void requestHover();

    void requestOnTypeFormatting();

    void requestReferences();

    void setStat(int current, int total);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    void focusOutEvent(QFocusEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void pairHandle(int ascii);

    void typeHandle() const;

private:
    void commentHandle();

    void duplicateHandle();

    void dwellHandle();

    void searchHandle();

    QString m_searchText = {};
    int m_searchFlag = 0;
    QList<QList<int>> m_searchList{};
    int m_currentIndex = 0;
    QHash<QChar, QChar> m_autoPairHash{};
    QTimer *m_dwellTimer{};
    QTimer *m_typeTimer{};

    struct CurrentWord {
        long wordStart = -1;
        long wordEnd = -1;
    };
    CurrentWord m_currentWord{};
};

#endif //UNICOMM_SCRIPTPAGE_H
