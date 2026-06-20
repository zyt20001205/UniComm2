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

    [[nodiscard]] bool isValid() const;

    [[nodiscard]] int rows() const;

    [[nodiscard]] int cols() const;

    void resizeTerminal(int rows, int cols);

    void reset(bool hard = true);

    qsizetype write(const QByteArray &bytes);

    qsizetype write(const QString &text);

    [[nodiscard]] QString lineText(int row) const;

    [[nodiscard]] QString screenText() const;

private:
    void flushDamage();

    VTerm *m_vterm{};
    VTermScreen *m_screen{};
    int m_rows{};
    int m_cols{};
};

#endif //UNICOMM_VTERMWIDGET_H
