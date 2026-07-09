#ifndef UNICOMM_VTERMWIDGET_H
#define UNICOMM_VTERMWIDGET_H

#include <QByteArray>
#include <QObject>
#include <QPoint>
#include <vterm.h>

extern "C" {
typedef VTerm VTerm;
typedef VTermState VTermState;
typedef VTermScreen VTermScreen;
}

struct TerminalCell;

class VtermWidget final : public QObject {
    Q_OBJECT

public:
    explicit VtermWidget(int rows = 24, int cols = 80, QObject *parent = nullptr);

    ~VtermWidget() override;

    void resize(int rows, int cols);

    void reset(bool hard = true);

    void inputWrite(const QByteArray &bytes);

    void keyPressed(int key, int modifiers, const QString &text);

    void mousePressed(int row, int col, int button, int modifiers);

    void mouseReleased(int row, int col, int button, int modifiers);

    void mouseMoved(int row, int col, int button, int modifiers);

    void mouseWheeled(int row, int col, int lines, int modifiers);

    void mouseScrolled(int lines);

    void linkOpen(int uri) const;

signals:
    void outputWrite(const QByteArray &bytes);

    void setScreen(int row, int col, const QList<TerminalCell> &cells, bool atBottom);

    void setCursorPosition(const QPoint &position, const QPoint &oldPosition);

    void setCursorVisible(bool visible);

    void setCursorBlink(bool blink);

    void setTitle(const QString &title);

    void setCursorShape(int shape);

    void setCursorMode(int mode);

private:
    void renderScreen();

    void outputRead();

    int cursorMove(VTermPos pos, VTermPos oldPos, int visible);

    int termPropSet(VTermProp prop, const VTermValue *value);

    int bell();

    int linePush(int cols, const VTermScreenCell *cells);

    int selectionSet(VTermSelectionMask mask, VTermStringFragment frag);

    int selectionQuery(VTermSelectionMask mask);

    int m_rows{};
    int m_cols{};
    VTerm *m_vterm{};
    VTermState *m_state{};
    VTermScreen *m_screen{};
    VTermScreenCallbacks m_callbacks{};
    VTermSelectionCallbacks m_selectionCallbacks{};
    QPoint m_cursorPosition{};
    bool m_cursorVisible{true};
    bool m_altScreen{false};
    QList<QList<TerminalCell> > m_scrollback{};
    int m_scrollOffset{};
    // osc52
    QByteArray m_selectionBuffer{};
    QString m_pendingSelection{};
};

#endif //UNICOMM_VTERMWIDGET_H
