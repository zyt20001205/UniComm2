#ifndef UNICOMM_SEARCHWINDOW_H
#define UNICOMM_SEARCHWINDOW_H

#include <QStandardItemModel>

class QQuickWidget;
class QVBoxLayout;
class QWidget;

class SearchModel;

class SearchWindow final : public QObject {
    Q_OBJECT

public:
    explicit SearchWindow(QObject *parent = nullptr);

    ~SearchWindow() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    Q_INVOKABLE void searchRequest() const;

    void open() const;

    Q_INVOKABLE void searchFlagsSet(bool matchCase, bool wholeWord, bool wordStart, bool regExp);

private:
    struct SearchFlags {
        bool matchCase;
        bool wholeWord;
        bool wordStart;
        bool regExp;
    };

    QWidget *m_widget{};
    QVBoxLayout *m_columnLayout{};
    QQuickWidget *m_searchWidget{};
    QObject *m_searchBar{};
    QObject *m_searchTextField{};
    QObject *m_searchPrevButton{};
    QObject *m_searchNextButton{};
    QObject *m_searchStatLabel{};
    QObject *m_replaceBar{};
    QObject *m_replaceTextField{};
    QObject *m_replaceTextButton{};
    QObject *m_replaceAllButton{};

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
