#ifndef UNICOMM_SCRIPTEDITOR_H
#define UNICOMM_SCRIPTEDITOR_H

#include "Qsci/qsciscintilla.h"

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

    void requestImplementation();

    void requestOnTypeFormatting();

    void requestReferences();

    void requestTypeDefinition();

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
    QList<QList<int> > m_searchList{};
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

#endif //UNICOMM_SCRIPTEDITOR_H