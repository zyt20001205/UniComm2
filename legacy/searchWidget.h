#ifndef UNICOMM_SEARCHWIDGET_H
#define UNICOMM_SEARCHWIDGET_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;

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

#endif //UNICOMM_SEARCHWIDGET_H