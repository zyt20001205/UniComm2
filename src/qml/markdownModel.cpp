#include "qml/markdownModel.h"

MarkdownModel::MarkdownModel(QObject *parent) : QAbstractListModel(parent) {
    m_timer.setInterval(32);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &MarkdownModel::update);
}

int MarkdownModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : static_cast<int>(m_blocks.size());
}

QVariant MarkdownModel::data(const QModelIndex &index, const int role) const {
    if (!index.isValid()) return {};
    const auto &block = m_blocks.at(index.row());
    switch (role) {
        case TypeRole: return block.type;
        case ContentRole: return block.content;
        case LanguageRole: return block.language;
        default: return {};
    }
}

QHash<int, QByteArray> MarkdownModel::roleNames() const {
    return {
        {TypeRole, "type"},
        {ContentRole, "content"},
        {LanguageRole, "language"}
    };
}

QString MarkdownModel::sourceGet() const {
    return m_source;
}

void MarkdownModel::sourceSet(const QString &source) {
    if (m_source == source) return;
    m_source = source;
    emit changeSource();
    if (!m_timer.isActive()) m_timer.start();
}

void MarkdownModel::flush() {
    if (!m_timer.isActive()) return;
    m_timer.stop();
    update();
}

// private
void MarkdownModel::update() {
    auto blocks = uni_cast<QList<MarkdownBlock>>(m_source);
    auto structureChanged = blocks.size() != m_blocks.size();
    for (qsizetype index = 0; !structureChanged && index < blocks.size(); ++index) {
        structureChanged = blocks.at(index).type != m_blocks.at(index).type;
    }

    if (structureChanged) {
        beginResetModel();
        m_blocks = std::move(blocks);
        endResetModel();
        return;
    }

    auto changed = false;
    for (qsizetype index = 0; index < blocks.size(); ++index) {
        if (blocks.at(index).content != m_blocks.at(index).content || blocks.at(index).language != m_blocks.at(index).language) changed = true;
    }
    if (!changed) return;
    m_blocks = std::move(blocks);
    emit dataChanged(index(0), index(rowCount({}) - 1), {ContentRole, LanguageRole});
}
