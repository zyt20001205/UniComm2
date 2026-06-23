#ifndef UNICOMM_VTERMWIDGET_H
#define UNICOMM_VTERMWIDGET_H

#include <QObject>
#include <QPoint>
#include <vterm.h>

extern "C" {
typedef VTerm VTerm;
typedef VTermScreen VTermScreen;
}

struct TerminalCell;

class VtermWidget final : public QObject {
    Q_OBJECT

public:
    explicit VtermWidget(int rows = 24, int cols = 80, QObject *parent = nullptr);

    ~VtermWidget() override;

    void resize(int rows, int cols);

    void reset(bool hard = true) const;

    void inputWrite(const QByteArray &bytes);

    void keyPressed(int key, int modifiers, const QString &text);

    void mousePressed(int row, int col, int button, int modifiers);

    void mouseReleased(int row, int col, int button, int modifiers);

    void mouseMoved(int row, int col, int button, int modifiers);

signals:
    void outputWrite(const QByteArray &bytes);

    void setScreen(int row, int col, const QList<TerminalCell> &cells, const QList<QList<TerminalCell>> &scrollback);

    void setCursorPosition(const QPoint &position);

    void setCursorVisible(bool visible);

    void setCursorBlink(bool blink);

    void setCursorShape(int shape);

private:
    void outputRead();

    int cursorMove(VTermPos pos, VTermPos oldPos, int visible);

    int termPropSet(VTermProp prop, const VTermValue *value);

    int linePush(int cols, const VTermScreenCell *cells);

    int m_rows{};
    int m_cols{};
    VTerm *m_vterm{};
    VTermScreen *m_screen{};
    VTermScreenCallbacks m_callbacks{};
    bool m_cursorVisible{true};
    QList<QList<TerminalCell>> m_scrollback{};
};

#endif //UNICOMM_VTERMWIDGET_H
