#include "terminal/module/vtermWidget.h"

#include <QDebug>
#include <QStringList>
#include <vterm.h>

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

void VtermWidget::inputWrite(const QByteArray &bytes) const {
    if (bytes.isEmpty()) return;
    vterm_input_write(m_vterm, bytes.constData(), static_cast<size_t>(bytes.size()));
    vterm_screen_flush_damage(m_screen);
}

QByteArray VtermWidget::keyboardKey(const int key, const int modifiers) const {
    vterm_keyboard_key(m_vterm, static_cast<VTermKey>(key), static_cast<VTermModifier>(modifiers));
    return outputRead();
}

QByteArray VtermWidget::keyboardUnichar(const QString &text, const int modifiers) const {
    for (const auto ch: text.toUcs4()) vterm_keyboard_unichar(m_vterm, ch, static_cast<VTermModifier>(modifiers));
    return outputRead();
}

QString VtermWidget::text() const {
    // all
    QStringList lines{};
    for (int row = 0; row < m_rows; ++row) {
        // line
        QString line{};

        QString buffer{};
        QString foreground{};
        for (int col = 0; col < m_cols; ++col) {
            QString cell{};
            // cell
            VTermScreenCell _cell{};
            vterm_screen_get_cell(m_screen, VTermPos{row, col}, &_cell);
            if (_cell.chars[0] == 0 || _cell.chars[0] == ' ') cell = "&nbsp;";
            else if (_cell.chars[0] == UINT32_MAX) cell = "&#8203;";
            else cell = QString::fromUcs4(&_cell.chars[0], 1).toHtmlEscaped();

            VTermColor fg = _cell.fg;
            vterm_screen_convert_color_to_rgb(m_screen, &fg);
            const auto &_foreground = QString("#%1%2%3")
                .arg(fg.rgb.red, 2, 16, QLatin1Char('0'))
                .arg(fg.rgb.green, 2, 16, QLatin1Char('0'))
                .arg(fg.rgb.blue, 2, 16, QLatin1Char('0'));
            // first
            if (foreground.isEmpty()) {
                foreground = _foreground;
            }
            // new style
            else if (foreground != _foreground) {
                line.append(QString("<span style=\"color:%1\">%2</span>").arg(foreground, buffer));
                buffer.clear();
                foreground = _foreground;
            }
            buffer.append(cell);
        }
        if (!buffer.isEmpty()) line.append(QString("<span style=\"color:%1\">%2</span>").arg(foreground, buffer));
        lines.append(line);
    }
    return lines.join("<br>");
}

int VtermWidget::cursorPosition() const {
    VTermPos pos{};
    vterm_state_get_cursorpos(vterm_obtain_state(m_vterm), &pos);
    return pos.row * (m_cols + 1) + pos.col;
}

QByteArray VtermWidget::outputRead() const {
    QByteArray output;
    char buffer[256]{};
    while (vterm_output_get_buffer_current(m_vterm) > 0) {
        const auto read = vterm_output_read(m_vterm, buffer, sizeof(buffer));
        if (read == 0) break;
        output.append(buffer, static_cast<qsizetype>(read));
    }
    return output;
}
