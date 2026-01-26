#ifndef UNICOMM_EDITORWIDGET_H
#define UNICOMM_EDITORWIDGET_H

#include <QUrl>
#include <Qsci/qsciscintilla.h>

class EditorWidget final : public QsciScintilla {
    Q_OBJECT

public:
    explicit EditorWidget(const QUrl &scriptUrl, QWidget *parent = nullptr);

    ~EditorWidget() override = default;

    void breakpointLoad();

    void textSearch(const QString &text, int flag);

    void prevSearch();

    void nextSearch();

    void cursorPositionSet(int line, int index);

    void cursorPositionGet(int *line, int *index) const;

    void selectionGet(int &indexFrom, int &indexTo, int &lineFrom, int &lineTo) const;

    void textSet(const QString &text);

    void textInsert(const QString &text, int line = -1, int index = -1);

    void textReplace(const QString &text);

    void textReplaceAll(const QString &text);

    void textReplace(const QString &text, int lineFrom, int indexFrom, int lineTo, int indexTo);

    void indicatorInsert(int type, int lineFrom, int indexFrom, int lineTo, int indexTo, int time = -1);

    void indicatorRemove(int type, int lineFrom = -1, int indexFrom = -1, int lineTo = -1, int indexTo = -1);

    void markerInsert(int type, int line, int time = -1);

    void markerRemove(int type, int line = -1);

signals:
    void showMenu(const QVariantHash &menuSession);

    void requestPermission();

    void requestIdle();

    void hideDwellWidget();

    void requestDefinition();

    void requestDocumentHighlight();

    void requestHover();

    void requestImplementation();

    void requestOnTypeFormatting();

    void requestReferences();

    void requestTypeDefinition();

    void setStat(int current, int total);

protected:
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

    QUrl m_scriptUrl{};
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

#endif //UNICOMM_EDITORWIDGET_H
