#ifndef UNICOMM_COMPLETIONPOPUP_H
#define UNICOMM_COMPLETIONPOPUP_H

#include <QObject>
#include <QSet>

#include "ScintillaTypes.h"

class QStandardItemModel;

class CompletionWidget final : public QObject {
    Q_OBJECT

public:
    explicit CompletionWidget(QWidget *parent = nullptr);

    ~CompletionWidget() override = default;

    void propertySet(const QVariantMap &objects);

    void fontSet(const QString &family, int pointSize) const;

    bool isVisible() const;

    void completionShow(const QVariantHash &completionSession, const QJsonArray &items);

    void completionHide() const;

    void completionPrev() const;

    void completionNext() const;

    Q_INVOKABLE void detailReload(int index) const;

    Q_INVOKABLE void textReplace();

signals:
    void setIndex(const QUrl &scriptUrl, int line, int character);

    void setText(const QUrl &scriptUrl, const QString &text, int startLine, int startCharacter, int endLine, int endCharacter);

    void addChar(const QUrl &scriptUrl, QChar character);

    void insertPort();

    void insertDatabase();

    void insertDatatable();

    void showPosition(const QVariantMap &positionSession);

private:
    void placeholderExpand(const QString &placeholder) const;

    QObject *m_tooltip{};
    QObject *m_tableView{};
    QVariantHash m_completionSession{};
    QSet<QString > m_placeholderSet{};
    QStandardItemModel *m_completionModel{};
    QStandardItemModel *m_detailModel{};

    enum {
        COMPLETION_MODE_FULL,
        COMPLETION_MODE_SIMPLE
    };
};

#endif //UNICOMM_COMPLETIONPOPUP_H
