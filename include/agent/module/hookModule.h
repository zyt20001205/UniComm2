#ifndef UNICOMM_HOOKMODULE_H
#define UNICOMM_HOOKMODULE_H

#include <QJsonObject>
#include <QStandardItemModel>

class HookModel;

class HookModule final : public QObject {
    Q_OBJECT

public:
    explicit HookModule(const QJsonObject &config, QObject *parent = nullptr);

    [[nodiscard]] HookModel *hookModelGet() const;

    [[nodiscard]] const QJsonObject &configGet() const;

    void hookEnabledSet(const QString &event, bool enabled);

    void hookScriptInsert(const QString &event, const QUrl &documentUrl);

    void hookScriptRemove(const QString &event, const QUrl &documentUrl);

    void hookRun(const QString &event) const;

private:
    [[nodiscard]] QVariantList scriptsGet(const QString &event) const;

    void hookUpdate(const QString &event) const;

    QJsonObject m_config{};
    HookModel *m_hookModel{};
};

class HookModel final : public QStandardItemModel {
    Q_OBJECT

public:
    explicit HookModel(QObject *parent = nullptr);

    enum Role {
        EventRole = Qt::UserRole + 1,
        DescriptionRole,
        EnabledRole,
        ScriptsRole
    };

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
};

#endif //UNICOMM_HOOKMODULE_H
