#ifndef UNICOMM_COMPLETIONPOPUP_H
#define UNICOMM_COMPLETIONPOPUP_H

#include <QObject>
#include <QSet>

#include "ScintillaTypes.h"

class QStandardItemModel;

class CompletionWidget final : public QObject {
    Q_OBJECT

public:
    explicit CompletionWidget(QObject *parent = nullptr);

    ~CompletionWidget() override = default;

    void propertySet(const QVariantHash &objects);

    void fontSet(const QString &family, int pointSize) const;

    [[nodiscard]] bool isVisible() const;

    void completionShow(const QVariantHash &completionSession, const QJsonArray &items);

    Q_INVOKABLE void completionHide() const;

    void completionPrev() const;

    void completionNext() const;

    Q_INVOKABLE void detailReload(int index) const;

    Q_INVOKABLE void textReplace();

signals:
    void appendLog(int type, const QString &prefix, const QString &message);

    void setIndex(const QUrl &documentUrl, int line, int character);

    void setText(const QUrl &documentUrl, const QString &text, int startLine, int startCharacter, int endLine, int endCharacter);

    void addChar(const QUrl &documentUrl, QChar character);

    void showPosition(const QVariantMap &positionSession);

private:
    void placeholderExpand(const QString &placeholder) const;

    QObject *m_tooltip{};
    QObject *m_tableView{};
    QVariantHash m_completionSession{};
    QSet<QString > m_placeholderSet{};
    QStandardItemModel *m_completionModel{};
    QStandardItemModel *m_detailModel{};

    enum CompletionKind {
        Text = 1,
        Method,
        Function,
        Constructor,
        Field,
        Variable,
        Class,
        Interface,
        Module,
        Property,
        Unit,
        Value,
        Enum,
        Keyword,
        Snippet,
        Color,
        File,
        Reference,
        Folder,
        EnumMember,
        Constant,
        Struct,
        Event,
        Operator,
        TypeParameter
    };

    enum CompletionMode {
        Full,
        Simple
    };
};

#endif //UNICOMM_COMPLETIONPOPUP_H
