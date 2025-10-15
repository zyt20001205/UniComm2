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

    void diagnosticsReturn(const QJsonArray &diagnosticsArray) const;

    void foldingRangeReturn(const QJsonArray &result) const;

    void formattingReturn(const QString &newText) const;

    void semanticTokensReturn(const QJsonArray &data) const;

    void textReplace(QString &text, const QString &kind) ;

    ScriptEditor *m_scriptEditor{};
    QUrl m_scriptUrl{};
    bool m_modified = false;

signals:
    void modifyScript(bool status);

    void insertBreakpoint(const QUrl &scriptUrl, int line);

    void removeBreakpoint(const QUrl &scriptUrl, int line);

    void requestJson(const QString &method, const QJsonObject &params);

    void notificationJson(const QString &method, const QJsonObject &params);

private slots:
    void scriptEdit() const;

    void charAdded(int ch);

    void dwellStart(int pos, int x, int y);

    void marginClick(int margin, int line, Qt::KeyboardModifiers state);

private:
    void scriptEditFinish();

    void didOpenNotification();

    void didChangeNotification();

    void completionRequest();

    void documentSymbolRequest();

    void foldingRangeRequest();

    void formattingRequest();

    void semanticTokensRequest();

    void signatureHelpRequest();

    void hoverRequest(int line, int character);

    void positionFill(int x, int y) const;

    QTimer* m_editTimer{};
    QByteArray m_scriptHash{};
    int m_version = 1;

    // semantic related
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

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void autoPairHandle(int ascii);

private:
    void commentHandle();

    void duplicateHandle();

    QHash<QChar, QChar> m_autoPairHash{};
};

#endif //UNICOMM_SCRIPTPAGE_H