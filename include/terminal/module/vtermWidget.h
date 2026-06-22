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
        QString text;
        QColor foreground;
        QColor background;
    };

    explicit VtermWidget(int rows = 24, int cols = 80, QObject *parent = nullptr);

    ~VtermWidget() override;

    [[nodiscard]] int rows() const {
        return m_rows;
    };

    [[nodiscard]] int cols() const {
        return m_cols;
    };

    void resize(int rows, int cols);

    void reset(bool hard = true) const;

    [[nodiscard]] QByteArray keyboardKey(int key, int modifiers) const;

    [[nodiscard]] QByteArray keyboardUnichar(const QString &text, int modifiers) const;

    void inputWrite(const QByteArray &bytes) const;

    [[nodiscard]] QString text() const;

    [[nodiscard]] QList<Cell> cells() const;

    [[nodiscard]] int cursorPosition() const;

private:
    [[nodiscard]] QByteArray outputRead() const;

    int m_rows{};
    int m_cols{};
    VTerm *m_vterm{};
    VTermScreen *m_screen{};
    const VTermScreenCallbacks m_callbacks{};
};

#endif //UNICOMM_VTERMWIDGET_H
