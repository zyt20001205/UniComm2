#ifndef UNICOMM_SEARCHWINDOW_H
#define UNICOMM_SEARCHWINDOW_H

#include <QStandardItemModel>

class QQuickView;

class SearchModel;

class SearchWindow final : public QObject {
    Q_OBJECT

public:
    explicit SearchWindow(QObject *parent = nullptr);

    ~SearchWindow() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void open() const;

    Q_INVOKABLE void searchRequest() const;

    Q_INVOKABLE void searchFlagsSet(bool matchCase, bool wholeWord, bool wordStart, bool regExp);

    Q_INVOKABLE void indicatorInsert(const QUrl &documentUrl, int startLine, int startCharacter, int endLine, int endCharacter);

signals:
    void insertIndicator(const QUrl &documentUrl, int type, int startLine, int startCharacter, int endLine, int endCharacter, int time);

private:
    struct SearchFlags {
        bool matchCase;
        bool wholeWord;
        bool wordStart;
        bool regExp;
    };

    QQuickView *m_searchWindow{};
    QObject *m_searchBar{};
    QObject *m_searchTextField{};
    QObject *m_searchStatLabel{};

    SearchModel *m_searchModel{};
    SearchFlags m_searchFlags{};
};

class SearchModel final : public QStandardItemModel {
    Q_OBJECT
    Q_PROPERTY(bool empty READ emptyGet NOTIFY emptyChanged)

public:
    explicit SearchModel(QObject *parent = nullptr);

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] bool emptyGet() const {
        return rowCount() == 0;
    }

    signals:
        void emptyChanged();
};

#endif //UNICOMM_SEARCHWINDOW_H
