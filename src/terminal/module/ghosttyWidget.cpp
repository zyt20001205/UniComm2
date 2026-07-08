#include "terminal/module/ghosttyWidget.h"

#include <QApplication>
#include <QByteArray>
#include <QMetaObject>
#include <QThread>
#include <QtGlobal>

#include <array>

#ifdef emit
#pragma push_macro("emit")
#undef emit
#define UNICOMM_RESTORE_QT_EMIT
#endif
#include <ghostty/vt/color.h>
#include <ghostty/vt/render.h>
#include <ghostty/vt/terminal.h>
// These two C headers don't provide their own C++ linkage guard.
extern "C" {
#include <ghostty/vt/key/encoder.h>
#include <ghostty/vt/mouse/encoder.h>
}
#ifdef UNICOMM_RESTORE_QT_EMIT
#pragma pop_macro("emit")
#undef UNICOMM_RESTORE_QT_EMIT
#endif

#include "globals.h"
#include "core/globalManager.h"
#include "util/uniCast.h"

// public
GhosttyWidget::GhosttyWidget(const int rows, const int cols, QObject *parent)
    : QObject(parent),
      m_rows(rows),
      m_cols(cols) {
    GhosttyTerminalOptions options{};
    options.cols = static_cast<uint16_t>(qMax(1, m_cols));
    options.rows = static_cast<uint16_t>(qMax(1, m_rows));
    options.max_scrollback = 10000;

    if (ghostty_terminal_new(nullptr, &m_terminal, options) != GHOSTTY_SUCCESS) m_terminal = nullptr;
    if (ghostty_render_state_new(nullptr, &m_renderState) != GHOSTTY_SUCCESS) m_renderState = nullptr;
    if (ghostty_render_state_row_iterator_new(nullptr, &m_rowIterator) != GHOSTTY_SUCCESS) m_rowIterator = nullptr;
    if (ghostty_render_state_row_cells_new(nullptr, &m_rowCells) != GHOSTTY_SUCCESS) m_rowCells = nullptr;
    if (ghostty_key_encoder_new(nullptr, &m_keyEncoder) != GHOSTTY_SUCCESS) m_keyEncoder = nullptr;
    if (ghostty_key_event_new(nullptr, &m_keyEvent) != GHOSTTY_SUCCESS) m_keyEvent = nullptr;
    if (ghostty_mouse_encoder_new(nullptr, &m_mouseEncoder) != GHOSTTY_SUCCESS) m_mouseEncoder = nullptr;
    if (ghostty_mouse_event_new(nullptr, &m_mouseEvent) != GHOSTTY_SUCCESS) m_mouseEvent = nullptr;

    ghostty_terminal_set(m_terminal, GHOSTTY_TERMINAL_OPT_USERDATA, this);
    ghostty_terminal_set(m_terminal, GHOSTTY_TERMINAL_OPT_WRITE_PTY, reinterpret_cast<const void *>(+[](GhosttyTerminal, void *userdata, const uint8_t *data, const size_t len) {
        static_cast<GhosttyWidget *>(userdata)->writePty(data, len);
    }));
    ghostty_terminal_set(m_terminal, GHOSTTY_TERMINAL_OPT_BELL, reinterpret_cast<const void *>(+[](GhosttyTerminal, void *userdata) {
        static_cast<GhosttyWidget *>(userdata)->bell();
    }));
    ghostty_terminal_set(m_terminal, GHOSTTY_TERMINAL_OPT_ENQUIRY, reinterpret_cast<const void *>(+[](GhosttyTerminal, void *userdata) -> GhosttyString {
        return static_cast<GhosttyWidget *>(userdata)->enquiry();
    }));
    ghostty_terminal_set(m_terminal, GHOSTTY_TERMINAL_OPT_XTVERSION, reinterpret_cast<const void *>(+[](GhosttyTerminal, void *userdata) -> GhosttyString {
        return static_cast<GhosttyWidget *>(userdata)->xtversion();
    }));
    ghostty_terminal_set(m_terminal, GHOSTTY_TERMINAL_OPT_TITLE_CHANGED, reinterpret_cast<const void *>(+[](GhosttyTerminal, void *userdata) {
        static_cast<GhosttyWidget *>(userdata)->titleChanged();
    }));
    ghostty_terminal_set(m_terminal, GHOSTTY_TERMINAL_OPT_SIZE, reinterpret_cast<const void *>(+[](GhosttyTerminal, void *userdata, GhosttySizeReportSize *outSize) -> bool {
        return static_cast<GhosttyWidget *>(userdata)->sizeReport(outSize);
    }));
    ghostty_terminal_set(m_terminal, GHOSTTY_TERMINAL_OPT_COLOR_SCHEME, reinterpret_cast<const void *>(+[](GhosttyTerminal, void *userdata, GhosttyColorScheme *outScheme) -> bool {
        return static_cast<GhosttyWidget *>(userdata)->colorScheme(outScheme);
    }));
    ghostty_terminal_set(m_terminal, GHOSTTY_TERMINAL_OPT_DEVICE_ATTRIBUTES, reinterpret_cast<const void *>(+[](GhosttyTerminal, void *userdata, GhosttyDeviceAttributes *outAttributes) -> bool {
        return static_cast<GhosttyWidget *>(userdata)->deviceAttributes(outAttributes);
    }));

    renderScreen();
    updateCursor();
    updateMouseMode();
}

GhosttyWidget::~GhosttyWidget() {
    ghostty_mouse_event_free(m_mouseEvent);
    ghostty_mouse_encoder_free(m_mouseEncoder);
    ghostty_key_event_free(m_keyEvent);
    ghostty_key_encoder_free(m_keyEncoder);
    ghostty_render_state_row_cells_free(m_rowCells);
    ghostty_render_state_row_iterator_free(m_rowIterator);
    ghostty_render_state_free(m_renderState);
    ghostty_terminal_free(m_terminal);
}

void GhosttyWidget::resize(const int rows, const int cols) {
    m_rows = qMax(1, rows);
    m_cols = qMax(1, cols);
    if (m_terminal) {
        ghostty_terminal_resize(m_terminal, static_cast<uint16_t>(m_cols), static_cast<uint16_t>(m_rows), 1, 1);
    }
    renderScreen();
    updateCursor();
    updateMouseMode();
}

void GhosttyWidget::reset(const bool hard) {
    Q_UNUSED(hard);
    ghostty_terminal_reset(m_terminal);
    renderScreen();
    updateCursor();
    updateMouseMode();
}

void GhosttyWidget::inputWrite(const QByteArray &bytes) {
    for (qsizetype offset = 0; offset < bytes.size(); ++offset) {
        const auto byte = static_cast<uint8_t>(bytes[offset]);
        ghostty_terminal_vt_write(m_terminal, &byte, 1);
    }
    renderScreen();
    updateCursor();
    updateMouseMode();
}

void GhosttyWidget::keyPressed(const int key, const int modifiers, const QString &text) {
    writeEncodedKey(key, modifiers, text);
}

void GhosttyWidget::mousePressed(const int row, const int col, const int button, const int modifiers) {
    m_mouseButtonPressed = true;
    writeEncodedMouse(row, col, button, modifiers, GHOSTTY_MOUSE_ACTION_PRESS);
}

void GhosttyWidget::mouseReleased(const int row, const int col, const int button, const int modifiers) {
    writeEncodedMouse(row, col, button, modifiers, GHOSTTY_MOUSE_ACTION_RELEASE);
    m_mouseButtonPressed = false;
}

void GhosttyWidget::mouseMoved(const int row, const int col, const int button, const int modifiers) {
    writeEncodedMouse(row, col, button, modifiers, GHOSTTY_MOUSE_ACTION_MOTION);
}

void GhosttyWidget::mouseWheeled(const int row, const int col, const int lines, const int modifiers) {
    if (lines == 0) return;
    const GhosttyMouseButton button = lines > 0 ? GHOSTTY_MOUSE_BUTTON_FOUR : GHOSTTY_MOUSE_BUTTON_FIVE;
    const int steps = qMax(1, qAbs(lines) / 3);
    for (int step = 0; step < steps; ++step) {
        writeEncodedMouse(row, col, -button, modifiers, GHOSTTY_MOUSE_ACTION_PRESS);
        writeEncodedMouse(row, col, -button, modifiers, GHOSTTY_MOUSE_ACTION_RELEASE);
    }
}

void GhosttyWidget::mouseScrolled(const int lines) {
    if (lines == 0 || !m_terminal) return;
    GhosttyTerminalScrollViewport viewport{};
    viewport.tag = GHOSTTY_SCROLL_VIEWPORT_DELTA;
    viewport.value.delta = -lines;
    ghostty_terminal_scroll_viewport(m_terminal, viewport);
    renderScreen();
    updateCursor();
}

// private render
void GhosttyWidget::renderScreen() {
    if (!m_terminal || !m_renderState || !m_rowIterator || !m_rowCells) return;
    if (ghostty_render_state_update(m_renderState, m_terminal) != GHOSTTY_SUCCESS) return;

    GhosttyRenderStateColors colors{};
    colors.size = sizeof(GhosttyRenderStateColors);
    if (ghostty_render_state_colors_get(m_renderState, &colors) != GHOSTTY_SUCCESS) return;

    if (ghostty_render_state_get(m_renderState, GHOSTTY_RENDER_STATE_DATA_ROW_ITERATOR, &m_rowIterator) != GHOSTTY_SUCCESS) return;

    auto blankCell = [&colors] {
        TerminalCell cell{};
        cell.width = 1;
        cell.text = QStringLiteral(" ");
        cell.foreground = uni_cast<QColor>(colors.foreground);
        cell.background = uni_cast<QColor>(colors.background);
        return cell;
    };

    auto currentRowCells = [&] {
        QList<TerminalCell> rowCells{};
        rowCells.reserve(m_cols);
        if (ghostty_render_state_row_get(m_rowIterator, GHOSTTY_RENDER_STATE_ROW_DATA_CELLS, &m_rowCells) == GHOSTTY_SUCCESS) {
            while (ghostty_render_state_row_cells_next(m_rowCells) && rowCells.size() < m_cols) {
                rowCells.append(uni_cast<TerminalCell>(GhosttyRenderCellRef{m_rowCells, &colors}));
            }
        }
        while (rowCells.size() < m_cols) rowCells.append(blankCell());
        return rowCells;
    };

    QList<TerminalCell> screen{};
    screen.reserve(m_rows * m_cols);

    int row = 0;
    while (ghostty_render_state_row_iterator_next(m_rowIterator) && row < m_rows) {
        screen.append(currentRowCells());

        const bool clean = false;
        ghostty_render_state_row_set(m_rowIterator, GHOSTTY_RENDER_STATE_ROW_OPTION_DIRTY, &clean);

        ++row;
    }

    while (screen.size() < m_rows * m_cols) screen.append(blankCell());
    Q_EMIT setScreen(m_rows, m_cols, screen, true);

    const GhosttyRenderStateDirty clean = GHOSTTY_RENDER_STATE_DIRTY_FALSE;
    ghostty_render_state_set(m_renderState, GHOSTTY_RENDER_STATE_OPTION_DIRTY, &clean);
}

void GhosttyWidget::updateCursor() {
    bool visible = true;
    bool blink = true;
    bool hasViewportPosition = false;
    uint16_t x = 0;
    uint16_t y = 0;
    GhosttyRenderStateCursorVisualStyle style = GHOSTTY_RENDER_STATE_CURSOR_VISUAL_STYLE_BAR;

    ghostty_render_state_get(m_renderState, GHOSTTY_RENDER_STATE_DATA_CURSOR_VISIBLE, &visible);
    ghostty_render_state_get(m_renderState, GHOSTTY_RENDER_STATE_DATA_CURSOR_BLINKING, &blink);
    ghostty_render_state_get(m_renderState, GHOSTTY_RENDER_STATE_DATA_CURSOR_VIEWPORT_HAS_VALUE, &hasViewportPosition);
    ghostty_render_state_get(m_renderState, GHOSTTY_RENDER_STATE_DATA_CURSOR_VISUAL_STYLE, &style);
    if (hasViewportPosition) {
        ghostty_render_state_get(m_renderState, GHOSTTY_RENDER_STATE_DATA_CURSOR_VIEWPORT_X, &x);
        ghostty_render_state_get(m_renderState, GHOSTTY_RENDER_STATE_DATA_CURSOR_VIEWPORT_Y, &y);
    }

    Q_EMIT setCursorVisible(visible);
    Q_EMIT setCursorBlink(blink);
    Q_EMIT setCursorPosition(QPoint{static_cast<int>(y), static_cast<int>(x)});
    Q_EMIT setCursorShape(uni_cast<int>(style));
}

void GhosttyWidget::updateMouseMode() {
    Q_EMIT setCursorMode(uni_cast<int, GhosttyTerminal>(m_terminal));
}

void GhosttyWidget::writeEncodedKey(const int key, const int modifiers, const QString &text) {
    if (!m_keyEncoder || !m_keyEvent) {
        if (!text.isEmpty()) Q_EMIT outputWrite(text.toUtf8());
        return;
    }

    ghostty_key_encoder_setopt_from_terminal(m_keyEncoder, m_terminal);
    ghostty_key_event_set_action(m_keyEvent, GHOSTTY_KEY_ACTION_PRESS);
    ghostty_key_event_set_key(m_keyEvent, uni_cast<GhosttyKey>(key));
    ghostty_key_event_set_mods(m_keyEvent, uni_cast<GhosttyMods>(modifiers));
    ghostty_key_event_set_consumed_mods(m_keyEvent, 0);
    ghostty_key_event_set_composing(m_keyEvent, false);

    const QByteArray utf8 = text.toUtf8();
    if (utf8.isEmpty()) {
        ghostty_key_event_set_utf8(m_keyEvent, nullptr, 0);
        ghostty_key_event_set_unshifted_codepoint(m_keyEvent, 0);
    } else {
        ghostty_key_event_set_utf8(m_keyEvent, utf8.constData(), static_cast<size_t>(utf8.size()));
        const auto codepoints = text.toUcs4();
        ghostty_key_event_set_unshifted_codepoint(m_keyEvent, codepoints.isEmpty() ? 0 : codepoints.first());
    }

    std::array<char, 128> buffer{};
    size_t len = 0;
    GhosttyResult result = ghostty_key_encoder_encode(m_keyEncoder, m_keyEvent, buffer.data(), buffer.size(), &len);
    if (result == GHOSTTY_OUT_OF_SPACE && len > 0) {
        QByteArray dynamic;
        dynamic.resize(static_cast<qsizetype>(len));
        result = ghostty_key_encoder_encode(m_keyEncoder, m_keyEvent, dynamic.data(), static_cast<size_t>(dynamic.size()), &len);
        if (result == GHOSTTY_SUCCESS && len > 0) {
            dynamic.resize(static_cast<qsizetype>(len));
            Q_EMIT outputWrite(dynamic);
            return;
        }
    }
    if (result == GHOSTTY_SUCCESS && len > 0) {
        Q_EMIT outputWrite(QByteArray(buffer.data(), static_cast<qsizetype>(len)));
        return;
    }
    if (!utf8.isEmpty()) Q_EMIT outputWrite(utf8);
}

void GhosttyWidget::writeEncodedMouse(const int row, const int col, const int button, const int modifiers, const int action) {
    GhosttyMouseEncoderSize size{};
    size.size = sizeof(GhosttyMouseEncoderSize);
    size.screen_width = static_cast<uint32_t>(qMax(1, m_cols));
    size.screen_height = static_cast<uint32_t>(qMax(1, m_rows));
    size.cell_width = 1;
    size.cell_height = 1;

    ghostty_mouse_encoder_setopt_from_terminal(m_mouseEncoder, m_terminal);
    ghostty_mouse_encoder_setopt(m_mouseEncoder, GHOSTTY_MOUSE_ENCODER_OPT_SIZE, &size);
    ghostty_mouse_encoder_setopt(m_mouseEncoder, GHOSTTY_MOUSE_ENCODER_OPT_ANY_BUTTON_PRESSED, &m_mouseButtonPressed);

    ghostty_mouse_event_set_action(m_mouseEvent, static_cast<GhosttyMouseAction>(action));
    if (action == GHOSTTY_MOUSE_ACTION_MOTION) {
        ghostty_mouse_event_clear_button(m_mouseEvent);
    } else {
        const GhosttyMouseButton ghosttyButton = button < 0
                                                    ? static_cast<GhosttyMouseButton>(-button)
                                                    : uni_cast<GhosttyMouseButton>(button);
        ghostty_mouse_event_set_button(m_mouseEvent, ghosttyButton);
    }
    ghostty_mouse_event_set_mods(m_mouseEvent, uni_cast<GhosttyMods>(modifiers));
    ghostty_mouse_event_set_position(m_mouseEvent, GhosttyMousePosition{static_cast<float>(col) + 0.5F, static_cast<float>(row) + 0.5F});

    std::array<char, 128> buffer{};
    size_t len = 0;
    const GhosttyResult result = ghostty_mouse_encoder_encode(m_mouseEncoder, m_mouseEvent, buffer.data(), buffer.size(), &len);
    if (result == GHOSTTY_SUCCESS && len > 0) Q_EMIT outputWrite(QByteArray(buffer.data(), static_cast<qsizetype>(len)));
}

// private callbacks
void GhosttyWidget::writePty(const uint8_t *data, const size_t len) {
    Q_EMIT outputWrite(QByteArray(reinterpret_cast<const char *>(data), static_cast<qsizetype>(len)));
}

void GhosttyWidget::bell() {
    QApplication::beep();
}

GhosttyString GhosttyWidget::enquiry() {
    return uni_cast<GhosttyString>(GhosttyStaticString("UniComm"));
}

GhosttyString GhosttyWidget::xtversion() {
    return uni_cast<GhosttyString>(GhosttyStaticString("UniComm"));
}

void GhosttyWidget::titleChanged() {
    GhosttyString title{};
    if (ghostty_terminal_get(m_terminal, GHOSTTY_TERMINAL_DATA_TITLE, &title) != GHOSTTY_SUCCESS) return;
    Q_EMIT setTitle(uni_cast<QString>(title));
}

bool GhosttyWidget::sizeReport(GhosttySizeReportSize *size) const {
    size->rows = static_cast<uint16_t>(qMax(1, m_rows));
    size->columns = static_cast<uint16_t>(qMax(1, m_cols));
    size->cell_width = 1;
    size->cell_height = 1;
    return true;
}

bool GhosttyWidget::colorScheme(GhosttyColorScheme *scheme) {
    *scheme = g_globalManager && g_globalManager->themeGet() == Theme::Light
                  ? GHOSTTY_COLOR_SCHEME_LIGHT
                  : GHOSTTY_COLOR_SCHEME_DARK;
    return true;
}

bool GhosttyWidget::deviceAttributes(GhosttyDeviceAttributes *attributes) {
    attributes->primary.conformance_level = GHOSTTY_DA_CONFORMANCE_VT220;
    attributes->primary.features[0] = GHOSTTY_DA_FEATURE_ANSI_COLOR;
    attributes->primary.features[1] = GHOSTTY_DA_FEATURE_CLIPBOARD;
    attributes->primary.num_features = 2;
    attributes->secondary.device_type = GHOSTTY_DA_DEVICE_TYPE_VT220;
    attributes->secondary.firmware_version = 1;
    attributes->secondary.rom_cartridge = 0;
    attributes->tertiary.unit_id = 0;
    return true;
}
