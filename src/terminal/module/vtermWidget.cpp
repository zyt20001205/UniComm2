#include "terminal/module/vtermWidget.h"

#include <QClipboard>

#include "globals.h"
#include "util/uniCast.h"

VtermWidget::VtermWidget(const int rows, const int cols, QObject *parent)
    : QObject(parent),
      m_rows(rows),
      m_cols(cols),
      m_vterm(vterm_new(m_rows, m_cols)),
      m_state(vterm_obtain_state(m_vterm)),
      m_screen(vterm_obtain_screen(m_vterm)),
      m_selectionBuffer(1024 * 1024, '\0') {
    vterm_set_utf8(m_vterm, 1);

    m_callbacks.movecursor = [](const VTermPos pos, const VTermPos oldPos, const int visible, void *user) -> int {
        return static_cast<VtermWidget *>(user)->cursorMove(pos, oldPos, visible);
    };
    m_callbacks.settermprop = [](const VTermProp prop, VTermValue *value, void *user) -> int {
        return static_cast<VtermWidget *>(user)->termPropSet(prop, value);
    };
    m_callbacks.bell = [](void *user) -> int {
        return static_cast<VtermWidget *>(user)->bell();
    };
    m_callbacks.sb_pushline = [](const int cols, const VTermScreenCell *cells, void *user) -> int {
        return static_cast<VtermWidget *>(user)->linePush(cols, cells);
    };
    m_fallbacks.osc = [](const int command, const VTermStringFragment frag, void *user) -> int {
        return static_cast<VtermWidget *>(user)->osc(command, frag);
    };
    m_selectionCallbacks.set = [](const VTermSelectionMask mask, const VTermStringFragment frag, void *user) -> int {
        return static_cast<VtermWidget *>(user)->selectionSet(mask, frag);
    };
    m_selectionCallbacks.query = [](const VTermSelectionMask mask, void *user) -> int {
        return static_cast<VtermWidget *>(user)->selectionQuery(mask);
    };

    vterm_screen_set_callbacks(m_screen, &m_callbacks, this);
    vterm_screen_set_unrecognised_fallbacks(m_screen, &m_fallbacks, this);
    vterm_state_set_selection_callbacks(m_state, &m_selectionCallbacks, this, m_selectionBuffer.data(), static_cast<size_t>(m_selectionBuffer.size()));
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
    renderScreen();
}

void VtermWidget::reset(const bool hard) {
    vterm_screen_reset(m_screen, hard ? 1 : 0);
    vterm_screen_flush_damage(m_screen);
    m_scrollOffset = 0;
    renderScreen();
}

void VtermWidget::inputWrite(const QByteArray &bytes) {
    if (bytes.isEmpty()) return;
    vterm_input_write(m_vterm, bytes.constData(), static_cast<size_t>(bytes.size()));
    vterm_screen_flush_damage(m_screen);
    m_scrollOffset = qBound(0, m_scrollOffset, m_scrollback.size());
    renderScreen();
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

void VtermWidget::mouseWheeled(const int row, const int col, const int lines, const int modifiers) {
    const auto &vtermModifier = uni_cast<VTermModifier>(modifiers);
    const int button = lines > 0 ? 4 : 5;
    const int steps = qMax(1, qAbs(lines) / 3);
    vterm_mouse_move(m_vterm, row, col, vtermModifier);
    for (int step = 0; step < steps; ++step) {
        vterm_mouse_button(m_vterm, button, true, vtermModifier);
        vterm_mouse_button(m_vterm, button, false, vtermModifier);
    }
    outputRead();
}

void VtermWidget::mouseScrolled(const int lines) {
    if (lines == 0) return;
    m_scrollOffset = qBound(0, m_scrollOffset + lines, m_scrollback.size());
    renderScreen();
}

// private
void VtermWidget::renderScreen() {
    QList<TerminalCell> screen{};
    screen.reserve(m_rows * m_cols);
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            VTermScreenCell _cell{};
            vterm_screen_get_cell(m_screen, VTermPos{row, col}, &_cell);
            screen.append(uni_cast<TerminalCell>(m_screen, _cell));
        }
    }

    m_scrollOffset = qBound(0, m_scrollOffset, m_scrollback.size());
    const int totalRows = m_scrollback.size() + m_rows;
    const int firstRow = qMax(0, totalRows - m_rows - m_scrollOffset);

    QList<TerminalCell> visible{};
    visible.reserve(m_rows * m_cols);
    for (int row = 0; row < m_rows; ++row) {
        const int sourceRow = firstRow + row;
        if (sourceRow < m_scrollback.size()) {
            const auto &line = m_scrollback[sourceRow];
            for (int col = 0; col < m_cols; ++col) {
                if (col < line.size()) visible.append(line[col]);
                else visible.append(TerminalCell{});
            }
        } else {
            const int screenRow = sourceRow - m_scrollback.size();
            for (int col = 0; col < m_cols; ++col) {
                visible.append(screen[screenRow * m_cols + col]);
            }
        }
    }

    emit setScreen(m_rows, m_cols, visible, m_scrollOffset == 0);
}

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

int VtermWidget::cursorMove(const VTermPos pos, const VTermPos oldPos, const int visible) {
    Q_UNUSED(oldPos);
    Q_UNUSED(visible);
    emit setCursorPosition({pos.row, pos.col});
    return 1;
}

int VtermWidget::termPropSet(const VTermProp prop, const VTermValue *value) {
    switch (static_cast<int>(prop)) {
        case VTERM_PROP_CURSORVISIBLE: emit setCursorVisible(value->boolean);
            break;
        case VTERM_PROP_CURSORBLINK: emit setCursorBlink(value->boolean);
            break;
        case VTERM_PROP_CURSORSHAPE: emit setCursorShape(value->number);
            break;
        case VTERM_PROP_MOUSE: emit setCursorMode(value->number);
            break;
        default: break;
    }
    return 1;
}

int VtermWidget::bell() {
    QApplication::beep();
    return 1;
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

int VtermWidget::osc(const int command, const VTermStringFragment frag) {
    switch (command) {
        case 8: return osc8(frag);
        default: return 0;
    }
}

int VtermWidget::osc8(const VTermStringFragment frag) {
    if (!frag.final) return 1;

    const QString payload = QString::fromUtf8(frag.str, static_cast<qsizetype>(frag.len));
    const int separator = payload.indexOf(';');
    if (separator < 0) return 1;

    const QString url = payload.mid(separator + 1);
    if (url.isEmpty()) return 1;

    m_hyperlink = url;
    return 1;
}

int VtermWidget::selectionSet(const VTermSelectionMask mask, const VTermStringFragment frag) {
    if (frag.initial) m_pendingSelection.clear();
    m_pendingSelection += QString::fromUtf8(frag.str, static_cast<qsizetype>(frag.len));

    if (!frag.final) return 1;

    if (!m_pendingSelection.isEmpty()) {
        auto *clipboard = QApplication::clipboard();
        if (static_cast<int>(mask) & VTERM_SELECTION_CLIPBOARD) {
            clipboard->setText(m_pendingSelection, QClipboard::Clipboard);
        }
        if ((static_cast<int>(mask) & VTERM_SELECTION_PRIMARY) && clipboard->supportsSelection()) {
            clipboard->setText(m_pendingSelection, QClipboard::Selection);
        }
    }
    m_pendingSelection.clear();
    return 1;
}

int VtermWidget::selectionQuery(const VTermSelectionMask mask) {
    qDebug() << "OSC 52 selection query:" << mask;
    return 1;
}
