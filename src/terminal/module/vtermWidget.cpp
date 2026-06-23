#include "terminal/module/vtermWidget.h"

#include "globals.h"
#include "util/uniCast.h"

VtermWidget::VtermWidget(const int rows, const int cols, QObject *parent)
    : QObject(parent),
      m_rows(rows),
      m_cols(cols),
      m_vterm(vterm_new(m_rows, m_cols)),
      m_screen(vterm_obtain_screen(m_vterm)) {
    vterm_set_utf8(m_vterm, 1);

    m_callbacks.sb_pushline = [](const int cols, const VTermScreenCell *cells, void *user) -> int {
        return static_cast<VtermWidget *>(user)->linePush(cols, cells);
    };

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
    QList<TerminalCell> cells{};
    cells.reserve(m_rows * m_cols);
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            VTermScreenCell _cell{};
            vterm_screen_get_cell(m_screen, VTermPos{row, col}, &_cell);
            const auto &cell = uni_cast<TerminalCell>(m_screen, _cell);
            cells.append(cell);
        }
    }
    VTermPos pos{};
    vterm_state_get_cursorpos(vterm_obtain_state(m_vterm), &pos);
    emit setScreen(m_rows, m_cols, cells, m_scrollback, {pos.row, pos.col});
}

void VtermWidget::keyPressed(const int key, const int modifiers, const QString &text) {
    const auto &vtermModifier = uni_cast<VTermModifier>(modifiers);
    const auto &vtermKey = uni_cast<VTermKey>(key);
    if (vtermKey != VTERM_KEY_NONE) vterm_keyboard_key(m_vterm, vtermKey, vtermModifier);
    else for (const auto ch: text.toUcs4()) vterm_keyboard_unichar(m_vterm, ch, vtermModifier);
    outputRead();
}

void VtermWidget::mousePressed(const int row, const int col, const int button, const int modifiers) {
    const auto &vtermModifier = uni_cast<VTermModifier>(modifiers);
    vterm_mouse_move(m_vterm, row, col, vtermModifier);
    const auto &vtermButton = uni_cast<VTermButton>(button);
    if (vtermButton > 0) vterm_mouse_button(m_vterm, vtermButton, true, vtermModifier);
    outputRead();
}

void VtermWidget::mouseReleased(const int row, const int col, const int button, const int modifiers) {
    const auto &vtermModifier = uni_cast<VTermModifier>(modifiers);
    vterm_mouse_move(m_vterm, row, col, vtermModifier);
    const auto &vtermButton = uni_cast<VTermButton>(button);
    if (vtermButton > 0) vterm_mouse_button(m_vterm, vtermButton, false, vtermModifier);
    outputRead();
}

void VtermWidget::mouseMoved(const int row, const int col, const int button, const int modifiers) {
    const auto &vtermModifier = uni_cast<VTermModifier>(modifiers);
    vterm_mouse_move(m_vterm, row, col, vtermModifier);
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

int VtermWidget::linePush(const int cols, const VTermScreenCell *cells) {
    QList<TerminalCell> row{};
    row.reserve(cols);
    for (int col = 0; col < cols; ++col) {
        const auto &cell = uni_cast<TerminalCell>(m_screen, cells[col]);
        row.append(cell);
    }
    m_scrollback.append(row);

    constexpr int maxScrollbackLines = 10000;
    if (m_scrollback.size() > maxScrollbackLines) m_scrollback.removeFirst();

    return 1;
}
