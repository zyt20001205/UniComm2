#ifndef UNICOMM_MARKDOWNMODEL_H
#define UNICOMM_MARKDOWNMODEL_H

#include <QAbstractListModel>
#include <QTimer>
#include <QtQmlIntegration/qqmlintegration.h>

#include "util/uniCast.h"

class MarkdownModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString source READ sourceGet WRITE sourceSet NOTIFY changeSource)

public:
    enum Type {
        Markdown = MarkdownBlock::Type::Markdown,
        Code = MarkdownBlock::Type::Code
    };
    Q_ENUM(Type)

    enum Role {
        TypeRole = Qt::UserRole + 1,
        ContentRole,
        LanguageRole
    };

    explicit MarkdownModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] QString sourceGet() const;

    void sourceSet(const QString &source);

    Q_INVOKABLE void flush();

signals:
    void changeSource();

private:
    void update();

    QString m_source{};
    QList<MarkdownBlock> m_blocks{};
    QTimer m_timer{};
};

#endif //UNICOMM_MARKDOWNMODEL_H
