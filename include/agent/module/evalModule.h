#ifndef UNICOMM_EVALMODULE_H
#define UNICOMM_EVALMODULE_H

#include <QList>
#include <QObject>
#include <QStandardItemModel>
#include <QVariantHash>

#include "agent/module/sqlModule.h"

class EvalModel;

class EvalModule final : public QObject {
    Q_OBJECT

public:
    explicit EvalModule(SqlModule *sqlModule, QObject *parent = nullptr);

    [[nodiscard]] EvalModel *modelGet() const;

    void update(const QString &conversationId) const;

private:
    SqlModule *m_sqlModule{};
    EvalModel *m_model{};
};

class EvalModel final : public QStandardItemModel {
    Q_OBJECT
    Q_PROPERTY(QVariantHash summary READ summaryGet NOTIFY changeSummary)

public:
    using QStandardItemModel::QStandardItemModel;

    enum Role {
        NodeIdRole = Qt::UserRole + 1,
        TurnIdRole,
        MessageRole,
        ContentRole,
        ReasoningContentRole,
        ArgumentsRole,
        ApprovedRole,
        StatusRole,
        ErrorRole,
        DurationValueRole,
        ToolDurationValueRole
    };

    [[nodiscard]] QVariantHash summaryGet() const;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    void update(const QString &conversationId, const QList<SqlModule::Message> &messages);

signals:
    void changeSummary();

private:
    void turnBuild(const QList<SqlModule::Message> &messages, qsizetype index);

    QVariantHash m_summary{};
};

#endif //UNICOMM_EVALMODULE_H
