#include "terminal/module/vtermWidget.h"

#include <QDebug>
#include <QStringList>
#include <Qt>
#include <vterm.h>

namespace {
VTermModifier toVtermModifiers(const int modifiers) {
    int vtermModifiers = VTERM_MOD_NONE;
    if (modifiers & Qt::ShiftModifier) vtermModifiers |= VTERM_MOD_SHIFT;
    if (modifiers & Qt::AltModifier) vtermModifiers |= VTERM_MOD_ALT;
    if (modifiers & Qt::ControlModifier) vtermModifiers |= VTERM_MOD_CTRL;
    return static_cast<VTermModifier>(vtermModifiers);
}

int toVtermButton(const int button) {
    switch (button) {
        case Qt::LeftButton:
            return 1;
        case Qt::MiddleButton:
            return 2;
        case Qt::RightButton:
            return 3;
        default:
            return 0;
    }
}
}

VtermWidget::VtermWidget(const int rows, const int cols, QObject *parent)
    : QObject(parent),
      m_rows(rows),
      m_cols(cols),
      m_vterm(vterm_new(m_rows, m_cols)),
      m_screen(vterm_obtain_screen(m_vterm)),
      m_callbacks{
          nullptr,
          nullptr,
          nullptr,
          nullptr,
          nullptr,
          nullptr,
          nullptr,
          nullptr
      } {
    vterm_set_utf8(m_vterm, 1);

    vterm_screen_set_callbacks(m_screen, &m_callbacks, this);
    vterm_screen_set_damage_merge(m_screen, VTERM_DAMAGE_SCROLL);
    vterm_screen_reset(m_screen, 1);
}

VtermWidget::~VtermWidget() {
    if (m_vterm) vterm_free(m_vterm);
}

void VtermWidget::resize(const int rows, const int cols) {
    m_rows = rows;
    m_cols = cols;
    vterm_set_size(m_vterm, m_rows, m_cols);
    vterm_screen_flush_damage(m_screen);
}

void VtermWidget::reset(const bool hard) const {
    vterm_screen_reset(m_screen, hard ? 1 : 0);
    vterm_screen_flush_damage(m_screen);
}

void VtermWidget::inputWrite(const QByteArray &bytes) {
    if (bytes.isEmpty()) return;
    vterm_input_write(m_vterm, bytes.constData(), static_cast<size_t>(bytes.size()));
    vterm_screen_flush_damage(m_screen);
    QList<Cell> cells{};
    cells.reserve(m_rows * m_cols);
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            VTermScreenCell _cell{};
            vterm_screen_get_cell(m_screen, VTermPos{row, col}, &_cell);
            Cell cell{};

            if (_cell.chars[0] == UINT32_MAX) {
                cell.text = QString{};
            } else if (_cell.chars[0] == 0 || _cell.chars[0] == ' ') {
                cell.text = ' ';
            } else {
                int length = 0;
                while (length < VTERM_MAX_CHARS_PER_CELL && _cell.chars[length] != 0 && _cell.chars[length] != UINT32_MAX) ++length;
                cell.text = QString::fromUcs4(_cell.chars, length);
            }

            VTermColor foreground = _cell.fg;
            vterm_screen_convert_color_to_rgb(m_screen, &foreground);
            cell.foreground = QColor(foreground.rgb.red, foreground.rgb.green, foreground.rgb.blue);

            VTermColor background = _cell.bg;
            vterm_screen_convert_color_to_rgb(m_screen, &background);
            cell.background = QColor(background.rgb.red, background.rgb.green, background.rgb.blue);

            cells.append(cell);
        }
    }
    VTermPos pos{};
    vterm_state_get_cursorpos(vterm_obtain_state(m_vterm), &pos);
    emit setScreen(m_rows, m_cols, cells, {pos.row, pos.col});
}

void VtermWidget::keyPressed(const int key, const int modifiers, const QString &text) {
    const auto vtermModifiers = toVtermModifiers(modifiers);
    int vtermKey = VTERM_KEY_NONE;
    switch (key) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            vtermKey = VTERM_KEY_ENTER;
            break;
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            vtermKey = VTERM_KEY_TAB;
            break;
        case Qt::Key_Backspace:
            vtermKey = VTERM_KEY_BACKSPACE;
            break;
        case Qt::Key_Escape:
            vtermKey = VTERM_KEY_ESCAPE;
            break;
        case Qt::Key_Up:
            vtermKey = VTERM_KEY_UP;
            break;
        case Qt::Key_Down:
            vtermKey = VTERM_KEY_DOWN;
            break;
        case Qt::Key_Left:
            vtermKey = VTERM_KEY_LEFT;
            break;
        case Qt::Key_Right:
            vtermKey = VTERM_KEY_RIGHT;
            break;
        case Qt::Key_Insert:
            vtermKey = VTERM_KEY_INS;
            break;
        case Qt::Key_Delete:
            vtermKey = VTERM_KEY_DEL;
            break;
        case Qt::Key_Home:
            vtermKey = VTERM_KEY_HOME;
            break;
        case Qt::Key_End:
            vtermKey = VTERM_KEY_END;
            break;
        case Qt::Key_PageUp:
            vtermKey = VTERM_KEY_PAGEUP;
            break;
        case Qt::Key_PageDown:
            vtermKey = VTERM_KEY_PAGEDOWN;
            break;
        default:
            if (key >= Qt::Key_F1 && key <= Qt::Key_F35) {
                vtermKey = VTERM_KEY_FUNCTION(key - Qt::Key_F1 + 1);
            }
            break;
    }

    if (vtermKey != VTERM_KEY_NONE) vterm_keyboard_key(m_vterm, static_cast<VTermKey>(vtermKey), vtermModifiers);
    else for (const auto ch: text.toUcs4()) vterm_keyboard_unichar(m_vterm, ch, vtermModifiers);
    outputRead();
}

void VtermWidget::mousePressed(const int row, const int col, const int button, const int modifiers) {
    const auto vtermModifiers = toVtermModifiers(modifiers);
    vterm_mouse_move(m_vterm, row, col, vtermModifiers);
    const int vtermButton = toVtermButton(button);
    if (vtermButton > 0) vterm_mouse_button(m_vterm, vtermButton, true, vtermModifiers);
    outputRead();
}

void VtermWidget::mouseReleased(const int row, const int col, const int button, const int modifiers) {
    const auto vtermModifiers = toVtermModifiers(modifiers);
    vterm_mouse_move(m_vterm, row, col, vtermModifiers);
    const int vtermButton = toVtermButton(button);
    if (vtermButton > 0) vterm_mouse_button(m_vterm, vtermButton, false, vtermModifiers);
    outputRead();
}

void VtermWidget::mouseMoved(const int row, const int col, const int button, const int modifiers) {
    const auto vtermModifiers = toVtermModifiers(modifiers);
    vterm_mouse_move(m_vterm, row, col, vtermModifiers);
    outputRead();
}

// private
void VtermWidget::outputRead() {
    QByteArray output;
    char buffer[256]{};
    while (vterm_output_get_buffer_current(m_vterm) > 0) {
        const auto read = vterm_output_read(m_vterm, buffer, sizeof(buffer));
        if (read == 0) break;
        output.append(buffer, static_cast<qsizetype>(read));
    }
    emit outputWrite(output);
}
