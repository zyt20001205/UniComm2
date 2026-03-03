#include "scriptModule/codeEditor/searchWidget.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

// SearchWidget public
SearchWidget::SearchWidget(QWidget *parent)
    : QWidget(parent),
      m_searchLineEdit(new QLineEdit()),
      m_wholeWordButton(new QPushButton()),
      m_matchCaseButton(new QPushButton()),
      m_wordStartButton(new QPushButton()),
      m_regExpButton(new QPushButton()),
      m_statLabel(new QLabel("0/0")),
      m_prevButton(new QPushButton()),
      m_nextButton(new QPushButton()),
      m_replaceLineEdit(new QLineEdit()),
      m_replaceButton(new QPushButton(tr("Replace"))),
      m_replaceAllButton(new QPushButton(tr("Replace All"))) {
    auto *layout = new QVBoxLayout(this); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);

    auto *searchBar = new QWidget(); // NOLINT
    layout->addWidget(searchBar);
    auto *searchLayout = new QHBoxLayout(searchBar); // NOLINT
    searchLayout->setContentsMargins(0, 0, 0, 0);
    searchLayout->addWidget(m_searchLineEdit);
    m_searchLineEdit->setClearButtonEnabled(true);
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, [this] { emit searchText(m_searchLineEdit->text(), m_searchFlag); });
    searchLayout->addWidget(m_wholeWordButton);
    m_wholeWordButton->setCheckable(true);
    m_wholeWordButton->setFixedSize(24, 24);
    m_wholeWordButton->setIcon(QIcon(":/icon/wholeWord.svg"));
    m_wholeWordButton->setToolTip(tr("Whole Word"));
    connect(m_wholeWordButton, &QPushButton::clicked, this, [this](const bool status) {
        // if (status) {
        //     m_searchFlag |= QsciScintilla::SCFIND_WHOLEWORD;
        // } else {
        //     m_searchFlag &= ~QsciScintilla::SCFIND_WHOLEWORD;
        // }
        emit searchText(m_searchLineEdit->text(), m_searchFlag);
    });
    searchLayout->addWidget(m_matchCaseButton);
    m_matchCaseButton->setCheckable(true);
    m_matchCaseButton->setFixedSize(24, 24);
    m_matchCaseButton->setIcon(QIcon(":/icon/matchCase.svg"));
    m_matchCaseButton->setToolTip(tr("Match Case"));
    connect(m_matchCaseButton, &QPushButton::clicked, this, [this](const bool status) {
        // if (status) {
        //     m_searchFlag |= QsciScintilla::SCFIND_MATCHCASE;
        // } else {
        //     m_searchFlag &= ~QsciScintilla::SCFIND_MATCHCASE;
        // }
        emit searchText(m_searchLineEdit->text(), m_searchFlag);
    });
    searchLayout->addWidget(m_wordStartButton);
    m_wordStartButton->setCheckable(true);
    m_wordStartButton->setFixedSize(24, 24);
    m_wordStartButton->setIcon(QIcon(":/icon/wordStart.svg"));
    m_wordStartButton->setToolTip(tr("Word Start"));
    connect(m_wordStartButton, &QPushButton::clicked, this, [this](const bool status) {
        // if (status) {
        //     m_searchFlag |= QsciScintilla::SCFIND_WORDSTART;
        // } else {
        //     m_searchFlag &= ~QsciScintilla::SCFIND_WORDSTART;
        // }
        emit searchText(m_searchLineEdit->text(), m_searchFlag);
    });
    searchLayout->addWidget(m_regExpButton);
    m_regExpButton->setCheckable(true);
    m_regExpButton->setFixedSize(24, 24);
    m_regExpButton->setIcon(QIcon(":/icon/regExp.svg"));
    m_regExpButton->setToolTip(tr("Regular Expression"));
    connect(m_regExpButton, &QPushButton::clicked, this, [this](const bool status) {
        // if (status) {
        //     m_searchFlag |= QsciScintilla::SCFIND_REGEXP;
        // } else {
        //     m_searchFlag &= ~QsciScintilla::SCFIND_REGEXP;
        // }
        // emit searchText(m_searchLineEdit->text(), m_searchFlag);
    });
    searchLayout->addStretch();
    searchLayout->addWidget(m_statLabel);
    searchLayout->addWidget(m_prevButton);
    m_prevButton->setEnabled(false);
    m_prevButton->setFixedSize(24, 24);
    m_prevButton->setIcon(QIcon(":/icon/arrowUp.svg"));
    m_prevButton->setToolTip(tr("Search Previous"));
    connect(m_prevButton, &QPushButton::clicked, this, &SearchWidget::searchPrev);
    searchLayout->addWidget(m_nextButton);
    m_nextButton->setEnabled(false);
    m_nextButton->setFixedSize(24, 24);
    m_nextButton->setIcon(QIcon(":/icon/arrowDown.svg"));
    m_nextButton->setToolTip(tr("Search Next"));
    connect(m_nextButton, &QPushButton::clicked, this, &SearchWidget::searchNext);

    auto *replaceBar = new QWidget(); // NOLINT
    layout->addWidget(replaceBar);
    auto *replaceLayout = new QHBoxLayout(replaceBar); // NOLINT
    replaceLayout->setContentsMargins(0, 0, 0, 0);
    replaceLayout->addWidget(m_replaceLineEdit);
    m_replaceLineEdit->setClearButtonEnabled(true);
    replaceLayout->addStretch();
    replaceLayout->addWidget(m_replaceButton);
    m_replaceButton->setEnabled(false);
    connect(m_replaceButton, &QPushButton::clicked, this, [this] { emit replaceText(m_replaceLineEdit->text()); });
    replaceLayout->addWidget(m_replaceAllButton);
    m_replaceAllButton->setEnabled(false);
    connect(m_replaceAllButton, &QPushButton::clicked, this, [this] { emit replaceAllText(m_replaceLineEdit->text()); });

    setTabOrder(m_searchLineEdit, m_replaceLineEdit);

    hide();
}

void SearchWidget::toggle() {
    if (isVisible()) hide();
    else {
        m_searchLineEdit->setFocus();
        show();
    }
}

void SearchWidget::statSet(int current, const int total) const {
    if (current == 0 && total == 0) {
        m_prevButton->setEnabled(false);
        m_nextButton->setEnabled(false);
        m_replaceButton->setEnabled(false);
        m_replaceAllButton->setEnabled(false);
    } else {
        m_prevButton->setEnabled(true);
        m_nextButton->setEnabled(true);
        m_replaceButton->setEnabled(true);
        m_replaceAllButton->setEnabled(true);
        current++;
    }
    m_statLabel->setText(QString("%1/%2").arg(QString::number(current), QString::number(total)));
}