#include "terminal/vtermWidget.h"

#include <algorithm>

#include <QStringList>

#include <vterm.h>

namespace {
int screenDamage(VTermRect, void *user) {
    return user ? 1 : 0;
}

const VTermScreenCallbacks kScreenCallbacks{
    screenDamage,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

QString cellText(const VTermScreenCell &cell) {
    if (cell.chars[0] == 0) {
        return QStringLiteral(" ");
    }

    int length = 0;
    while (length < VTERM_MAX_CHARS_PER_CELL && cell.chars[length] != 0) {
        ++length;
    }
    return QString::fromUcs4(cell.chars, length);
}
}

VtermWidget::VtermWidget(const int rows, const int cols, QObject *parent)
    : QObject(parent),
      m_rows(std::max(1, rows)),
      m_cols(std::max(1, cols)) {
    m_vterm = vterm_new(m_rows, m_cols);
    if (!m_vterm) {
        m_rows = 0;
        m_cols = 0;
        return;
    }

    vterm_set_utf8(m_vterm, 1);
    m_screen = vterm_obtain_screen(m_vterm);
    vterm_screen_set_callbacks(m_screen, &kScreenCallbacks, this);
    vterm_screen_set_damage_merge(m_screen, VTERM_DAMAGE_SCROLL);
    vterm_screen_reset(m_screen, 1);
}

VtermWidget::~VtermWidget() {
    if (m_vterm) {
        vterm_free(m_vterm);
    }
}

bool VtermWidget::isValid() const {
    return m_vterm && m_screen;
}

int VtermWidget::rows() const {
    return m_rows;
}

int VtermWidget::cols() const {
    return m_cols;
}

void VtermWidget::resizeTerminal(const int rows, const int cols) {
    if (!isValid()) {
        return;
    }

    m_rows = std::max(1, rows);
    m_cols = std::max(1, cols);
    vterm_set_size(m_vterm, m_rows, m_cols);
    flushDamage();
}

void VtermWidget::reset(const bool hard) {
    if (!isValid()) {
        return;
    }

    vterm_screen_reset(m_screen, hard ? 1 : 0);
    flushDamage();
}

qsizetype VtermWidget::write(const QByteArray &bytes) {
    if (!isValid() || bytes.isEmpty()) {
        return 0;
    }

    const auto written = vterm_input_write(m_vterm, bytes.constData(), static_cast<size_t>(bytes.size()));
    flushDamage();
    return static_cast<qsizetype>(written);
}

qsizetype VtermWidget::write(const QString &text) {
    return write(text.toUtf8());
}

QString VtermWidget::lineText(const int row) const {
    if (!isValid() || row < 0 || row >= m_rows) {
        return {};
    }

    QString text;
    text.reserve(m_cols);
    for (int col = 0; col < m_cols; ++col) {
        VTermScreenCell cell{};
        if (!vterm_screen_get_cell(m_screen, VTermPos{row, col}, &cell)) {
            text.append(QLatin1Char(' '));
            continue;
        }
        text.append(cellText(cell));
    }
    return text;
}

QString VtermWidget::screenText() const {
    if (!isValid()) {
        return {};
    }

    QStringList lines;
    lines.reserve(m_rows);
    for (int row = 0; row < m_rows; ++row) {
        lines.append(lineText(row));
    }
    return lines.join(QLatin1Char('\n'));
}

void VtermWidget::flushDamage() {
    if (m_screen) {
        vterm_screen_flush_damage(m_screen);
    }
}
