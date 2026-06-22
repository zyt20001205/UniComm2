#ifndef UNICOMM_VTERMWIDGET_H
#define UNICOMM_VTERMWIDGET_H

#include <QColor>
#include <QList>
#include <QObject>
#include <QString>
#include <vterm.h>

extern "C" {
typedef VTerm VTerm;
typedef VTermScreen VTermScreen;
}

class VtermWidget final : public QObject {
    Q_OBJECT

public:
    struct Cell {
        QString text{};
        QColor foreground{};
        QColor background{};
    };

    struct Cursor {
        int row{};
        int col{};
    };

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

    void setScreen(int row, int col, const QList<Cell> &cells, const Cursor &cursor);

private:
    void outputRead();

    int m_rows{};
    int m_cols{};
    VTerm *m_vterm{};
    VTermScreen *m_screen{};
    const VTermScreenCallbacks m_callbacks{};
};

#endif //UNICOMM_VTERMWIDGET_H
