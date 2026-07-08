#include "terminal/module/ghosttyWidget.h"

#include <QApplication>
#include <QByteArray>
#include <QColor>
#include <QMetaObject>
#include <QThread>
#include <QVector>

#include <algorithm>
#include <array>
#include <cstring>

#ifdef emit
#pragma push_macro("emit")
#undef emit
#define UNICOMM_RESTORE_QT_EMIT
#endif
#include <ghostty/vt/color.h>
#include <ghostty/vt/modes.h>
#include <ghostty/vt/render.h>
#include <ghostty/vt/screen.h>
#include <ghostty/vt/terminal.h>
extern "C" {
#include <ghostty/vt/key/encoder.h>
#include <ghostty/vt/mouse/encoder.h>
}
#ifdef UNICOMM_RESTORE_QT_EMIT
#pragma pop_macro("emit")
#undef UNICOMM_RESTORE_QT_EMIT
#endif
#include <vterm.h>

#include "globals.h"

namespace {
    [[nodiscard]] QColor toQColor(const GhosttyColorRgb &color) {
        return {color.r, color.g, color.b};
    }

    [[nodiscard]] QColor faintColor(const QColor &foreground, const QColor &background) {
        return {
            (foreground.red() * 55 + background.red() * 45) / 100,
            (foreground.green() * 55 + background.green() * 45) / 100,
            (foreground.blue() * 55 + background.blue() * 45) / 100
        };
    }

    [[nodiscard]] QColor styleColor(
        const GhosttyStyleColor &color,
        const GhosttyColorRgb &fallback,
        const GhosttyColorRgb palette[256]
    ) {
        switch (color.tag) {
            case GHOSTTY_STYLE_COLOR_RGB:
                return toQColor(color.value.rgb);
            case GHOSTTY_STYLE_COLOR_PALETTE:
                return toQColor(palette[color.value.palette]);
            case GHOSTTY_STYLE_COLOR_NONE:
            default:
                return toQColor(fallback);
        }
    }

    [[nodiscard]] GhosttyMods toGhosttyMods(const int modifiers) {
        GhosttyMods mods = 0;
        if (modifiers & Qt::ShiftModifier) mods |= GHOSTTY_MODS_SHIFT;
        if (modifiers & Qt::ControlModifier) mods |= GHOSTTY_MODS_CTRL;
        if (modifiers & Qt::AltModifier) mods |= GHOSTTY_MODS_ALT;
        if (modifiers & Qt::MetaModifier) mods |= GHOSTTY_MODS_SUPER;
        return mods;
    }

    [[nodiscard]] GhosttyKey toGhosttyKey(const int key) {
        if (key >= Qt::Key_A && key <= Qt::Key_Z) return static_cast<GhosttyKey>(GHOSTTY_KEY_A + key - Qt::Key_A);
        if (key >= Qt::Key_0 && key <= Qt::Key_9) return static_cast<GhosttyKey>(GHOSTTY_KEY_DIGIT_0 + key - Qt::Key_0);
        if (key >= Qt::Key_F1 && key <= Qt::Key_F25) return static_cast<GhosttyKey>(GHOSTTY_KEY_F1 + key - Qt::Key_F1);

        switch (key) {
            case Qt::Key_QuoteLeft: return GHOSTTY_KEY_BACKQUOTE;
            case Qt::Key_Backslash: return GHOSTTY_KEY_BACKSLASH;
            case Qt::Key_BracketLeft: return GHOSTTY_KEY_BRACKET_LEFT;
            case Qt::Key_BracketRight: return GHOSTTY_KEY_BRACKET_RIGHT;
            case Qt::Key_Comma: return GHOSTTY_KEY_COMMA;
            case Qt::Key_Equal: return GHOSTTY_KEY_EQUAL;
            case Qt::Key_Minus: return GHOSTTY_KEY_MINUS;
            case Qt::Key_Period: return GHOSTTY_KEY_PERIOD;
            case Qt::Key_Apostrophe: return GHOSTTY_KEY_QUOTE;
            case Qt::Key_Semicolon: return GHOSTTY_KEY_SEMICOLON;
            case Qt::Key_Slash: return GHOSTTY_KEY_SLASH;
            case Qt::Key_Return:
            case Qt::Key_Enter: return GHOSTTY_KEY_ENTER;
            case Qt::Key_Backspace: return GHOSTTY_KEY_BACKSPACE;
            case Qt::Key_Tab:
            case Qt::Key_Backtab: return GHOSTTY_KEY_TAB;
            case Qt::Key_Escape: return GHOSTTY_KEY_ESCAPE;
            case Qt::Key_Space: return GHOSTTY_KEY_SPACE;
            case Qt::Key_Insert: return GHOSTTY_KEY_INSERT;
            case Qt::Key_Delete: return GHOSTTY_KEY_DELETE;
            case Qt::Key_Home: return GHOSTTY_KEY_HOME;
            case Qt::Key_End: return GHOSTTY_KEY_END;
            case Qt::Key_PageUp: return GHOSTTY_KEY_PAGE_UP;
            case Qt::Key_PageDown: return GHOSTTY_KEY_PAGE_DOWN;
            case Qt::Key_Up: return GHOSTTY_KEY_ARROW_UP;
            case Qt::Key_Down: return GHOSTTY_KEY_ARROW_DOWN;
            case Qt::Key_Left: return GHOSTTY_KEY_ARROW_LEFT;
            case Qt::Key_Right: return GHOSTTY_KEY_ARROW_RIGHT;
            default: return GHOSTTY_KEY_UNIDENTIFIED;
        }
    }

    [[nodiscard]] GhosttyMouseButton toGhosttyMouseButton(const int button) {
        switch (button) {
            case Qt::LeftButton: return GHOSTTY_MOUSE_BUTTON_LEFT;
            case Qt::RightButton: return GHOSTTY_MOUSE_BUTTON_RIGHT;
            case Qt::MiddleButton: return GHOSTTY_MOUSE_BUTTON_MIDDLE;
            default: return GHOSTTY_MOUSE_BUTTON_UNKNOWN;
        }
    }

    [[nodiscard]] int toVtermCursorShape(const GhosttyRenderStateCursorVisualStyle style) {
        switch (style) {
            case GHOSTTY_RENDER_STATE_CURSOR_VISUAL_STYLE_BLOCK:
            case GHOSTTY_RENDER_STATE_CURSOR_VISUAL_STYLE_BLOCK_HOLLOW:
                return VTERM_PROP_CURSORSHAPE_BLOCK;
            case GHOSTTY_RENDER_STATE_CURSOR_VISUAL_STYLE_UNDERLINE:
                return VTERM_PROP_CURSORSHAPE_UNDERLINE;
            case GHOSTTY_RENDER_STATE_CURSOR_VISUAL_STYLE_BAR:
            default:
                return VTERM_PROP_CURSORSHAPE_BAR_LEFT;
        }
    }

    [[nodiscard]] int ghosttyMouseMode(const GhosttyTerminal terminal) {
        bool any = false;
        bool button = false;
        bool normal = false;
        bool x10 = false;
        ghostty_terminal_mode_get(terminal, GHOSTTY_MODE_ANY_MOUSE, &any);
        ghostty_terminal_mode_get(terminal, GHOSTTY_MODE_BUTTON_MOUSE, &button);
        ghostty_terminal_mode_get(terminal, GHOSTTY_MODE_NORMAL_MOUSE, &normal);
        ghostty_terminal_mode_get(terminal, GHOSTTY_MODE_X10_MOUSE, &x10);
        if (any) return VTERM_PROP_MOUSE_MOVE;
        if (button) return VTERM_PROP_MOUSE_DRAG;
        if (normal || x10) return VTERM_PROP_MOUSE_CLICK;
        return VTERM_PROP_MOUSE_NONE;
    }

    template<typename Callback>
    [[nodiscard]] const void *ghosttyCallbackOption(const Callback callback) {
        static_assert(sizeof(callback) == sizeof(const void *));
        const void *value{};
        std::memcpy(&value, &callback, sizeof(value));
        return value;
    }

    [[nodiscard]] QByteArray utf8ForCell(const GhosttyRenderStateRowCells cells) {
        std::array<uint8_t, 64> storage{};
        GhosttyBuffer buffer{storage.data(), storage.size(), 0};
        GhosttyResult result = ghostty_render_state_row_cells_get(cells, GHOSTTY_RENDER_STATE_ROW_CELLS_DATA_GRAPHEMES_UTF8, &buffer);
        if (result == GHOSTTY_SUCCESS) return {reinterpret_cast<const char *>(storage.data()), static_cast<qsizetype>(buffer.len)};
        if (result != GHOSTTY_OUT_OF_SPACE || buffer.len == 0) return {};

        QByteArray dynamic;
        dynamic.resize(static_cast<qsizetype>(buffer.len));
        GhosttyBuffer dynamicBuffer{reinterpret_cast<uint8_t *>(dynamic.data()), static_cast<size_t>(dynamic.size()), 0};
        result = ghostty_render_state_row_cells_get(cells, GHOSTTY_RENDER_STATE_ROW_CELLS_DATA_GRAPHEMES_UTF8, &dynamicBuffer);
        if (result != GHOSTTY_SUCCESS) return {};
        dynamic.resize(static_cast<qsizetype>(dynamicBuffer.len));
        return dynamic;
    }

    [[nodiscard]] TerminalCell terminalCellFromGhostty(
        const GhosttyRenderStateRowCells cells,
        const GhosttyColorRgb &defaultForeground,
        const GhosttyColorRgb &defaultBackground
    ) {
        TerminalCell cell{};
        cell.width = 1;

        GhosttyCell raw{};
        if (ghostty_render_state_row_cells_get(cells, GHOSTTY_RENDER_STATE_ROW_CELLS_DATA_RAW, &raw) == GHOSTTY_SUCCESS) {
            GhosttyCellWide wide = GHOSTTY_CELL_WIDE_NARROW;
            if (ghostty_cell_get(raw, GHOSTTY_CELL_DATA_WIDE, &wide) == GHOSTTY_SUCCESS) {
                switch (wide) {
                    case GHOSTTY_CELL_WIDE_WIDE:
                        cell.width = 2;
                        break;
                    case GHOSTTY_CELL_WIDE_SPACER_TAIL:
                        cell.width = 0;
                        break;
                    default:
                        cell.width = 1;
                        break;
                }
            }
        }

        const QByteArray text = utf8ForCell(cells);
        cell.text = text.isEmpty() ? QStringLiteral(" ") : QString::fromUtf8(text);

        GhosttyColorRgb foreground = defaultForeground;
        if (ghostty_render_state_row_cells_get(cells, GHOSTTY_RENDER_STATE_ROW_CELLS_DATA_FG_COLOR, &foreground) == GHOSTTY_NO_VALUE) {
            foreground = defaultForeground;
        }
        GhosttyColorRgb background = defaultBackground;
        if (ghostty_render_state_row_cells_get(cells, GHOSTTY_RENDER_STATE_ROW_CELLS_DATA_BG_COLOR, &background) == GHOSTTY_NO_VALUE) {
            background = defaultBackground;
        }

        cell.foreground = toQColor(foreground);
        cell.background = toQColor(background);

        GhosttyStyle style{};
        style.size = sizeof(GhosttyStyle);
        if (ghostty_render_state_row_cells_get(cells, GHOSTTY_RENDER_STATE_ROW_CELLS_DATA_STYLE, &style) == GHOSTTY_SUCCESS) {
            cell.bold = style.bold;
            cell.faint = style.faint;
            cell.italic = style.italic;
            cell.underline = style.underline != 0;
            cell.strike = style.strikethrough;
            if (style.inverse) std::swap(cell.foreground, cell.background);
            if (style.invisible) cell.foreground = cell.background;
        }

        if (cell.faint) cell.foreground = faintColor(cell.foreground, cell.background);
        if (cell.width == 0) cell.text.clear();
        return cell;
    }

    [[nodiscard]] TerminalCell terminalCellFromGridRef(
        const GhosttyGridRef &ref,
        const GhosttyColorRgb &defaultForeground,
        const GhosttyColorRgb &defaultBackground,
        const GhosttyColorRgb palette[256]
    ) {
        TerminalCell cell{};
        cell.width = 1;
        cell.text = QStringLiteral(" ");
        cell.foreground = toQColor(defaultForeground);
        cell.background = toQColor(defaultBackground);

        GhosttyCell raw{};
        if (ghostty_grid_ref_cell(&ref, &raw) == GHOSTTY_SUCCESS) {
            GhosttyCellWide wide = GHOSTTY_CELL_WIDE_NARROW;
            if (ghostty_cell_get(raw, GHOSTTY_CELL_DATA_WIDE, &wide) == GHOSTTY_SUCCESS) {
                switch (wide) {
                    case GHOSTTY_CELL_WIDE_WIDE:
                        cell.width = 2;
                        break;
                    case GHOSTTY_CELL_WIDE_SPACER_TAIL:
                        cell.width = 0;
                        cell.text.clear();
                        break;
                    default:
                        break;
                }
            }

            GhosttyCellContentTag contentTag = GHOSTTY_CELL_CONTENT_CODEPOINT;
            if (ghostty_cell_get(raw, GHOSTTY_CELL_DATA_CONTENT_TAG, &contentTag) == GHOSTTY_SUCCESS) {
                if (contentTag == GHOSTTY_CELL_CONTENT_BG_COLOR_RGB) {
                    GhosttyColorRgb background{};
                    if (ghostty_cell_get(raw, GHOSTTY_CELL_DATA_COLOR_RGB, &background) == GHOSTTY_SUCCESS) {
                        cell.background = toQColor(background);
                    }
                } else if (contentTag == GHOSTTY_CELL_CONTENT_BG_COLOR_PALETTE) {
                    GhosttyColorPaletteIndex index{};
                    if (ghostty_cell_get(raw, GHOSTTY_CELL_DATA_COLOR_PALETTE, &index) == GHOSTTY_SUCCESS) {
                        cell.background = toQColor(palette[index]);
                    }
                }
            }
        }

        std::array<uint32_t, 16> graphemes{};
        size_t graphemeCount{};
        GhosttyResult result = ghostty_grid_ref_graphemes(&ref, graphemes.data(), graphemes.size(), &graphemeCount);
        if (result == GHOSTTY_OUT_OF_SPACE && graphemeCount > 0) {
            QVector<char32_t> dynamic;
            dynamic.resize(static_cast<qsizetype>(graphemeCount));
            QVector<uint32_t> rawGraphemes;
            rawGraphemes.resize(static_cast<qsizetype>(graphemeCount));
            result = ghostty_grid_ref_graphemes(&ref, rawGraphemes.data(), static_cast<size_t>(rawGraphemes.size()), &graphemeCount);
            if (result == GHOSTTY_SUCCESS && graphemeCount > 0) {
                for (size_t index = 0; index < graphemeCount; ++index) dynamic[static_cast<qsizetype>(index)] = static_cast<char32_t>(rawGraphemes[static_cast<qsizetype>(index)]);
                cell.text = QString::fromUcs4(dynamic.constData(), static_cast<qsizetype>(graphemeCount));
            }
        } else if (result == GHOSTTY_SUCCESS && graphemeCount > 0) {
            QVector<char32_t> text;
            text.reserve(static_cast<qsizetype>(graphemeCount));
            for (size_t index = 0; index < graphemeCount; ++index) text.append(static_cast<char32_t>(graphemes[index]));
            cell.text = QString::fromUcs4(text.constData(), static_cast<qsizetype>(text.size()));
        }

        GhosttyStyle style{};
        style.size = sizeof(GhosttyStyle);
        if (ghostty_grid_ref_style(&ref, &style) == GHOSTTY_SUCCESS) {
            cell.foreground = styleColor(style.fg_color, defaultForeground, palette);
            if (style.bg_color.tag != GHOSTTY_STYLE_COLOR_NONE) cell.background = styleColor(style.bg_color, defaultBackground, palette);
            cell.bold = style.bold;
            cell.faint = style.faint;
            cell.italic = style.italic;
            cell.underline = style.underline != 0;
            cell.strike = style.strikethrough;
            if (style.inverse) std::swap(cell.foreground, cell.background);
            if (style.invisible) cell.foreground = cell.background;
        }

        if (cell.faint) cell.foreground = faintColor(cell.foreground, cell.background);
        return cell;
    }

}

GhosttyWidget::GhosttyWidget(const int rows, const int cols, QObject *parent)
    : QObject(parent),
      m_rows(rows),
      m_cols(cols),
      m_cursorShape(VTERM_PROP_CURSORSHAPE_BAR_LEFT),
      m_cursorMode(VTERM_PROP_MOUSE_NONE) {
    GhosttyTerminalOptions options{};
    options.cols = static_cast<uint16_t>(std::max(1, m_cols));
    options.rows = static_cast<uint16_t>(std::max(1, m_rows));
    options.max_scrollback = 10000;

    if (ghostty_terminal_new(nullptr, &m_terminal, options) != GHOSTTY_SUCCESS) m_terminal = nullptr;
    if (ghostty_render_state_new(nullptr, &m_renderState) != GHOSTTY_SUCCESS) m_renderState = nullptr;
    if (ghostty_render_state_row_iterator_new(nullptr, &m_rowIterator) != GHOSTTY_SUCCESS) m_rowIterator = nullptr;
    if (ghostty_render_state_row_cells_new(nullptr, &m_rowCells) != GHOSTTY_SUCCESS) m_rowCells = nullptr;
    if (ghostty_key_encoder_new(nullptr, &m_keyEncoder) != GHOSTTY_SUCCESS) m_keyEncoder = nullptr;
    if (ghostty_key_event_new(nullptr, &m_keyEvent) != GHOSTTY_SUCCESS) m_keyEvent = nullptr;
    if (ghostty_mouse_encoder_new(nullptr, &m_mouseEncoder) != GHOSTTY_SUCCESS) m_mouseEncoder = nullptr;
    if (ghostty_mouse_event_new(nullptr, &m_mouseEvent) != GHOSTTY_SUCCESS) m_mouseEvent = nullptr;

    // Keep effects disabled while validating the core VT parser path.
    // ghostty_terminal_set() passes callback pointers through const void*, and
    // this build currently crashes inside vt_write when effects may fire.

    renderScreen();
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
    m_rows = std::max(1, rows);
    m_cols = std::max(1, cols);
    if (m_terminal) {
        ghostty_terminal_resize(m_terminal, static_cast<uint16_t>(m_cols), static_cast<uint16_t>(m_rows), static_cast<uint32_t>(m_cols), static_cast<uint32_t>(m_rows));
    }
    renderScreen();
}

void GhosttyWidget::reset(const bool hard) {
    Q_UNUSED(hard);
    ghostty_terminal_reset(m_terminal);
    renderScreen();
}

void GhosttyWidget::inputWrite(const QByteArray &bytes) {
    if (bytes.isEmpty() || !m_terminal) return;
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, [this, bytes] { inputWrite(bytes); }, Qt::QueuedConnection);
        return;
    }
    for (qsizetype offset = 0; offset < bytes.size(); ++offset) {
        const auto byte = static_cast<uint8_t>(bytes[offset]);
        ghostty_terminal_vt_write(m_terminal, &byte, 1);
    }
    renderScreen();
}

void GhosttyWidget::keyPressed(const int key, const int modifiers, const QString &text) {
    if (!m_terminal) return;
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
    const int steps = std::max(1, std::abs(lines) / 3);
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
}

void GhosttyWidget::writePtyCallback(GhosttyTerminal terminal, void *userdata, const uint8_t *data, const size_t len) {
    Q_UNUSED(terminal);
    if (!userdata) return;
    static_cast<GhosttyWidget *>(userdata)->writePty(data, len);
}

void GhosttyWidget::titleChangedCallback(GhosttyTerminal terminal, void *userdata) {
    Q_UNUSED(terminal);
    if (!userdata) return;
    static_cast<GhosttyWidget *>(userdata)->titleChanged();
}

void GhosttyWidget::bellCallback(GhosttyTerminal terminal, void *userdata) {
    Q_UNUSED(terminal);
    if (!userdata) return;
    static_cast<GhosttyWidget *>(userdata)->bell();
}

void GhosttyWidget::renderScreen() {
    if (!m_terminal) return;

    if (m_renderState) ghostty_render_state_update(m_renderState, m_terminal);

    GhosttyColorRgb foreground{240, 240, 240};
    GhosttyColorRgb background{0, 0, 0};
    GhosttyColorRgb palette[256]{};
    ghostty_color_palette_default(palette);
    ghostty_terminal_get(m_terminal, GHOSTTY_TERMINAL_DATA_COLOR_FOREGROUND, &foreground);
    ghostty_terminal_get(m_terminal, GHOSTTY_TERMINAL_DATA_COLOR_BACKGROUND, &background);
    ghostty_terminal_get(m_terminal, GHOSTTY_TERMINAL_DATA_COLOR_PALETTE, palette);

    QList<TerminalCell> screen{};
    screen.reserve(m_rows * m_cols);

    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            TerminalCell blank{};
            blank.width = 1;
            blank.text = QStringLiteral(" ");
            blank.foreground = toQColor(foreground);
            blank.background = toQColor(background);

            GhosttyPoint point{};
            point.tag = GHOSTTY_POINT_TAG_VIEWPORT;
            point.value.coordinate.x = static_cast<uint16_t>(col);
            point.value.coordinate.y = static_cast<uint32_t>(row);

            GhosttyGridRef ref{};
            ref.size = sizeof(GhosttyGridRef);
            if (ghostty_terminal_grid_ref(m_terminal, point, &ref) == GHOSTTY_SUCCESS) {
                screen.append(terminalCellFromGridRef(ref, foreground, background, palette));
            } else {
                screen.append(blank);
            }
        }
    }

    updateCursor();
    updateMouseMode();
    emit setScreen(m_rows, m_cols, screen, true);
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

    const QPoint nextPosition{static_cast<int>(y), static_cast<int>(x)};
    if (m_cursorPosition != nextPosition) {
        const QPoint oldPosition = m_cursorPosition;
        m_cursorPosition = nextPosition;
        emit setCursorPosition(m_cursorPosition, oldPosition);
    }
    if (m_cursorVisible != visible) {
        m_cursorVisible = visible;
        emit setCursorVisible(visible);
    }
    if (m_cursorBlink != blink) {
        m_cursorBlink = blink;
        emit setCursorBlink(blink);
    }

    const int nextShape = toVtermCursorShape(style);
    if (m_cursorShape != nextShape) {
        m_cursorShape = nextShape;
        emit setCursorShape(nextShape);
    }
}

void GhosttyWidget::updateMouseMode() {
    const int mode = ghosttyMouseMode(m_terminal);
    if (m_cursorMode == mode) return;
    m_cursorMode = mode;
    emit setCursorMode(mode);
}

void GhosttyWidget::writeEncodedKey(const int key, const int modifiers, const QString &text) {
    if (!m_keyEncoder || !m_keyEvent) {
        if (!text.isEmpty()) emit outputWrite(text.toUtf8());
        return;
    }

    ghostty_key_encoder_setopt_from_terminal(m_keyEncoder, m_terminal);
    ghostty_key_event_set_action(m_keyEvent, GHOSTTY_KEY_ACTION_PRESS);
    ghostty_key_event_set_key(m_keyEvent, toGhosttyKey(key));
    ghostty_key_event_set_mods(m_keyEvent, toGhosttyMods(modifiers));
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
            emit outputWrite(dynamic);
            return;
        }
    }
    if (result == GHOSTTY_SUCCESS && len > 0) {
        emit outputWrite(QByteArray(buffer.data(), static_cast<qsizetype>(len)));
        return;
    }
    if (!utf8.isEmpty()) emit outputWrite(utf8);
}

void GhosttyWidget::writeEncodedMouse(const int row, const int col, const int button, const int modifiers, const int action) {
    if (!m_terminal || !m_mouseEncoder || !m_mouseEvent) return;

    GhosttyMouseEncoderSize size{};
    size.size = sizeof(GhosttyMouseEncoderSize);
    size.screen_width = static_cast<uint32_t>(std::max(1, m_cols));
    size.screen_height = static_cast<uint32_t>(std::max(1, m_rows));
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
                                                    : toGhosttyMouseButton(button);
        ghostty_mouse_event_set_button(m_mouseEvent, ghosttyButton);
    }
    ghostty_mouse_event_set_mods(m_mouseEvent, toGhosttyMods(modifiers));
    ghostty_mouse_event_set_position(m_mouseEvent, GhosttyMousePosition{static_cast<float>(col) + 0.5F, static_cast<float>(row) + 0.5F});

    std::array<char, 128> buffer{};
    size_t len = 0;
    const GhosttyResult result = ghostty_mouse_encoder_encode(m_mouseEncoder, m_mouseEvent, buffer.data(), buffer.size(), &len);
    if (result == GHOSTTY_SUCCESS && len > 0) emit outputWrite(QByteArray(buffer.data(), static_cast<qsizetype>(len)));
}

void GhosttyWidget::writePty(const uint8_t *data, const size_t len) {
    if (!data || len == 0) return;
    emit outputWrite(QByteArray(reinterpret_cast<const char *>(data), static_cast<qsizetype>(len)));
}

void GhosttyWidget::titleChanged() {
    GhosttyString title{};
    if (ghostty_terminal_get(m_terminal, GHOSTTY_TERMINAL_DATA_TITLE, &title) != GHOSTTY_SUCCESS) return;
    emit setTitle(QString::fromUtf8(reinterpret_cast<const char *>(title.ptr), static_cast<qsizetype>(title.len)));
}

void GhosttyWidget::bell() {
    QApplication::beep();
}
