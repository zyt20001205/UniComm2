#ifndef UNICOMM_SCRIPTPAGE_H
#define UNICOMM_SCRIPTPAGE_H

#include <QJsonObject>
#include <Qsci/qsciscintilla.h>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class ScriptEditor;

class ScriptPage final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit ScriptPage(const QJsonObject &scriptConfig = QJsonObject(), const QUrl &scriptUrl = QUrl());

    ~ScriptPage() override = default;

    void scriptSave();

    void scriptClose();

    void diagnosticsResponse(const QJsonArray &diagnosticsArray) const;

    void foldingRangeResponse(const QJsonArray &result) const;

    void formattingResponse(const QString &newText) const;

    void semanticTokensResponse(const QJsonArray &data) const;

    void textReplace(QString &text, const QString &kind);

    ScriptEditor *m_scriptEditor{};
    QUrl m_scriptUrl{};

signals:
    void appendLog(const QString &message, const QString &level);

    void modifyScript(bool status);

    void closeScript(const QUrl &scriptUrl);

    void insertPort(int index, const QJsonObject &portConfig);

    void insertDatabase(int index, const QString &key);

    void insertDatatable(int index, const QString &key);

    void showPositionTooltip();

    void insertMarker(const QUrl &scriptUrl, int type, int line, int time);

    void removeMarker(const QUrl &scriptUrl, int type, int line);

    void insertBreakpoint(const QUrl &scriptUrl, int line);

    void removeBreakpoint(const QUrl &scriptUrl, int line);

    void requestJson(const QString &method, const QJsonObject &params);

    void notificationJson(const QString &method, const QJsonObject &params);

    void setFullCompletion(bool status);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void scriptEdit() const;

    void charAdded(int ch);

    void dwellStart(int pos, int x, int y);

    void marginClick(int margin, int line, Qt::KeyboardModifiers state);

private:
    void scriptEditFinish();

    void didOpenNotification();

    void didChangeNotification();

    void didSaveNotification();

    void didCloseNotification();

    void completionRequest();

    void definitionRequest(int line, int character);

    void documentSymbolRequest();

    void foldingRangeRequest();

    void formattingRequest();

    void semanticTokensRequest();

    void signatureHelpRequest();

    void hoverRequest(int line, int character);

    void positionFill(int x, int y) const;

    bool m_modified = false;
    QTimer *m_editTimer{};
    QByteArray m_scriptHash{};
    int m_version = 1;

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

class ScriptEditor final : public QsciScintilla {
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = nullptr);

    ~ScriptEditor() override = default;

signals:
    void dockRight();

    void dockLeft();

    void dockTop();

    void dockBottom();

    void requestDefinition(int line, int character);

    void requestFormatting();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void pairHandle(int ascii);

private:
    void commentHandle();

    void duplicateHandle();

    void definitionHandle();

    QHash<QChar, QChar> m_autoPairHash{};
    bool m_ctrlPressed = false;
    bool m_jumpValid = false;
};

#endif //UNICOMM_SCRIPTPAGE_H
