#ifndef UNICOMM_VTERMWIDGET_H
#define UNICOMM_VTERMWIDGET_H

#include <QByteArray>
#include <QObject>
#include <QString>

extern "C" {
typedef struct VTerm VTerm;
typedef struct VTermScreen VTermScreen;
}

class VtermWidget final : public QObject {
    Q_OBJECT

public:
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

    void write(const QByteArray &bytes) const;

    [[nodiscard]] QByteArray keyboardKey(int key, int modifiers) const;

    [[nodiscard]] QByteArray keyboardUnichar(const QString &text, int modifiers) const;

    [[nodiscard]] QString text() const;

    [[nodiscard]] int cursorPosition() const;

private:
    [[nodiscard]] QByteArray readOutput() const;

    int m_rows{};
    int m_cols{};
    VTerm *m_vterm{};
    VTermScreen *m_screen{};
};

#endif //UNICOMM_VTERMWIDGET_H
