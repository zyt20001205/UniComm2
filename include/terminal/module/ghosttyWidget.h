#ifndef UNICOMM_GHOSTTYWIDGET_H
#define UNICOMM_GHOSTTYWIDGET_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QPoint>

#include <cstddef>
#include <cstdint>

#ifdef emit
#pragma push_macro("emit")
#undef emit
#define UNICOMM_GHOSTTYWIDGET_RESTORE_QT_EMIT
#endif
#include <ghostty/vt/terminal.h>
#ifdef UNICOMM_GHOSTTYWIDGET_RESTORE_QT_EMIT
#pragma pop_macro("emit")
#undef UNICOMM_GHOSTTYWIDGET_RESTORE_QT_EMIT
#endif

typedef struct GhosttyKeyEncoderImpl *GhosttyKeyEncoder;
typedef struct GhosttyKeyEventImpl *GhosttyKeyEvent;
typedef struct GhosttyMouseEncoderImpl *GhosttyMouseEncoder;
typedef struct GhosttyMouseEventImpl *GhosttyMouseEvent;

struct TerminalCell;

class GhosttyWidget final : public QObject {
    Q_OBJECT

public:
    explicit GhosttyWidget(int rows = 24, int cols = 80, QObject *parent = nullptr);

    ~GhosttyWidget() override;

    void resize(int rows, int cols);

    void reset(bool hard = true);

    void inputWrite(const QByteArray &bytes);

    void keyPressed(int key, int modifiers, const QString &text);

    void mousePressed(int row, int col, int button, int modifiers);

    void mouseReleased(int row, int col, int button, int modifiers);

    void mouseMoved(int row, int col, int button, int modifiers);

    void mouseWheeled(int row, int col, int lines, int modifiers);

    void mouseScrolled(int lines);

signals:
    void outputWrite(const QByteArray &bytes);

    void setScreen(int row, int col, const QList<TerminalCell> &cells, bool atBottom);

    void setCursorPosition(const QPoint &position);

    void setCursorVisible(bool visible);

    void setCursorBlink(bool blink);

    void setTitle(const QString &title);

    void setCursorShape(int shape);

    void setCursorMode(int mode);

private:
    void renderScreen();

    void updateCursor();

    void updateMouseMode();

    void writeEncodedKey(int key, int modifiers, const QString &text);

    void writeEncodedMouse(int row, int col, int button, int modifiers, int action);

    void writePty(const uint8_t *data, size_t len);

    static void bell();

    [[nodiscard]] static GhosttyString enquiry();

    [[nodiscard]] static GhosttyString xtversion();

    void titleChanged();

    [[nodiscard]] bool sizeReport(GhosttySizeReportSize *size) const;

    [[nodiscard]] static bool colorScheme(GhosttyColorScheme *scheme);

    [[nodiscard]] static bool deviceAttributes(GhosttyDeviceAttributes *attributes);

    int m_rows{};
    int m_cols{};
    GhosttyTerminal m_terminal{};
    GhosttyRenderState m_renderState{};
    GhosttyKeyEncoder m_keyEncoder{};
    GhosttyKeyEvent m_keyEvent{};
    GhosttyMouseEncoder m_mouseEncoder{};
    GhosttyMouseEvent m_mouseEvent{};
    bool m_mouseButtonPressed{};
};

#endif //UNICOMM_GHOSTTYWIDGET_H
