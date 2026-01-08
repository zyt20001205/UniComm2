#ifndef UNICOMM_COMPLETIONPOPUP_H
#define UNICOMM_COMPLETIONPOPUP_H

#include <QSortFilterProxyModel>
#include <QWidget>

class QLabel;
class QListView;
class QPushButton;
class QStandardItemModel;

class CompletionWidget final : public QObject {
    Q_OBJECT

public:
    explicit CompletionWidget(QWidget *parent = nullptr);

    ~CompletionWidget() override = default;

    void propertySet(const QVariantMap &objects);

    void fontSet(const QString &family, int pointSize) const;

    bool isVisible() const;

    void completionShow(const QVariantMap &completionSession, const QJsonArray &items);

    void completionHide() const;

    void completionPrev() const;

    void completionNext() const;

    Q_INVOKABLE void textReplace();

signals:
    void setCursorPosition(const QUrl &scriptUrl, int startLine, int startCharacter);

    void replaceText(const QUrl &scriptUrl, const QString &text, int lineFrom, int indexFrom, int lineTo, int indexTo);

    void addChar(const QUrl &scriptUrl, QChar character);

    void insertPort();

    void insertDatabase();

    void insertDatatable();

    void showPosition(const QVariantMap &positionSession);

private:
    void placeholderExpand(const QString &placeholder) const;

    QObject *m_tooltip{};
    QObject *m_tableView{};
    QVariantMap m_completionSession{};
    QSet<QString > m_placeholderSet{};
    QStandardItemModel *m_completionModel{};

    enum {
        COMPLETION_MODE_FULL,
        COMPLETION_MODE_SIMPLE
    };
};

#endif //UNICOMM_COMPLETIONPOPUP_H
